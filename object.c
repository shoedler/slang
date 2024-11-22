#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "chunk.h"
#include "hashtable.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

// Allocates a heap-allocated string in a string object.
// The string object sort of acts like a wrapper for the c string.
// Both the string object and the c string are heap-allocated.
static ObjString* allocate_string(char* chars, int length, uint64_t hash) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_GC_STRING);
  string->length    = length;
  string->chars     = chars;
  string->obj.hash  = hash;

  push(str_value(string));  // Prevent GC from freeing string
  hashtable_set(&vm.strings, str_value(string), nil_value());
  pop();

  return string;
}

ObjBoundMethod* new_bound_method(Value receiver, Obj* method) {
  ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_GC_BOUND_METHOD);
  bound->receiver       = receiver;
  bound->method         = method;
  return bound;
}

ObjClass* new_class(ObjString* name, ObjClass* base) {
  ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_GC_CLASS);
  klass->name     = name;
  klass->base     = base;

  klass->__get_prop = NULL;
  klass->__set_prop = NULL;
  klass->__get_subs = NULL;
  klass->__set_subs = NULL;

  klass->__ctor   = NULL;
  klass->__to_str = NULL;
  klass->__has    = NULL;
  klass->__slice  = NULL;

  init_hashtable(&klass->methods);
  init_hashtable(&klass->static_methods);
  return klass;
}

void finalize_new_class(ObjClass* klass) {
  struct MethodMap {
    Obj** field;
    SpecialMethodNames index;
  };

  struct MethodMap specials[] = {
      {&klass->__ctor, SPECIAL_METHOD_CTOR},
      {&klass->__to_str, SPECIAL_METHOD_TO_STR},
      {&klass->__has, SPECIAL_METHOD_HAS},
      {&klass->__slice, SPECIAL_METHOD_SLICE},
      {NULL, SPECIAL_METHOD_MAX},
  };

  Value temp = nil_value();
  for (struct MethodMap* entry = specials; entry->field != NULL; entry++) {
    // Try to populate the field from the class itself, or any of its base classes
    ObjClass* base = klass;
    while (base != NULL) {
      if (hashtable_get_by_string(&base->methods, vm.special_method_names[entry->index], &temp)) {
        break;
      }
      base = base->base;
    }

    if (base != NULL && is_callable(temp)) {
      *entry->field = temp.as.obj;
    } else {
      *entry->field = NULL;
    }
  }

  // Also copy the accessors from the base class, if the klass doesn't have them
  if (klass->base != NULL) {
    klass->__get_prop = klass->__get_prop != NULL ? klass->__get_prop : klass->base->__get_prop;
    klass->__set_prop = klass->__set_prop != NULL ? klass->__set_prop : klass->base->__set_prop;
    klass->__get_subs = klass->__get_subs != NULL ? klass->__get_subs : klass->base->__get_subs;
    klass->__set_subs = klass->__set_subs != NULL ? klass->__set_subs : klass->base->__set_subs;
  }
}

ObjObject* new_instance(ObjClass* klass) {
  ObjObject* object      = ALLOCATE_OBJ(ObjObject, OBJ_GC_OBJECT);
  object->instance_class = klass;
  init_hashtable(&object->fields);
  return object;
}

ObjUpvalue* new_upvalue(Value* slot) {
  ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_GC_UPVALUE);
  upvalue->closed     = nil_value();
  upvalue->location   = slot;
  upvalue->next       = NULL;
  return upvalue;
}

ObjClosure* new_closure(ObjFunction* function) {
  ObjUpvalue** upvalues = ALLOCATE_ARRAY(ObjUpvalue*, function->upvalue_count);
  for (int i = 0; i < function->upvalue_count; i++) {
    upvalues[i] = NULL;
  }

  ObjClosure* closure    = ALLOCATE_OBJ(ObjClosure, OBJ_GC_CLOSURE);
  closure->function      = function;
  closure->upvalues      = upvalues;
  closure->upvalue_count = function->upvalue_count;
  return closure;
}

ObjFunction* new_function() {
  ObjFunction* function     = ALLOCATE_OBJ(ObjFunction, OBJ_GC_FUNCTION);
  function->arity           = 0;
  function->upvalue_count   = 0;
  function->name            = NULL;
  function->globals_context = NULL;
  init_chunk(&function->chunk);
  return function;
}

ObjSeq* new_seq() {
  ValueArray items;
  init_value_array(&items);
  return take_seq(&items);
}

ObjTuple* new_tuple() {
  ValueArray items;
  init_value_array(&items);
  return take_tuple(&items);
}

ObjNative* new_native(NativeFn function, ObjString* name, int arity) {
  ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_GC_NATIVE);
  native->function  = function;
  native->name      = name;
  native->arity     = arity;
  return native;
}

// Hashes a string using the FNV-1a algorithm.
static uint64_t hash_string(const char* key, int length) {
  uint64_t hash = FNV_1A_64_OFFSET_BASIS;  // FNV-1a 64-bit hash offset basis
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= FNV_1A_64_PRIME;  // FNV-1a 64-bit prime
  }
  return hash;
}

// // Hashes a tuple. Borrowed from Python.
static uint64_t hash_tuple(ValueArray* items) {
  uint64_t length = items->count;
  uint64_t result = TUPLE_HASH_INITIAL;
  for (uint64_t i = 0; i < length; i++) {
    result = (result * TUPLE_HASH_MULTIPLIER) ^ hash_value(items->values[i]);
  }
  result += TUPLE_HASH_OFFSET;
  return result;
}

ObjSeq* take_seq(ValueArray* items) {
  // Pause while we allocate an object for seq, because this might trigger a GC. This allows us to prepare a value array with it's
  // values being out of reach of the GC.
  VM_SET_FLAG(VM_FLAG_PAUSE_GC);
  ObjSeq* seq = ALLOCATE_OBJ(ObjSeq, OBJ_GC_SEQ);
  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);
  seq->items = *items;
  return seq;
}

ObjTuple* take_tuple(ValueArray* items) {
  // Pause while we allocate an object for tuple, because this might trigger a GC. This allows us to prepare a value array with
  // it's values being out of reach of the GC.
  VM_SET_FLAG(VM_FLAG_PAUSE_GC);
  ObjTuple* tuple = ALLOCATE_OBJ(ObjTuple, OBJ_GC_TUPLE);
  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);
  tuple->items    = *items;
  tuple->obj.hash = hash_tuple(items);
  return tuple;
}

ObjObject* take_object(HashTable* fields) {
  ObjObject* object      = ALLOCATE_OBJ(ObjObject, OBJ_GC_OBJECT);
  object->instance_class = vm.obj_class;
  object->fields         = *fields;
  return object;
}

ObjString* take_string(char* chars, int length) {
  uint64_t hash       = hash_string(chars, length);
  ObjString* interned = hashtable_find_string(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocate_string(chars, length, hash);
}

ObjString* copy_string(const char* chars, int length) {
  uint64_t hash       = hash_string(chars, length);
  ObjString* interned = hashtable_find_string(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    return interned;
  }

  char* heap_chars = ALLOCATE_ARRAY(char, length + 1);
  memcpy(heap_chars, chars, length);
  heap_chars[length] = '\0';
  return allocate_string(heap_chars, length, hash);
}
