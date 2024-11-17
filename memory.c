#include "memory.h"
#include <assert.h>
#include <pthread.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "compiler.h"
#include "vm.h"

#ifdef DEBUG_GC_PHASE_TIMES
#include <windows.h>
#endif

#ifdef DEBUG_GC_WORKER
#define GC_WORKER_LOG(...) printf(__VA_ARGS__)
#else
#define GC_WORKER_LOG(...)
#endif

#ifdef DEBUG_GC_SWEEP
#define GC_SWEEP_LOG(...) printf(__VA_ARGS__)
#else
#define GC_SWEEP_LOG(...)
#endif

typedef enum {
  TASK_CUSTOM,
  TASK_MARK_OBJ
} GCTaskType;

typedef struct GCTask {
  GCTaskType type;
  union {
    struct {  // For TASK_CUSTOM
      void (*function)(void* arg);
      void* arg;
    };
    Obj* obj;  // For TASK_MARK_OBJ
  };
} GCTask;

// Ring buffer for the deque
typedef struct RingBuf {
  int64_t capacity;
  int64_t mask;    // Bit mask for modulo operations
  GCTask* buffer;  // Array of tasks
} RingBuf;

// Work-stealing deque structure
typedef struct WorkStealingDeque {
  // Cache line alignment for atomic variables
  alignas(64) atomic_int_least64_t top;
  alignas(64) atomic_int_least64_t bottom;
  alignas(64) atomic_uintptr_t buffer;  // Points to RingBuf

  // Store old buffers for cleanup
  RingBuf** garbage;
  size_t garbage_size;
  size_t garbage_capacity;
} WorkStealingDeque;

#ifdef DEBUG_GC_WORKER_STATS
typedef struct {
  atomic_size_t objects_marked;
  atomic_size_t objects_swept;
  uint64_t work_steal_attempts;
  uint64_t successful_steals;
} GCWorkerStats;
#endif
typedef struct {
  alignas(64) WorkStealingDeque* deque;
  alignas(64) pthread_t thread;
  alignas(64) int id;
  alignas(64) atomic_bool done;
#ifdef DEBUG_GC_WORKER_STATS
  alignas(64) GCWorkerStats stats;
#endif
} GCWorker;

typedef struct {
  GCWorker* workers;
  int worker_count;
  atomic_bool shutdown;
  atomic_size_t object_count;  // For sweep parallelization

  // Add synchronization primitives
  atomic_bool should_work;
  HANDLE work_event;  // Event to signal work
} GCThreadPool;

typedef struct {
  union {
    ValueArray* array;
    HashTable* table;
  };
  int start;
  int end;
  enum {
    MARK_ARRAY,
    MARK_HASHTABLE
  } type;
} MarkRangeTaskArg;

// Forward declarations
static void blacken_object(Obj* object);
static void gc_wait_for_workers(void);
static void gc_submit_custom_task(void (*function)(void*), void* arg);
static void gc_submit_mark_task(Obj* object);
#ifdef DEBUG_GC_WORKER_STATS
static void gc_print_worker_stats();
#endif

// Thread-local storage
static __thread GCWorker* current_worker = NULL;

// Global state
static GCThreadPool gc_thread_pool = {0};

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
  vm.bytes_allocated += new_size - old_size;

  if (new_size > old_size && !(vm.flags & VM_FLAG_PAUSE_GC)) {
    // Only collect_garbage if we're not freeing memory
#ifdef DEBUG_STRESS_GC
    collect_garbage();
#else
    if (vm.bytes_allocated > vm.next_gc) {
      collect_garbage();
    }
#endif
  }

  if (new_size == 0) {  // Freeing
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, new_size);
  if (result == NULL) {
    INTERNAL_ERROR("Not enough memory to reallocate");
    exit(EMEM_ERROR);
  }

  return result;
}

// Create a new ring buffer with given capacity (must be power of 2)
static RingBuf* ring_buff_init(int64_t capacity) {
  assert(capacity > 0 && !(capacity & (capacity - 1)) && "Capacity must be power of 2!");

  RingBuf* rb = (RingBuf*)malloc(sizeof(RingBuf));
  if (!rb) {
    return NULL;
  }

  rb->capacity = capacity;
  rb->mask     = capacity - 1;
  rb->buffer   = (GCTask*)calloc(capacity, sizeof(GCTask));

  if (!rb->buffer) {
    free(rb);
    return NULL;
  }

  return rb;
}

// Store task at modulo index
static void ring_buff_store(RingBuf* rb, int64_t i, GCTask task) {
  rb->buffer[i & rb->mask] = task;
}

// Load task at modulo index
static GCTask ring_buff_load(RingBuf* rb, int64_t i) {
  return rb->buffer[i & rb->mask];
}

