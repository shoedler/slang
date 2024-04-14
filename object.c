#include <memory.h>

#include "hashtable.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include "debug.h"
#endif

// Allocates a new object of the given type.
#define ALLOCATE_OBJ(type, object_type) (type*)allocate_object(sizeof(type), object_type)

// Allocates a new object of the given type and size.
// It also initializes the object's fields. Might trigger GC, but (obviously)
// won't free the new object to be allocated.
static Obj* allocate_object(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0,
                                 size);  // Might trigger GC, but it's fine since our new object
                                         // isn't referenced by anything yet -> not reachable by GC
  object->type      = type;
  object->is_marked = false;
  object->hash      = (uint32_t)((intptr_t)(object) >> 4 | (intptr_t)(object) << 28);  // Get a better distribution of hash
                                                                                       // values, by shifting the address
  object->next = vm.objects;
  vm.objects   = object;

#ifdef DEBUG_LOG_GC_ALLOCATIONS
  printf(ANSI_RED_STR("[GC] ") ANSI_MAGENTA_STR("[ALLOC] ") "%p allocate %zu for type %d\n", (void*)object, size, type);
#endif

  return object;
}

// Allocates a heap-allocated string in a string object.
// The string object sort of acts like a wrapper for the c string.
// Both the string object and the c string are heap-allocated.
static ObjString* allocate_string(char* chars, int length, uint32_t hash) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length    = length;
  string->chars     = chars;
  string->obj.hash  = hash;

  push(OBJ_VAL(string));  // Prevent GC from freeing string
  hashtable_set(&vm.strings, OBJ_VAL(string), NIL_VAL);
  pop();

  return string;
}

ObjBoundMethod* new_bound_method(Value receiver, Obj* method) {
  ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
  bound->receiver       = receiver;
  bound->method         = method;
  return bound;
}

ObjClass* new_class(ObjString* name, ObjClass* base) {
  ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  klass->name     = name;
  klass->base     = base;
  init_hashtable(&klass->methods);
  init_hashtable(&klass->static_methods);
  return klass;
}

void finalize_new_class(ObjClass* klass) {
  struct TypeMap {
    Obj** field;
    SpecialFieldNames index;
  };

  struct TypeMap specials[] = {
      {&klass->__ctor, SPECIAL_METHOD_CTOR},
      {&klass->__get, SPECIAL_METHOD_GET},
      {&klass->__set, SPECIAL_METHOD_SET},
      {&klass->__len, SPECIAL_METHOD_LEN},
      {&klass->__to_str, SPECIAL_METHOD_TO_STR},
      {&klass->__has, SPECIAL_METHOD_HAS},
      {&klass->__get_slice, SPECIAL_METHOD_GETSLICE},
      {&klass->__set_slice, SPECIAL_METHOD_SETSLICE},
      {&klass->__name, SPECIAL_PROP_NAME},
      {&klass->__doc, SPECIAL_PROP_DOC},
      {&klass->__file_path, SPECIAL_PROP_FILE_PATH},
      {&klass->__module_name, SPECIAL_PROP_MODULE_NAME},
      {NULL, SPECIAL_FIELD_MAX},
  };

  Value temp;
  for (struct TypeMap* entry = specials; entry->field != NULL; entry++) {
    // Try to populate the field from the class itself, or any of its base classes
    ObjClass* base = klass;
    while (base != NULL) {
      if (hashtable_get_by_string(&base->methods, vm.special_field_names[entry->index], &temp)) {
        break;
      }
      base = base->base;
    }

    if (base != NULL && IS_CALLABLE(temp)) {
      *entry->field = AS_OBJ(temp);
    } else {
      *entry->field = NULL;
    }
  }
}

ObjObject* new_instance(ObjClass* klass) {
  ObjObject* object = ALLOCATE_OBJ(ObjObject, OBJ_OBJECT);
  object->klass     = klass;
  init_hashtable(&object->fields);
  return object;
}

ObjUpvalue* new_upvalue(Value* slot) {
  ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
  upvalue->closed     = NIL_VAL;
  upvalue->location   = slot;
  upvalue->next       = NULL;
  return upvalue;
}

ObjClosure* new_closure(ObjFunction* function) {
  ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalue_count);
  for (int i = 0; i < function->upvalue_count; i++) {
    upvalues[i] = NULL;
  }

  ObjClosure* closure    = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function      = function;
  closure->upvalues      = upvalues;
  closure->upvalue_count = function->upvalue_count;
  return closure;
}

ObjFunction* new_function() {
  ObjFunction* function     = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity           = 0;
  function->upvalue_count   = 0;
  function->name            = NULL;
  function->doc             = NULL;
  function->globals_context = NULL;
  init_chunk(&function->chunk);
  return function;
}

ObjSeq* new_seq() {
  ValueArray items;
  init_value_array(&items);
  return take_seq(&items);
}

ObjNative* new_native(NativeFn function, ObjString* name, ObjString* doc, int arity) {
  ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function  = function;
  native->name      = name;
  native->doc       = doc;
  native->arity     = arity;
  return native;
}

// Hashes a string using the FNV-1a algorithm.
static uint32_t hash_string(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjSeq* prealloc_seq(int count) {
  ValueArray items;
  init_value_array(&items);

  int capacity   = GROW_CAPACITY(count);
  items.values   = RESIZE_ARRAY(Value, items.values, 0, capacity);
  items.capacity = capacity;
  items.count    = count;

  for (int i = 0; i < count; i++) {
    items.values[i] = NIL_VAL;
  }

  return take_seq(&items);
}

ObjSeq* take_seq(ValueArray* items) {
  ObjSeq* seq = ALLOCATE_OBJ(ObjSeq, OBJ_SEQ);
  seq->items  = *items;
  return seq;
}

ObjObject* take_object(HashTable* fields) {
  ObjObject* object = ALLOCATE_OBJ(ObjObject, OBJ_OBJECT);
  object->klass     = vm.__builtin_Obj_class;
  object->fields    = *fields;
  return object;
}

ObjString* take_string(char* chars, int length) {
  uint32_t hash       = hash_string(chars, length);
  ObjString* interned = hashtable_find_string(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocate_string(chars, length, hash);
}

ObjString* copy_string(const char* chars, int length) {
  uint32_t hash       = hash_string(chars, length);
  ObjString* interned = hashtable_find_string(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    return interned;
  }

  char* heap_chars = ALLOCATE(char, length + 1);
  memcpy(heap_chars, chars, length);
  heap_chars[length] = '\0';
  return allocate_string(heap_chars, length, hash);
}
