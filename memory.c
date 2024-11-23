#include "memory.h"

#include <assert.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "gc.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static void blacken_object(Obj* object);

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
  vm.bytes_allocated += new_size - old_size;

  if (new_size > old_size && !VM_HAS_FLAG(VM_FLAG_PAUSE_GC)) {  // Allocating
    if (vm.bytes_allocated > vm.next_gc || VM_HAS_FLAG(VM_FLAG_STRESS_GC)) {
      collect_garbage();
    }
  }

  if (new_size == 0) {  // Freeing
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, new_size);
  if (result == NULL) {
    INTERNAL_ERROR("Not enough memory to reallocate");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  return result;
}

Obj* allocate_object(size_t size, ObjGcType type) {
  Obj* object = (Obj*)reallocate(NULL, 0,
                                 size);  // Might trigger GC, but it's fine since our new object
                                         // isn't referenced by anything yet -> not reachable by GC
  object->type = type;
  object->hash = (uint64_t)((uintptr_t)(object) >> 4 | (uintptr_t)(object) << 60);  // Get a better distribution of hash
                                                                                    // values, by shifting the address

  // Atomically reset the is_marked flag
  atomic_init(&object->is_marked, false);

  // Atomically increment the object count
  atomic_fetch_add_explicit(&vm.object_count, 1, memory_order_relaxed);

  object->next = vm.objects;
  vm.objects   = object;

  return object;
}

void free_object(Obj* object) {
  // Atomically decrement the object count. Doesn't matter that this is at the top, since we only check the object count at the
  // end of the GC cycle.
  atomic_fetch_sub(&vm.object_count, 1);
  GC_WORKER_STATS_INC_FREED();

  switch (object->type) {
    case OBJ_GC_BOUND_METHOD: FREE(ObjBoundMethod, object); break;
    case OBJ_GC_CLASS: {
      ObjClass* klass = (ObjClass*)object;
      free_hashtable(&klass->methods);
      free_hashtable(&klass->static_methods);
      FREE(ObjClass, object);
      break;
    }
    case OBJ_GC_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalue_count);
      FREE(ObjClosure, object);
      break;
    }
    case OBJ_GC_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      free_chunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }
    case OBJ_GC_OBJECT: {
      ObjObject* object_ = (ObjObject*)object;
      free_hashtable(&object_->fields);
      FREE(ObjObject, object);
      break;
    }
    case OBJ_GC_NATIVE: FREE(ObjNative, object); break;
    case OBJ_GC_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
    case OBJ_GC_SEQ: {
      ObjSeq* seq = (ObjSeq*)object;
      free_value_array(&seq->items);
      FREE(ObjSeq, object);
      break;
    }
    case OBJ_GC_TUPLE: {
      ObjTuple* tuple = (ObjTuple*)object;
      free_value_array(&tuple->items);
      FREE(ObjTuple, object);
      break;
    }
    case OBJ_GC_UPVALUE: FREE(ObjUpvalue, object); break;
    default: INTERNAL_ERROR("Don't know how to free unknown object type: %d at %p", object->type, object);
  }
}

void mark_obj(Obj* object) {
  // Unnecessary NULL check if we're called from mark_value, but if called from
  // elsewhere, the object arg could be NULL.
  if (object == NULL) {
    return;
  }

  // Atomically check and set the is_marked flag
  if (!atomic_exchange(&object->is_marked, true)) {
    GC_WORKER_STATS_INC_MARKED();
    blacken_object(object);
  }
}

void mark_value(Value value) {
  if (!is_primitive(value)) {
    mark_obj(value.as.obj);
  }
}

// Marks all entries in a value array gray.
static void mark_array(ValueArray* array) {
  if (array->count < GC_PARALLEL_MARK_ARRAY_THRESHOLD) {
    for (int i = 0; i < array->count; i++) {
      mark_value(array->values[i]);
    }
  } else {
    gc_parallel_mark_array(array);
  }
}

static void mark_hashtable(HashTable* table) {
  if (table->count < GC_PARALLEL_MARK_HASHTABLE_THRESHOLD) {
    for (int i = 0; i < table->capacity; i++) {
      Entry* entry = &table->entries[i];
      if (!is_empty_internal(entry->key)) {
        mark_value(entry->key);
        mark_value(entry->value);
      }
    }
  } else {
    gc_parallel_mark_hashtable(table);
  }
}