// Create a new ring buffer with double capacity and copy elements
static RingBuf* ring_buff_resize(RingBuf* rb, int64_t bottom, int64_t top) {
  RingBuf* new_rb = ring_buff_init(rb->capacity * 2);
  if (!new_rb) {
    return NULL;
  }

  for (int64_t i = top; i != bottom; ++i) {
    ring_buff_store(new_rb, i, ring_buff_load(rb, i));
  }

  return new_rb;
}

// Create a new work-stealing deque
WorkStealingDeque* ws_deque_init(int64_t initial_capacity) {
  WorkStealingDeque* deque = (WorkStealingDeque*)malloc(sizeof(WorkStealingDeque));
  if (!deque) {
    return NULL;
  }

  RingBuf* rb = ring_buff_init(initial_capacity);
  if (!rb) {
    free(deque);
    return NULL;
  }

  atomic_init(&deque->top, 0);
  atomic_init(&deque->bottom, 0);
  atomic_init(&deque->buffer, (uintptr_t)rb);

  deque->garbage_capacity = GC_DEQUE_INITIAL_GARBAGE_CAPACITY;
  deque->garbage          = (RingBuf**)malloc(sizeof(RingBuf*) * deque->garbage_capacity);
  deque->garbage_size     = 0;

  if (!deque->garbage) {
    free(rb->buffer);
    free(rb);
    free(deque);
    return NULL;
  }

  return deque;
}

// Get current size
size_t ws_deque_size(WorkStealingDeque* deque) {
  int64_t b = atomic_load_explicit(&deque->bottom, memory_order_relaxed);
  int64_t t = atomic_load_explicit(&deque->top, memory_order_relaxed);
  return b >= t ? (size_t)(b - t) : 0;
}

size_t ws_deque_capacity(WorkStealingDeque* deque) {
  return ((RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed))->capacity;
}

// Check if empty
bool ws_deque_empty(WorkStealingDeque* deque) {
  return ws_deque_size(deque) == 0;
}

// Push a task (only owner thread)
bool ws_deque_push(WorkStealingDeque* deque, GCTask task) {
  int64_t b   = atomic_load_explicit(&deque->bottom, memory_order_relaxed);
  int64_t t   = atomic_load_explicit(&deque->top, memory_order_acquire);
  RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);

  if (rb->capacity < (b - t) + 1) {
    RingBuf* new_rb = ring_buff_resize(rb, b, t);
    if (!new_rb)
      return false;

    if (deque->garbage_size == deque->garbage_capacity) {
      size_t new_capacity   = deque->garbage_capacity * 2;
      RingBuf** new_garbage = (RingBuf**)realloc(deque->garbage, sizeof(RingBuf*) * new_capacity);
      if (!new_garbage) {
        free(new_rb->buffer);
        free(new_rb);
        return false;
      }
      deque->garbage          = new_garbage;
      deque->garbage_capacity = new_capacity;
    }

    deque->garbage[deque->garbage_size++] = rb;
    atomic_store_explicit(&deque->buffer, (uintptr_t)new_rb, memory_order_relaxed);
    rb = new_rb;
  }

  ring_buff_store(rb, b, task);
  atomic_thread_fence(memory_order_release);
  atomic_store_explicit(&deque->bottom, b + 1, memory_order_relaxed);
  return true;
}

// Pop a task (only owner thread)
bool ws_deque_pop(WorkStealingDeque* deque, GCTask* task) {
  int64_t b   = atomic_load_explicit(&deque->bottom, memory_order_relaxed) - 1;
  RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);

  atomic_store_explicit(&deque->bottom, b, memory_order_relaxed);
  atomic_thread_fence(memory_order_seq_cst);

  int64_t t    = atomic_load_explicit(&deque->top, memory_order_relaxed);
  bool success = false;

  if (t <= b) {
    if (t == b) {
      if (!atomic_compare_exchange_strong_explicit(&deque->top, &t, t + 1, memory_order_seq_cst, memory_order_relaxed)) {
        success = false;
      } else {
        *task   = ring_buff_load(rb, b);
        success = true;
      }
      atomic_store_explicit(&deque->bottom, b + 1, memory_order_relaxed);
    } else {
      *task   = ring_buff_load(rb, b);
      success = true;
    }
  } else {
    atomic_store_explicit(&deque->bottom, b + 1, memory_order_relaxed);
    success = false;
  }

  return success;
}

// Steal a task (any thread)
bool ws_deque_steal(WorkStealingDeque* deque, GCTask* task) {
  int64_t t = atomic_load_explicit(&deque->top, memory_order_acquire);
  atomic_thread_fence(memory_order_seq_cst);
  int64_t b = atomic_load_explicit(&deque->bottom, memory_order_acquire);

  if (t < b) {
    RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_consume);
    *task       = ring_buff_load(rb, t);

    if (!atomic_compare_exchange_strong_explicit(&deque->top, &t, t + 1, memory_order_seq_cst, memory_order_relaxed)) {
      return false;
    }

    return true;
  }

  return false;
}

