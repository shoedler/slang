#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "vm.h"

#if defined(DEBUG_LOG_GC) || defined(DEBUG_LOG_GC_FREE)
#include <stdio.h>
#include "debug.h"
#include "value.h"
#endif

#ifdef DEBUG_LOG_GC_FREE
#include "string.h"
#endif

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
  vm.bytes_allocated += new_size - old_size;

  if (new_size > old_size && !(vm.flags & VM_FLAG_PAUSE_GC)) {
    // Only collect_garbage if we're not freeing memory
#ifdef DEBUG_STRESS_GC
    collect_garbage();
#endif
    if (vm.bytes_allocated > vm.next_gc) {
      collect_garbage();
    }
  }

  if (new_size == 0) {
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

void mark_obj(Obj* object) {
  // Unnecessary NULL check if we're called from mark_value, but if called from
  // elsewhere, the object arg could be NULL.
  if (object == NULL) {
    return;
  }

  if (object->is_marked) {
    return;
  }

#ifdef DEBUG_LOG_GC
  printf(ANSI_RED_STR("[GC] ") ANSI_YELLOW_STR("[MARK] ") "at %p, ", (void*)object);
  printf("%s\n", typeof_(OBJ_GC_VAL(object))->name->chars);
#endif

  object->is_marked = true;

  if (vm.gray_capacity < vm.gray_count + 1) {
    vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);
    vm.gray_stack    = (Obj**)realloc(vm.gray_stack, sizeof(Obj*) * vm.gray_capacity);

    if (vm.gray_stack == NULL) {
      INTERNAL_ERROR("Not enough memory to reallocate gray stack");
      exit(EMEM_ERROR);
    }
  }

  vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(Value value) {
  if (!is_primitive(value)) {
    mark_obj(value.as.obj);
  }
}

// Marks all entries in a value array gray.
void mark_array(ValueArray* array) {
  for (int i = 0; i < array->count; i++) {
    mark_value(array->values[i]);
  }
}

// Blackens an object by marking all objects it references.
// A black object is one that has been marked and all objects it references have
// been marked as well.
static void blacken_object(Obj* object) {
#ifdef DEBUG_LOG_GC
  printf(ANSI_RED_STR("[GC] ") ANSI_BLUE_STR("[BLACKEN] ") "at %p, ", (void*)object);
  printf("%s\n", typeof_(OBJ_GC_VAL(object))->name->chars);
#endif

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

// Frees an object from our heap.
// How we free an object depends on its type.
static void free_object(Obj* object) {
#ifdef DEBUG_LOG_GC_FREE
  printf(ANSI_RED_STR("[GC] ") ANSI_GREEN_STR("[FREE] ") "at %p, type: %d\n", (void*)object, object->type);
#endif

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
    default: INTERNAL_ERROR("Don't know how to free unknown object type: %d", object->type);
  }
}

void free_objects() {
  Obj* object = vm.objects;
  while (object != NULL) {
    Obj* next = object->next;
    free_object(object);
    object = next;
  }

  free(vm.gray_stack);
}

// Starts at the roots of the objects in the heap and marks all reachable objects.
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

  // Also mark the active module, if there is one and the table of loaded modules.
  mark_hashtable(&vm.modules);
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

// Traces all references from the gray stack and marks them black e.g. marking
// all objects referenced by the gray object. Since the gray object is no black,
// it's popped from the gray stack.
static void trace_references() {
  while (vm.gray_count > 0) {
    Obj* object = vm.gray_stack[--vm.gray_count];
    blacken_object(object);
  }
}

// Sweeps the heap and frees all unmarked objects.
// This is done by iterating over the linked list of objects and freeing all
// white objects. White objects are objects that have not been marked during the
// mark phase and are therefore unreachable.
static void sweep() {
  Obj* previous = NULL;
  Obj* object   = vm.objects;

  while (object != NULL) {
    if (object->is_marked) {
      object->is_marked = false;  // Unmark for next gc cycle
      previous          = object;
      object            = object->next;
    } else {
      Obj* unreached = object;

      object = object->next;
      if (previous != NULL) {
        previous->next = object;
      } else {
        vm.objects = object;
      }

      free_object(unreached);  // 🎉
    }
  }
}

void collect_garbage() {
#ifdef DEBUG_LOG_GC
  printf("== Gc begin collect ==\n");
#endif

  size_t before = vm.bytes_allocated;

  mark_roots();
  trace_references();
  // Remove all white entries from the vm's interned strings hashtable. This has
  // to be done after the mark phase, but before the sweep phase. This allows us
  // to just remove all white string objects from the table, because they are
  // not referenced by any other object. Doing this before the sweep phase is
  // important, because white string objects would be freed during the sweep and
  // leave us with dangling pointers in the hashtable.
  hashtable_remove_white(&vm.strings);
  sweep();

  // If we reach the threshold, we grow the heap by adding a fixed amount of
  // bytes. This ties next_gc down to earth, so it
  // doesn't grow into the abyss of the space-time continuum.
  if (vm.bytes_allocated < GC_HEAP_GROW_THRESHOLD) {
    vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;
  } else {
    vm.next_gc = vm.bytes_allocated + GC_HEAP_GROW_THRESHOLD;
  }

  vm.prev_gc_freed = before - vm.bytes_allocated;

#ifdef DEBUG_LOG_GC
  printf(ANSI_RED_STR("[GC] ") "Done. collected %zu bytes (from %zu to %zu) next at %zu\n", vm.prev_gc_freed, before,
         vm.bytes_allocated, vm.next_gc);
  printf("== Gc end collect ==\n");
#endif
}