// Blackens an object by marking all objects it references. A black object is one that has been marked and all objects it
// references have been marked as well.
static void blacken_object(Obj* object) {
  switch (object->type) {
    case OBJ_GC_BOUND_METHOD: {
      ObjBoundMethod* bound = (ObjBoundMethod*)object;
      mark_value(bound->receiver);
      mark_obj(bound->method);
      break;
    }
    case OBJ_GC_CLASS: {
      ObjClass* klass = (ObjClass*)object;
      mark_obj((Obj*)klass->name);
      mark_obj((Obj*)klass->base);
      mark_hashtable(&klass->methods);
      mark_hashtable(&klass->static_methods);
      break;
    }
    case OBJ_GC_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      mark_obj((Obj*)closure->function);
      for (int i = 0; i < closure->upvalue_count; i++) {
        mark_obj((Obj*)closure->upvalues[i]);
      }
      break;
    }
    case OBJ_GC_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      mark_obj((Obj*)function->name);
      mark_obj((Obj*)function->globals_context);
      mark_array(&function->chunk.constants);
      break;
    }
    case OBJ_GC_OBJECT: {
      ObjObject* object_ = (ObjObject*)object;
      mark_obj((Obj*)object_->instance_class);
      mark_hashtable(&object_->fields);
      break;
    }
    case OBJ_GC_UPVALUE: mark_value(((ObjUpvalue*)object)->closed); break;
    case OBJ_GC_SEQ: {
      ObjSeq* seq = (ObjSeq*)object;
      mark_array(&seq->items);
      break;
    }
    case OBJ_GC_TUPLE: {
      ObjTuple* tuple = (ObjTuple*)object;
      mark_array(&tuple->items);
      break;
    }
    case OBJ_GC_NATIVE: {
      ObjNative* native = (ObjNative*)object;
      mark_obj((Obj*)native->name);
      break;
    }
    case OBJ_GC_STRING: break;
    default:
      break;  // TODO (recovery): What do we do here? Throw? Probably yes, bc we
              // need to mark all objects
  }
}

void free_heap() {
  gc_assign_current_worker(0);  // Assign the main thread as the worker
  Obj* object = vm.objects;
  while (object != NULL) {
    Obj* next = object->next;
    free_object(object);
    object = next;
  }
  gc_assign_current_worker(-1);  // Unassign
}

// Starts at the roots of the objects in the heap and marks all reachable objects. It's important that all root objects get marked
// in this phase - if you e.g. had a root which is a long array which gets split into different mark tasks and distributed between
// the workers, that'd be a problem because the workers are idle until after mark_roots, leaving us with not all roots marked.
static void mark_roots() {
  // Most roots are local variables, which are on the stack.
  for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
    mark_value(*slot);
  }

  // Call frames are also roots, because they contain function call state.
  for (int i = 0; i < vm.frame_count; i++) {
    mark_obj((Obj*)vm.frames[i].closure);
  }

  // Open upvalues are also roots directly accessible by the vm.
  for (ObjUpvalue* upvalue = vm.open_upvalues; upvalue != NULL; upvalue = upvalue->next) {
    mark_obj((Obj*)upvalue);
  }

  // Mark the cache of loaded modules
  for (int i = 0; i < vm.modules.capacity; i++) {
    Entry* entry = &vm.modules.entries[i];
    if (!is_empty_internal(entry->key)) {
      mark_value(entry->key);
      mark_value(entry->value);
    }
  }

  // As well as the active module
  if (vm.module != NULL) {
    mark_obj((Obj*)vm.module);
  }

  // Mark the current error
  mark_value(vm.current_error);

  // And the native functions and types
  mark_hashtable(&vm.natives);

  // And the reserved field names
  for (int i = 0; i < SPECIAL_METHOD_MAX; i++) {
    mark_obj((Obj*)(vm.special_method_names[i]));
  }

  // And the special prop names
  for (int i = 0; i < SPECIAL_PROP_MAX; i++) {
    mark_obj((Obj*)(vm.special_prop_names[i]));
  }

  // And the base classes
  mark_obj((Obj*)vm.obj_class);
  mark_obj((Obj*)vm.num_class);
  mark_obj((Obj*)vm.int_class);
  mark_obj((Obj*)vm.float_class);
  mark_obj((Obj*)vm.bool_class);
  mark_obj((Obj*)vm.nil_class);
  mark_obj((Obj*)vm.seq_class);
  mark_obj((Obj*)vm.tuple_class);
  mark_obj((Obj*)vm.str_class);
  mark_obj((Obj*)vm.fn_class);
  mark_obj((Obj*)vm.class_class);

  mark_obj((Obj*)vm.upvalue_class);
  mark_obj((Obj*)vm.handler_class);

  mark_obj((Obj*)vm.module_class);

  // And the compiler roots. The GC can run while compiling, so we need to mark
  // the compiler's internal state as well.
  mark_compiler_roots();
}

static void mark() {
  mark_roots();
  gc_wait_for_workers();
}

static void sweep_sequential() {
  GC_SWEEP_LOG(ANSI_RED_STR("[GC]") " " ANSI_CYAN_STR("[SWEEP]") " Sweeping heap sequential\n");
  GC_SWEEP_LOG("  Sweeping %zu objects\n", atomic_load(&vm.object_count));

  Obj* previous = NULL;
  Obj* object   = vm.objects;

  while (object != NULL) {
    // Atomically check and reset the is_marked flag
    if (atomic_exchange(&object->is_marked, false)) {  // Object is marked (black), keep it
      previous = object;
      object   = object->next;
    } else {  // Object is unmarked (white), remove it
      Obj* unreached = object;

      object = object->next;
      if (previous != NULL) {
        previous->next = object;
      } else {
        vm.objects = object;
      }

      free_object(unreached);
    }
  }

  GC_SWEEP_LOG("  Done sweeping\n\n");
}

