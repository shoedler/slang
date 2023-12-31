#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
  vm.bytes_allocated += new_size - old_size;

  if (new_size > old_size) {
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
    // TODO (recovery): Handle out of memory
    INTERNAL_ERROR("Not enough memory to reallocate");
    exit(70);
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
  printf(ANSI_RED_STR("[GC] ") ANSI_YELLOW_STR("[MARK] ") "%p, ",
         (void*)object);
  print_value(OBJ_VAL(object));
  printf("\n");
#endif

  object->is_marked = true;

  if (vm.gray_capacity < vm.gray_count + 1) {
    vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);
    vm.gray_stack =
        (Obj**)realloc(vm.gray_stack, sizeof(Obj*) * vm.gray_capacity);

    // TODO (recovery): Handle out of memory
    if (vm.gray_stack == NULL) {
      INTERNAL_ERROR("Not enough memory to reallocate gray stack");
      exit(70);
    }
  }

  vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(Value value) {
  if (IS_OBJ(value)) {
    mark_obj(AS_OBJ(value));
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
  printf(ANSI_RED_STR("[GC] ") ANSI_BLUE_STR("[BLACKEN] ") "%p, ",
         (void*)object);
  print_value(OBJ_VAL(object));
  printf("\n");
#endif

  switch (object->type) {
    case OBJ_BOUND_METHOD: {
      ObjBoundMethod* bound = (ObjBoundMethod*)object;
      mark_value(bound->receiver);
      mark_obj((Obj*)bound->method);
      break;
    }
    case OBJ_CLASS: {
      ObjClass* klass = (ObjClass*)object;
      mark_obj((Obj*)klass->name);
      mark_hashtable(&klass->methods);
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      mark_obj((Obj*)closure->function);
      for (int i = 0; i < closure->upvalue_count; i++) {
        mark_obj((Obj*)closure->upvalues[i]);
      }
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      mark_obj((Obj*)function->name);
      mark_array(&function->chunk.constants);
      break;
    }
    case OBJ_INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      mark_obj((Obj*)instance->klass);
      mark_hashtable(&instance->fields);
      break;
    }
    case OBJ_UPVALUE:
      mark_value(((ObjUpvalue*)object)->closed);
      break;
    case OBJ_SEQ: {
      ObjSeq* seq = (ObjSeq*)object;
      mark_array(&seq->items);
      break;
    }
    case OBJ_NATIVE:
    case OBJ_STRING:
      break;
    default:
      break;  // TODO (recovery): What do we do here? Throw? Probably yes, bc we
              // need to mark all objects
  }
}

// Frees an object from our heap.
// How we free an object depends on its type.
static void free_object(Obj* object) {
#ifdef DEBUG_LOG_GC
  printf(ANSI_RED_STR("[GC] ") ANSI_GREEN_STR("[FREE] ") "%p, type %s\n",
         (void*)object, obj_type_to_string(object->type));
#endif

  switch (object->type) {
    case OBJ_BOUND_METHOD:
      FREE(ObjBoundMethod, object);
      break;
    case OBJ_CLASS: {
      ObjClass* klass = (ObjClass*)object;
      free_hashtable(&klass->methods);
      FREE(ObjClass, object);
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalue_count);
      FREE(ObjClosure, object);
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      free_chunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }
    case OBJ_INSTANCE: {
      ObjInstance* instance = (ObjInstance*)object;
      free_hashtable(&instance->fields);
      FREE(ObjInstance, object);
      break;
    }
    case OBJ_NATIVE:
      FREE(ObjNative, object);
      break;
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
    case OBJ_SEQ: {
      ObjSeq* seq = (ObjSeq*)object;
      free_value_array(&seq->items);
      FREE(ObjSeq, object);
      break;
    }
    case OBJ_UPVALUE:
      FREE(ObjUpvalue, object);
      break;
      // TODO (recovery): We probably need a default case here. What do we do
      // there? Throw? Probably yes, bc we need to free all objects
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

// Starts at the roots of the objects in the heap and marks all reachable
// objects.
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
  for (ObjUpvalue* upvalue = vm.open_upvalues; upvalue != NULL;
       upvalue = upvalue->next) {
    mark_obj((Obj*)upvalue);
  }

  // Also mark the globals hashtable.
  mark_hashtable(&vm.globals);

  // And the compiler roots. The GC can run while compiling, so we need to mark
  // the compiler's internal state as well.
  mark_compiler_roots();

  // And the init string.
  mark_obj((Obj*)vm.init_string);
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
  Obj* object = vm.objects;

  while (object != NULL) {
    if (object->is_marked) {
      object->is_marked = false;  // Unmark for next gc cycle
      previous = object;
      object = object->next;
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
  size_t before = vm.bytes_allocated;
#endif

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

  vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
  printf(
      ANSI_RED_STR(
          "[GC] ") "Done. collected %zu bytes (from %zu to %zu) next at %zu\n",
      before - vm.bytes_allocated, before, vm.bytes_allocated, vm.next_gc);
  printf("== Gc end collect ==\n");
#endif
}