// Cleanup and destroy the deque
void ws_deque_free(WorkStealingDeque* deque) {
  if (!deque) {
    return;
  }

  RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);
  free(rb->buffer);
  free(rb);

  for (size_t i = 0; i < deque->garbage_size; i++) {
    free(deque->garbage[i]->buffer);
    free(deque->garbage[i]);
  }

  free(deque->garbage);
  free(deque);
}

void mark_obj(Obj* object) {
  // Unnecessary NULL check if we're called from mark_value, but if called from
  // elsewhere, the object arg could be NULL.
  if (object == NULL) {
    return;
  }

  // Atomically check and set the is_marked flag
  if (!atomic_exchange(&object->is_marked, true)) {
#ifdef DEBUG_GC_WORKER_STATS
    atomic_fetch_add(&current_worker->stats.objects_marked, 1);
#endif
    gc_submit_mark_task(object);  // TODO: Test blacken_object directly
  }
}

void mark_value(Value value) {
  if (!is_primitive(value)) {
    mark_obj(value.as.obj);
  }
}

static void mark_range_task(void* arg) {
  MarkRangeTaskArg* task_arg = (MarkRangeTaskArg*)arg;

  if (task_arg->type == MARK_ARRAY) {
    ValueArray* array = task_arg->array;
    for (int i = task_arg->start; i < task_arg->end; i++) {
      mark_value(array->values[i]);
    }
  } else {
    HashTable* table = task_arg->table;
    for (int i = task_arg->start; i < task_arg->end; i++) {
      Entry* entry = &table->entries[i];
      if (!is_empty_internal(entry->key)) {
        mark_value(entry->key);
        mark_value(entry->value);
      }
    }
  }

  free(task_arg);
}

void parallel_mark_array(ValueArray* array) {
  static_assert(GC_PARALLEL_MARK_ARRAY_THRESHOLD > GC_THREAD_COUNT * 200,
                "Probably not worth parallelizing if a chunk would be smaller than 200 elements. ");
  int num_chunks = gc_thread_pool.worker_count;
  int chunk_size = (array->count + num_chunks - 1) / num_chunks;

  // Generate tasks in our own deque, since we can't push to other deques directly. Other workers will steal from us.
  for (int i = 0; i < num_chunks; i++) {
    int start = i * chunk_size;
    if (start >= array->count) {
      break;
    }

    int end = MIN(start + chunk_size, array->count);

    MarkRangeTaskArg* arg = malloc(sizeof(MarkRangeTaskArg));
    arg->array            = array;
    arg->start            = start;
    arg->end              = end;
    arg->type             = MARK_ARRAY;

    gc_submit_custom_task(mark_range_task, arg);
  }
}

// Marks all entries in a value array gray.
void mark_array(ValueArray* array) {
  if (array->count < GC_PARALLEL_MARK_ARRAY_THRESHOLD) {
    for (int i = 0; i < array->count; i++) {
      mark_value(array->values[i]);
    }
  } else {
    parallel_mark_array(array);
  }
}

void parallel_mark_hashtable(HashTable* table) {
  static_assert(GC_PARALLEL_MARK_HASHTABLE_THRESHOLD > GC_THREAD_COUNT * 100,
                "Probably not worth parallelizing if a chunk would be smaller than 100 elements. ");
  int num_chunks = gc_thread_pool.worker_count;
  int chunk_size = (table->capacity + num_chunks - 1) / num_chunks;

  // Generate tasks in our own deque, since we can't push to other deques directly. Other workers will steal from us.
  for (int i = 0; i < num_chunks; i++) {
    int start = i * chunk_size;
    if (start >= table->capacity) {
      break;
    }

    int end = MIN(start + chunk_size, table->capacity);

    MarkRangeTaskArg* arg = malloc(sizeof(MarkRangeTaskArg));
    arg->table            = table;
    arg->start            = start;
    arg->end              = end;
    arg->type             = MARK_HASHTABLE;

    gc_submit_custom_task(mark_range_task, arg);
  }
}

void mark_hashtable(HashTable* table) {
  if (table->count < GC_PARALLEL_MARK_HASHTABLE_THRESHOLD) {
    for (int i = 0; i < table->capacity; i++) {
      Entry* entry = &table->entries[i];
      if (!is_empty_internal(entry->key)) {
        mark_value(entry->key);
        mark_value(entry->value);
      }
    }
  } else {
    parallel_mark_hashtable(table);
  }
}