// Sweeps the heap and frees all unmarked objects. This is done by iterating over the linked list of objects and freeing all
// white objects. White objects are objects that have not been marked during the mark phase and are therefore unreachable.
static void sweep() {
  size_t total_object_count = atomic_load(&vm.object_count);

  if (total_object_count <= GC_PARALLEL_SWEEP_THRESHOLD) {
    sweep_sequential();
    return;
  }

  if (!gc_parallel_sweep()) {
    INTERNAL_WARN("Parallel sweep failed, falling back to sequential sweep");
    sweep_sequential();
  }
}

#ifdef DEBUG_GC_PHASE_TIMES
static GcPhaseTimes gc_times = {(size_t)0, 0, 0, 0, 0, -1 /* prev_mutator_time*/, 0};

#define DEBUG_GC_PHASE_TIMESTAMP(varname) gc_times.varname = get_time();

static void print_phase_times() {
  printf(ANSI_RED_STR(
      "[GC]") " " ANSI_MAGENTA_STR("[Cycle %zu]")" Phase Times"
              ":\n", gc_times.cycle_count++);
  printf("  Mutator execution time:        %fms\n", (gc_times.runtime) * 1000);
  printf("  1) Mark time:                  %fms\n", (gc_times.mark_time - gc_times.cycle_start) * 1000);
  printf("  2) Remove white strings time:  %fms\n", (gc_times.remove_white_time - gc_times.mark_time) * 1000);
  printf("  3) Sweep time:                 %fms\n", (gc_times.sweep_time - gc_times.remove_white_time) * 1000);
  printf("  Total GC pause time:           " ANSI_CYAN_STR("%fms") "\n", (gc_times.sweep_time - gc_times.cycle_start) * 1000);
  printf("\n");
}
#else
#define DEBUG_GC_PHASE_TIMESTAMP(varname)
#endif

#ifdef DEBUG_GC_HEAP_STATS
#define KIB(bytes) (bytes / 1024.0)
#define MIB(bytes) (bytes / (1024.0 * 1024.0))
#define GIB(bytes) (bytes / (1024.0 * 1024.0 * 1024.0))
#define STATS(bytes) "%zuB / %.2fKiB / %.2fMiB / %.2fGiB\n", bytes, KIB(bytes), MIB(bytes), GIB(bytes)
static void print_heap_stats() {
  printf(ANSI_RED_STR("[GC]") " " ANSI_GREEN_STR("[Heap Stats]") ":\n");
  printf("  Total objects on the heap:    %zu\n", atomic_load(&vm.object_count));
  printf("  Current heap size:            " STATS(vm.bytes_allocated));
  printf("  Bytes collected (This cycle): " STATS(vm.prev_gc_freed));
  printf("  Next GC at:                   " STATS(vm.next_gc));
  printf("\n");
}
#undef KIB
#undef MIB
#undef GIB
#undef STATS
#endif

void collect_garbage() {
  DEBUG_GC_PHASE_TIMESTAMP(cycle_start);
#ifdef DEBUG_GC_PHASE_TIMES
  gc_times.runtime = gc_times.cycle_start - gc_times.prev_mutator_time;
  if (gc_times.prev_mutator_time == -1) {
    gc_times.runtime = 0;
  }
  gc_times.prev_mutator_time = gc_times.cycle_start;
#endif

  size_t before = vm.bytes_allocated;

  // Main thread acts as worker 0 for a GC cycle.
  gc_assign_current_worker(0);
  gc_wake_workers();

  // Step 1: Mark roots - this is single-threaded. Generates work for workers, essentially each mark-task can be viewed as a as a
  // gray object that needs to be blackened. It's important that all root objects get marked in this phase - if you e.g. had a
  // root which is a long array which gets split into different mark tasks and distributed between the workers, that'd be a
  // problem because the workers are idle until after mark_roots, leaving us with not all roots marked.
  mark();
  DEBUG_GC_PHASE_TIMESTAMP(mark_time);

  // Step 2: Handle interned strings.
  hashtable_remove_white(&vm.strings);
  DEBUG_GC_PHASE_TIMESTAMP(remove_white_time);

  // Step 3: Sweep the heap
  sweep();
  DEBUG_GC_PHASE_TIMESTAMP(sweep_time);

  gc_workers_put_to_sleep();
  gc_assign_current_worker(-1);

  // Update GC thresholds
  if (vm.bytes_allocated < HEAP_GROW_THRESHOLD) {
    vm.next_gc = vm.bytes_allocated * HEAP_GROW_FACTOR;
  } else {
    vm.next_gc = vm.bytes_allocated + HEAP_GROW_THRESHOLD;
  }

  vm.prev_gc_freed = before - vm.bytes_allocated;

#ifdef DEBUG_GC_PHASE_TIMES
  print_phase_times();
#endif

#ifdef DEBUG_GC_WORKER_STATS
  gc_print_worker_stats();
#endif

#ifdef DEBUG_GC_HEAP_STATS
  print_heap_stats();
#endif
}