// Blackens an object by marking all objects it references.
// A black object is one that has been marked and all objects it references have
// been marked as well.
static void blacken_object(Obj* object) {
  switch (object->type) {
    case OBJ_GC_BOUND_METHOD: {
      ObjBoundMethod* bound = (ObjBoundMethod*)object;
      mark_value(bound->receiver);
      mark_obj((Obj*)bound->method);
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
  atomic_fetch_add_explicit(&gc_thread_pool.object_count, 1, memory_order_relaxed);

  object->next = vm.objects;
  vm.objects   = object;

  return object;
}

// Frees an object from our heap.
// How we free an object depends on its type.
static void free_object(Obj* object) {
  // Atomically decrement the object count
  atomic_fetch_sub(&gc_thread_pool.object_count, 1);

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

void free_heap() {
  Obj* object = vm.objects;
  while (object != NULL) {
    Obj* next = object->next;
    free_object(object);
    object = next;
  }
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

  // And the builtin object.
  mark_obj((Obj*)vm.builtin);

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

static void sweep_sequential() {
  GC_SWEEP_LOG(ANSI_RED_STR("[GC]") " " ANSI_CYAN_STR("[SWEEP]") " Sweeping heap sequential\n");
  GC_SWEEP_LOG("  Sweeping %zu objects\n", atomic_load(&gc_thread_pool.object_count));

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

#ifdef DEBUG_GC_WORKER_STATS
      atomic_fetch_add(&current_worker->stats.objects_swept, 1);
#endif

      free_object(unreached);
    }
  }

  GC_SWEEP_LOG("  Done sweeping\n\n");
}

// TODO: Does this need to be aligned?
typedef struct {
  alignas(64) Obj* start;   // Inclusive. If NULL, then the whole chunk was freed
  alignas(64) Obj* end;     // Inclusive too
  alignas(64) size_t size;  // Number of objects in the chunk. E.g. size=4 means 0=start, 1, 2, 3=end
} SweepChunk;

static void sweep_chunk_task(void* arg) {
  SweepChunk* chunk = (SweepChunk*)arg;

  size_t chunk_size = chunk->size;
  Obj* current      = chunk->start;

  Obj* previous  = NULL;
  Obj* new_start = NULL;
  Obj* new_end   = NULL;

  GC_SWEEP_LOG("  Worker %d: Sweeping chunk %p-%p (%zu objects)\n", current_worker->id, chunk->start, chunk->end, chunk->size);

#ifdef DEBUG_GC_WORKER_STATS
  size_t freed = 0;
#endif

  assert(current != NULL && "Object count mismatch");  // TODO (optimize): Remove
  for (size_t i = 0; i < chunk_size; i++) {
    // Atomically check and reset the is_marked flag
    if (atomic_exchange(&current->is_marked, false)) {  // Object is marked (black), keep it
      // Move new_start to the first marked object
      if (new_start == NULL) {
        new_start = current;
      }

      new_end = current;  // Move end to the last marked object

      previous = current;
      current  = current->next;
    } else {  // Object is unmarked (white), remove it
      Obj* unreached = current;
      current        = current->next;

      // Update the linked list
      if (previous != NULL) {
        previous->next = current;
      }

      free_object(unreached);
#ifdef DEBUG_GC_WORKER_STATS
      freed++;
#endif
    }
  }

  // Update chunk boundaries for list reconstruction
  chunk->start = new_start;  // Could be NULL if all objects in chunk were freed
  chunk->end   = new_end;    // Could be NULL if all objects in chunk were freed

  GC_SWEEP_LOG("  Worker %d: Done sweeping\n", current_worker->id);
#ifdef DEBUG_GC_WORKER_STATS
  // Update worker stats
  atomic_fetch_add(&current_worker->stats.objects_swept, freed);
#endif
}

// Sweeps the heap and frees all unmarked objects. This is done by iterating over the linked list of objects and freeing all
// white objects. White objects are objects that have not been marked during the mark phase and are therefore unreachable.
static void sweep() {
  size_t total_object_count = atomic_load(&gc_thread_pool.object_count);

  if (total_object_count <= GC_PARALLEL_SWEEP_THRESHOLD) {
    sweep_sequential();
    return;
  }

  GC_SWEEP_LOG(ANSI_RED_STR("[GC]") " " ANSI_CYAN_STR("[SWEEP]") " Sweeping heap parallel\n");

  // Calculate base chunk size and remainder
  static_assert(
      GC_PARALLEL_SWEEP_THRESHOLD > ((2 /* 2 objs per chunk */ * GC_THREAD_COUNT) * 2 /* we want 2*threadcount chunks */),
      "We must have at least two objects per chunk so we can guarantee that start and end are different");
  int num_chunks         = gc_thread_pool.worker_count * 2;  // *2, so *maybe* the main thread can help out after generating tasks
  size_t base_chunk_size = total_object_count / num_chunks;
  size_t remainder       = total_object_count % num_chunks;

  GC_SWEEP_LOG("  Sweeping %zu objects in %d chunks of size %zu with remainder %zu\n", total_object_count, num_chunks,
               base_chunk_size, remainder);

  // Allocate chunks on heap since they'll be accessed asynchronously
  SweepChunk** chunks = malloc(sizeof(SweepChunk) * num_chunks);
  if (chunks == NULL) {
    INTERNAL_ERROR("Failed to allocate memory for sweep chunks, falling back to sequential sweep");
    sweep_sequential();  // Fallback to sequential if allocation fails
    return;
  }

  // Distribute objects into chunks
  Obj* current = vm.objects;
  for (int chunk_i = 0; chunk_i < num_chunks; chunk_i++) {
    chunks[chunk_i] = malloc(sizeof(SweepChunk));
    if (chunks[chunk_i] == NULL) {
      INTERNAL_ERROR("Failed to allocate memory for sweep chunk.");
      exit(EMEM_ERROR);
      return;
    }

    chunks[chunk_i]->start = current;
    chunks[chunk_i]->size  = base_chunk_size;

    if (remainder > 0) {  // Since remainder is total_object_count % num_chunks, it's always less than num_chunks
      chunks[chunk_i]->size++;
      remainder--;
    }

    // Move to end of this chunk (exclusive)
    for (size_t i = 0; i < chunks[chunk_i]->size - 1; i++) {
      current = current->next;
    }

    chunks[chunk_i]->end = current;
    current              = current->next;  // Move to start of next chunk

    // Submit task to worker
    GC_SWEEP_LOG("  Worker %d: Submitting sweep task for chunk #%d %p-%p  (%zu objects)\n", current_worker->id, chunk_i,
                 chunks[chunk_i]->start, chunks[chunk_i]->end, chunks[chunk_i]->size);
    gc_submit_custom_task(sweep_chunk_task, chunks[chunk_i]);
  }
  assert(current == NULL &&
         "Object count mismatch, current should be at the end of the obj linked-list");  // TODO (optimize): Remove

  // Wait for workers and pause them
  gc_wait_for_workers();

  // TODO: Return here if no objects were freed

  // Stitch the list back together, being careful about NULLs
  vm.objects = NULL;
  Obj* tail  = NULL;
  for (int chunk_i = 0; chunk_i < num_chunks; chunk_i++) {
    if (chunks[chunk_i]->start == NULL) {
      continue;  // Skip empty chunks
    }
    if (vm.objects == NULL) {
      vm.objects = chunks[chunk_i]->start;  // Pick up the first non-NULL chunk
    } else {
      tail->next = chunks[chunk_i]->start;
    }
    tail = chunks[chunk_i]->end;
    free(chunks[chunk_i]);
  }

  free(chunks);

  GC_SWEEP_LOG("  Done sweeping heap parallel\n\n");
}

#ifdef DEBUG_GC_PHASE_TIMES
static double mutator_stop_time = 0;
static int gc_cycle_count       = 0;

static double get_time() {
  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);

  return (double)now.QuadPart / frequency.QuadPart;
}

#define DEBUG_GC_PHASE_TIMESTAMP(varname) double varname = get_time();

static void print_phase_times(double start,
                              double mark_roots_time,
                              double mark_phase_time,
                              double remove_white_time,
                              double sweep_time,
                              double mutator_time) {
  printf(ANSI_RED_STR(
      "[GC]") " " ANSI_MAGENTA_STR("[Cycle %d]")" Phase Times"
              ":\n", gc_cycle_count++);
  printf("  Mutator execution time:        %fms\n", (mutator_time) * 1000);
  printf("  1) Mark roots time:            %fms\n", (mark_roots_time - start) * 1000);
  printf("     Mark phase time:            %fms\n", (mark_phase_time - mark_roots_time) * 1000);
  printf("  2) Remove white strings time:  %fms\n", (remove_white_time - mark_phase_time) * 1000);
  printf("  3) Sweep time:                 %fms\n", (sweep_time - remove_white_time) * 1000);
  printf("  Total GC pause time:           " ANSI_CYAN_STR("%fms") "\n", (sweep_time - start) * 1000);
  printf("\n");
}
#else
#define DEBUG_GC_PHASE_TIMESTAMP(varname)
#endif

void collect_garbage() {
  // TODO:  Parallelization
  //   TODO: Make a mark() function that decides whether to mark sequentially or in parallel
  //   TODO: Refactor synchronization - use the simple approach of testing if there any tasks remaining (See claude output)
  //   TODO: Cleanup gc_wait_for_workers() and gc_worker() - maybe we can abstract the worker loop into a function
  //   TODO: Move deque operations to a separate file
  //   TODO: Remove GCWorker param from gc_submit_custom_task and gc_submit_mark_task, because we can only assign to the current
  //   worker.
  //   TODO: Do the TODOs sprinkled throughout the code
  //   TODO: Use function pointers to determine which mark/sweep function to use (sequential or parallel)
  //   TODO: Learn if malloc_aligned / free_aligned is actually required...?
#ifdef DEBUG_GC_PHASE_TIMES
  DEBUG_GC_PHASE_TIMESTAMP(start);
  double mutator_execution_time = start - mutator_stop_time;
  mutator_stop_time             = start;
#endif

  size_t before = vm.bytes_allocated;

  // Main thread acts as worker 0 for a GC cycle.
  current_worker = &gc_thread_pool.workers[0];

  // Step 1: Mark roots - this is single-threaded. Generates work for workers, essentially each mark-task can be viewed as a
  // as a gray object that needs to be blackened.
  // It's important that all root objects get marked in this phase - if you e.g. had a root which is a long array which gets split
  // into different mark tasks and distributed between the workers, that'd be a problem because the workers are idle until after
  // mark_roots, leaving us with not all roots marked.
  atomic_store(&gc_thread_pool.should_work, true);
  SetEvent(gc_thread_pool.work_event);

  mark_roots();
  DEBUG_GC_PHASE_TIMESTAMP(mark_roots_time);
  gc_wait_for_workers();
  DEBUG_GC_PHASE_TIMESTAMP(mark_phase_time);

  // Step 2: Handle interned strings
  hashtable_remove_white(&vm.strings);
  DEBUG_GC_PHASE_TIMESTAMP(remove_white_time);

  // Step 3: Sweep the heap
  sweep();
  DEBUG_GC_PHASE_TIMESTAMP(sweep_time);

  atomic_store(&gc_thread_pool.should_work, false);
  current_worker = NULL;

  // Update GC thresholds
  if (vm.bytes_allocated < GC_HEAP_GROW_THRESHOLD) {
    vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;
  } else {
    vm.next_gc = vm.bytes_allocated + GC_HEAP_GROW_THRESHOLD;
  }

  vm.prev_gc_freed = before - vm.bytes_allocated;

#ifdef DEBUG_GC_PHASE_TIMES
  print_phase_times(start, mark_roots_time, mark_phase_time, remove_white_time, sweep_time, mutator_execution_time);
#endif

#ifdef DEBUG_GC_WORKER_STATS
  gc_print_worker_stats();
#endif
}

static void gc_process_task(GCTask task) {
#ifdef DEBUG_GC_WORKER
  if (current_worker->id == 0) {
    GC_WORKER_LOG("Main thread executing task %p of type %d\n", (void*)&task, task.type);
  } else {
    GC_WORKER_LOG("Worker %d executing task %p of type %d\n", current_worker->id, (void*)&task, task.type);
  }
#endif
  switch (task.type) {
    case TASK_MARK_OBJ: blacken_object(task.obj); break;
    case TASK_CUSTOM: task.function(task.arg); break;
    default: INTERNAL_ERROR("Unknown task type: %d", task.type); break;
  }
}

static bool gc_worker_complete_work() {
  // Do our own work
  GCTask task;
  bool have_task = ws_deque_pop(current_worker->deque, &task);

  // Try to steal work if we don't have any of our own
  if (!have_task) {
    for (int attempt = 0; attempt < GC_WORKER_MAX_STEAL_ATTEMPTS; attempt++) {
      for (int i = 0; i < gc_thread_pool.worker_count; i++) {
        if (i == current_worker->id) {
          continue;
        }

#ifdef DEBUG_GC_WORKER_STATS
        current_worker->stats.work_steal_attempts++;
#endif
        if (ws_deque_steal(gc_thread_pool.workers[i].deque, &task)) {
#ifdef DEBUG_GC_WORKER_STATS
          current_worker->stats.successful_steals++;
#endif
          have_task = true;
          break;
        }
      }
      if (have_task) {
        break;
      }
    }
  }

  if (have_task) {
    gc_process_task(task);
  }

  return !have_task;
}

static void gc_wait_for_workers() {
  bool all_done;
  do {
    //     // Do all of our own work
    //     GCTask task;
    //     while (ws_deque_pop(current_worker->deque, &task)) {
    //       gc_process_task(task);
    //     }

    //     for (int i = 1; i < gc_thread_pool.worker_count; i++) {
    // #ifdef DEBUG_GC_WORKER_STATS
    //       current_worker->stats.work_steal_attempts++;
    // #endif
    //       if (ws_deque_steal(gc_thread_pool.workers[i].deque, &task)) {
    // #ifdef DEBUG_GC_WORKER_STATS
    //         current_worker->stats.successful_steals++;
    // #endif
    //         gc_process_task(task);
    //         break;  // Only process one stolen task per iteration
    //       }
    //     }
    if (!gc_worker_complete_work()) {
      continue;
    }

    // Check if all workers are done - skip worker 0, since that's us
    all_done = true;
    for (int i = 1; i < gc_thread_pool.worker_count; i++) {
      if (!atomic_load(&gc_thread_pool.workers[i].done)) {
        all_done = false;
        break;
      }
    }
    // TODO: Check if we can just set all_done to true here
  } while (!all_done);
}

static void* gc_worker(void* arg) {
  GCWorker* worker = (GCWorker*)arg;
  current_worker   = worker;
  GC_WORKER_LOG("Worker %d started\n", worker->id);

  while (!atomic_load(&gc_thread_pool.shutdown)) {
    // Between GC cycles - deep sleep with no busy waiting
    if (!atomic_load(&gc_thread_pool.should_work)) {
      WaitForSingleObject(gc_thread_pool.work_event, INFINITE);
      continue;
    }

    atomic_store(&worker->done, false);
    if (gc_worker_complete_work()) {
      atomic_store(&worker->done, true);
      usleep(1000);
    }
  }

  GC_WORKER_LOG("Worker %d shutting down\n", worker->id);
  return NULL;
}

static void gc_submit_task(GCTask task) {
  atomic_store(&current_worker->done, false);
  if (!ws_deque_push(current_worker->deque, task)) {
    INTERNAL_ERROR("Failed to push task to current worker's deque");
    exit(EMEM_ERROR);
  }
}

static void gc_submit_mark_task(Obj* object) {
  gc_submit_task((GCTask){.type = TASK_MARK_OBJ, .obj = object});
}

static void gc_submit_custom_task(void (*function)(void*), void* arg) {
  gc_submit_task((GCTask){.type = TASK_CUSTOM, .function = function, .arg = arg});
}

// Monitoring and statistics
#ifdef DEBUG_GC_WORKER_STATS
static void gc_print_worker_stats() {
  printf(ANSI_RED_STR("[GC]") " Detailed Worker Statistics:\n");
  printf("  " ANSI_GREEN_STR("Per-worker statistics") ":\n");

  // Print header with proper alignment
  printf("    %-6s  %-12s  %-12s  %-12s  %-12s  %-12s  %-12s\n", "Worker", "Marked", "Swept", "Steal Tries", "Steals",
         "Success Rate", "Deque Cap");
  printf("    %-6s  %-12s  %-12s  %-12s  %-12s  %-12s  %-12s\n", "------", "------", "-----", "-----------", "------",
         "------------", "---------");

  size_t total_marked = 0;
  size_t total_swept  = 0;

  for (int i = 0; i < gc_thread_pool.worker_count; i++) {
    GCWorker* worker  = &gc_thread_pool.workers[i];
    size_t marked     = atomic_load(&worker->stats.objects_marked);
    size_t swept      = atomic_load(&worker->stats.objects_swept);
    uint64_t attempts = worker->stats.work_steal_attempts;
    uint64_t steals   = worker->stats.successful_steals;
    int64_t deque_cap = ws_deque_capacity(worker->deque);
    double rate       = (attempts > 0) ? (steals * 100.0 / attempts) : 0.0;

    printf("    " ANSI_CYAN_STR("%-6d") "  %-12zu  %-12zu  %-12llu  %-12llu  %11.2f%%  %-12lld\n", i, marked, swept, attempts,
           steals, rate, deque_cap);

    total_marked += marked;
    total_swept += swept;

    // Reset stats
    atomic_store(&worker->stats.objects_marked, 0);
    atomic_store(&worker->stats.objects_swept, 0);
    worker->stats.work_steal_attempts = 0;
    worker->stats.successful_steals   = 0;
  }

  // Totals
  printf("    %-6s  %-12s  %-12s\n", "------", "------", "-----");
  printf("    " ANSI_CYAN_STR("%-6s") "  %-12zu  %-12zu\n", "Total", total_marked, total_swept);

  printf("\n");
  printf("  " ANSI_GREEN_STR("Heap statistics") ":\n");
  printf("    Total objects:                %zu\n", atomic_load(&gc_thread_pool.object_count));
  printf("    Bytes allocated (Heap size):  %zuB / %.2fMB / %.2fGB\n", vm.bytes_allocated, vm.bytes_allocated / (1024.0 * 1024.0),
         vm.bytes_allocated / (1024.0 * 1024.0 * 1024.0));
  printf("    Bytes collected (This cycle): %zuB / %.2fMB / %.2fGB\n", vm.prev_gc_freed, vm.prev_gc_freed / (1024.0 * 1024.0),
         vm.prev_gc_freed / (1024.0 * 1024.0 * 1024.0));
  printf("    Next GC at:                   %zuB / %.2fMB / %.2fGB\n", vm.next_gc, vm.next_gc / (1024.0 * 1024.0),
         vm.next_gc / (1024.0 * 1024.0 * 1024.0));
}
#endif

void gc_init_thread_pool(int num_threads) {
  GC_WORKER_LOG("Initializing thread pool with %d threads\n", num_threads);

#ifdef DEBUG_GC_PHASE_TIMES
  mutator_stop_time = get_time();
#endif

  if (num_threads <= 0) {
    INTERNAL_ERROR("Invalid number of threads: %d", num_threads);
    exit(ESW_ERROR);
  }

  // Prioritize main thread
  HANDLE main_thread_handle = GetCurrentThread();
  if (!SetThreadPriority(main_thread_handle, THREAD_PRIORITY_HIGHEST)) {
    // Log warning but continue - not critical
    GC_WORKER_LOG("Warning: Could not set main thread priority: %lu\n", GetLastError());
  }
  DWORD_PTR main_thread_affinity_mask = 1ULL;  // Assign main thread to core 0
  if (!SetThreadAffinityMask(main_thread_handle, main_thread_affinity_mask)) {
    // Log warning but continue - not critical
    GC_WORKER_LOG("Warning: Could not set main thread affinity: %lu\n", GetLastError());
  }

  gc_thread_pool.worker_count = num_threads;
  size_t alloc_size           = num_threads * sizeof(GCWorker);

  gc_thread_pool.workers = _aligned_malloc(alloc_size, 64);
  if (!gc_thread_pool.workers) {
    INTERNAL_ERROR("Failed to allocate memory for worker threads");
    exit(EMEM_ERROR);
  }
  memset(gc_thread_pool.workers, 0, alloc_size);

  size_t object_count = 0;
  for (Obj* object = vm.objects; object != NULL; object = object->next) {
    object_count++;
  }

  atomic_init(&gc_thread_pool.shutdown, false);
  atomic_init(&gc_thread_pool.object_count, object_count);

  // Initialize sync primitives
  atomic_init(&gc_thread_pool.should_work, false);                   // TODO: We can concat the thread creation loops again
  gc_thread_pool.work_event = CreateEvent(NULL, TRUE, FALSE, NULL);  // Manual reset event

  // Initialize workers
  for (int i = 0; i < num_threads; i++) {
    GC_WORKER_LOG("Initializing worker %d\n", i);
    gc_thread_pool.workers[i].id = i;
    atomic_init(&gc_thread_pool.workers[i].done, false);

#ifdef DEBUG_GC_WORKER_STATS
    // Initialize worker stats
    atomic_init(&gc_thread_pool.workers[i].stats.objects_marked, 0);
    atomic_init(&gc_thread_pool.workers[i].stats.objects_swept, 0);
    gc_thread_pool.workers[i].stats.work_steal_attempts = 0;
    gc_thread_pool.workers[i].stats.successful_steals   = 0;
#endif
    gc_thread_pool.workers[i].deque = ws_deque_init(GC_DEQUE_INITIAL_CAPACITY);
    if (!gc_thread_pool.workers[i].deque) {
      INTERNAL_ERROR("Failed to initialize work-stealing deque for worker %d", i);
      gc_shutdown_thread_pool();
      exit(EMEM_ERROR);
    }
  }

  // Create worker threads, main thread is worker 0, so we skip that
  for (int i = 1; i < num_threads; i++) {
    int result = pthread_create(&gc_thread_pool.workers[i].thread, NULL, gc_worker, &gc_thread_pool.workers[i]);
    if (result != 0) {
      INTERNAL_ERROR("Failed to create worker thread %d: %s", i, strerror(result));
      gc_shutdown_thread_pool();
      exit(ESW_ERROR);
    }
    HANDLE thread_handle = (HANDLE)gc_thread_pool.workers[i].thread;
    if (!SetThreadPriority(thread_handle, THREAD_PRIORITY_ABOVE_NORMAL)) {
      // Log warning but continue - not critical
      GC_WORKER_LOG("Warning: Could not set thread priority: %lu\n", GetLastError());
    }
    // Affinity
    DWORD_PTR affinity_mask = (1ULL) << i;  // Assign each worker to a different core
    if (!SetThreadAffinityMask(thread_handle, affinity_mask)) {
      // Log warning but continue - not critical
      GC_WORKER_LOG("Warning: Could not set thread affinity: %lu\n", GetLastError());
    }
  }
}

void gc_shutdown_thread_pool() {
  if (!gc_thread_pool.workers) {
    return;
  }

  atomic_store(&gc_thread_pool.shutdown, true);
  atomic_store(&gc_thread_pool.should_work, true);

  // Wake up all workers so they can see shutdown flag
  SetEvent(gc_thread_pool.work_event);

  // Join only threads 1 onwards (not worker[0] which is main thread)
  for (int i = 1; i < gc_thread_pool.worker_count; i++) {
    pthread_join(gc_thread_pool.workers[i].thread, NULL);
  }

  // Free sync resources
  CloseHandle(gc_thread_pool.work_event);

  // Free all deques including worker[0]'s
  for (int i = 0; i < gc_thread_pool.worker_count; i++) {
    ws_deque_free(gc_thread_pool.workers[i].deque);
  }

  _aligned_free(gc_thread_pool.workers);
}
