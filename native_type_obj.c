#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "hashtable.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool obj_get_prop(Value receiver, ObjString* name, Value* result);
static bool obj_set_prop(Value receiver, ObjString* name, Value value);
static bool obj_get_subs(Value receiver, Value index, Value* result);
static bool obj_set_subs(Value receiver, Value index, Value value);

static Value obj_ctor(int argc, Value argv[]);
static Value obj_to_str(int argc, Value argv[]);
static Value obj_has(int argc, Value argv[]);
static Value obj_entries(int argc, Value argv[]);
static Value obj_values(int argc, Value argv[]);
static Value obj_keys(int argc, Value argv[]);

ObjClass* partial_init_native_obj_class() {
  ObjClass* obj_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  obj_class->__get_prop = obj_get_prop;
  obj_class->__set_prop = obj_set_prop;
  obj_class->__get_subs = obj_get_subs;
  obj_class->__set_subs = obj_set_subs;
  obj_class->__equals   = native_default_obj_equals;
  obj_class->__hash     = native_default_obj_hash;

  return obj_class;
}

void finalize_native_obj_class() {
  define_native(&vm.obj_class->methods, STR(SP_METHOD_CTOR), obj_ctor, 0);
  define_native(&vm.obj_class->methods, STR(SP_METHOD_TO_STR), obj_to_str, 0);
  define_native(&vm.obj_class->methods, STR(SP_METHOD_HAS), obj_has, 1);
  define_native(&vm.obj_class->methods, "entries", obj_entries, 0);
  define_native(&vm.obj_class->methods, "values", obj_values, 0);
  define_native(&vm.obj_class->methods, "keys", obj_keys, 0);
  finalize_new_class(vm.obj_class);
}

static bool obj_get_prop(Value receiver, ObjString* name, Value* result) {
  ObjObject* object = AS_OBJECT(receiver);
  if (hashtable_get_by_string(&object->fields, name, result)) {
    return true;
  }
  if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {
    *result = int_value(object->fields.count);
    return true;
  }
  NATIVE_DEFAULT_GET_PROP_BODY(
      receiver.type)  // We must use the receiver's type instead of vm.obj_class here: Instances are ObjObjects.
}

static bool obj_set_prop(Value receiver, ObjString* name, Value value) {
  ObjObject* object = AS_OBJECT(receiver);
  hashtable_set(&object->fields, str_value(name), value);
  return true;
}

static bool obj_get_subs(Value receiver, Value index, Value* result) {
  ObjObject* object = AS_OBJECT(receiver);
  if (hashtable_get(&object->fields, index, result)) {
    return true;
  }
  *result = nil_value();
  return true;
}

static bool obj_set_subs(Value receiver, Value index, Value value) {
  ObjObject* object = AS_OBJECT(receiver);
  hashtable_set(&object->fields, index, value);
  return true;
}

/**
 * TYPENAME_OBJ.SP_METHOD_CTOR() -> TYPENAME_OBJ
 * @brief returns a new empty TYPENAME_OBJ.
 */
static Value obj_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  // This is a base implementation, all objects which don't have a custom implementation will use this one.
  // Works nicely for all classes, because instances are ObjObjects. Any other value type, like primitives, are handled internally
  // and have their own constructors.

  return argv[0];
}

static Value instance_object_to_str(int argc, Value* argv) {
  UNUSED(argc);

  ObjString* name = (argv[0]).type->name;
  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(str_value(name));

  size_t buf_size = VALUE_STRFTM_INSTANCE_LEN + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFTM_INSTANCE, name->chars);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return str_value(str_obj);
}

static Value anonymous_object_to_str(int argc, Value* argv) {
  UNUSED(argc);

  ObjObject* object = AS_OBJECT(argv[0]);
  size_t buf_size   = 64;  // Start with a reasonable size
  char* chars       = malloc(buf_size);
  int processed     = 0;  // Keep track of how many non-EMPTY fields we've processed to know when to skip the
                          // last delimiter

  strcpy(chars, VALUE_STR_OBJECT_START);
  for (int i = 0; i < object->fields.capacity; i++) {
    if (is_empty_internal(object->fields.entries[i].key)) {
      continue;
    }

    // Execute the to_str method on the key
    push(object->fields.entries[i].key);  // Push the receiver (key at i) for to_str
    ObjString* key_str = AS_STR(exec_callable(fn_value(object->fields.entries[i].key.type->__to_str), 0));
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
      return nil_value();
    }

    push(str_value(key_str));  // GC Protection

    // Execute the to_str method on the value
    push(object->fields.entries[i].value);  // Push the receiver (value at i) for to_str
    ObjString* value_str = AS_STR(exec_callable(fn_value(object->fields.entries[i].value.type->__to_str), 0));
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
      return nil_value();
    }

    pop();  // Key str

    // Expand chars to fit the separator, delimiter plus the next key and value
    size_t new_buf_size = strlen(chars) + strlen(key_str->chars) + strlen(value_str->chars) +
                          (STR_LEN(VALUE_STR_OBJECT_SEPARATOR) - 1) + (sizeof(VALUE_STR_OBJECT_DELIM)) +
                          (STR_LEN(VALUE_STR_OBJECT_END));  // Consider the closing bracket - if we're done after this iteration
                                                            // we won't need to expand and can just slap it on there

    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size        = new_buf_size;
      size_t old_size = strlen(chars);
      chars           = realloc(chars, buf_size);
      chars[old_size] = '\0';  // Ensure null-termination at the end of the old string
    }

    // Append the strings
    strcat(chars, key_str->chars);
    strcat(chars, VALUE_STR_OBJECT_SEPARATOR);
    strcat(chars, value_str->chars);
    if (processed < object->fields.count - 1) {
      strcat(chars, VALUE_STR_OBJECT_DELIM);
    }
    processed++;
  }

  strcat(chars, VALUE_STR_OBJECT_END);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj =
      copy_string(chars,
                  (int)strlen(chars));  // TODO (optimize): Use buf_size here, but
                                        // we need to make sure that the string is
                                        // null-terminated. Also, if it's < 64 chars long, we need to shorten the length.
  free(chars);
  return str_value(str_obj);
}

/**
 * TYPENAME_OBJ.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of the TYPENAME_OBJ.
 */
static Value obj_to_str(int argc, Value argv[]) {
  // This is a base implementation, all objects which don't have a custom implementation will use this one.
  // Anything that came here is at least a ObjObject, because every other value-type has an internal to_str implementation.

  // Handle anonymous objects and instance objects differently
  if (is_obj(argv[0])) {
    return anonymous_object_to_str(argc, argv);
  }

  return instance_object_to_str(argc, argv);

  // TODO (refactor): Currently, only ObjObject objs are anonymous. The rest is handled as an instance.

  // // This here is the catch-all for all values. We print the type-name and memory address of the value.
  // ObjString* t_name = argv[0].type->name;

  // // Print the memory address of the object using (void*)AS_OBJ(argv[0]).
  // // We need to know the size of the buffer to allocate, so we calculate it first.
  // size_t adr_str_len = snprintf(NULL, 0, "%p", (void*)argv[0].as.obj);

  // size_t buf_size = VALUE_STRFMT_OBJ_LEN + t_name->length + adr_str_len;
  // char* chars     = malloc(buf_size);
  // snprintf(chars, buf_size, VALUE_STRFMT_OBJ, t_name->chars, (void*)argv[0].as.obj);
  // // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // // allocates the memory - we don't want this block in our own memory pool.
  // ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  // free(chars);
  // return str_value(str_obj);
}

/**
 * TYPENAME_OBJ.SP_METHOD_HAS(value: TYPENAME_VALUE) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the TYPENAME_OBJ contains a value which equals 'value'.
 *
 * TYPENAME_OBJ.SP_METHOD_HAS(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the TYPENAME_OBJ contains a value for which 'pred' evaluates to VALUE_STR_TRUE.
 */
static Value obj_has(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.obj_class)

  if (is_fn(argv[1])) {
    Value seq = obj_keys(0, argv);
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {  // Currently, obj_keys doesn't error, but it might in the future.
      return nil_value();
    }

    ValueArray items = AS_SEQ(seq)->items;
    int count        = items.count;  // We need to store this. Might change during the loop

    // Function predicate
    for (int i = 0; i < count; i++) {
      // Execute the provided function on the item
      push(argv[1]);          // Push the function
      push(items.values[i]);  // Push the item
      Value result = exec_callable(argv[1], 1);
      if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
        return nil_value();  // Propagate the error
      }
      // We don't use is_falsey here, because we want a boolean value.
      if (is_bool(result) && result.as.boolean) {
        return bool_value(true);
      }
    }

    return bool_value(false);
  }

  // Value equality, which is easy given that an obj is a hash table.
  Value discard;
  bool has = hashtable_get(&AS_OBJECT(argv[0])->fields, argv[1], &discard);
  return bool_value(has);
}

/**
 * TYPENAME_OBJ.entries() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ of key-value pairs (which are TYPENAME_SEQs of length 2) of an TYPENAME_OBJ, containing all
 * entries.
 */
static Value obj_entries(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER_INHERITS(vm.obj_class)

  ObjObject* object = AS_OBJECT(argv[0]);
  ValueArray items  = init_value_array_of_size(object->fields.count);
  ObjSeq* seq       = take_seq(&items);  // We can already take the seq, because seqs don't calculate the hash upon taking.
  push(seq_value(seq));                  // GC Protection

  // Since we took the seq, we shouldn't manipule the items array directly, because only its [values] field lives on the heap.
  // Modifying items.count for example, would not be reflected in the seq.
  for (int i = 0; i < object->fields.capacity; i++) {
    Entry* entry = &object->fields.entries[i];
    if (!is_empty_internal(entry->key)) {
      push(entry->key);
      push(entry->value);
      make_seq(2);                                    // Leaves a seq with the key-value on the stack
      seq->items.values[seq->items.count++] = pop();  // The seq
    }
  }

  return pop();  // The seq
}

/**
 * TYPENAME_OBJ.keys() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ of all keys of an TYPENAME_OBJ.
 */
static Value obj_keys(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER_INHERITS(vm.obj_class)

  ObjObject* object = AS_OBJECT(argv[0]);
  ValueArray items  = init_value_array_of_size(object->fields.count);

  // We can direclty manipulate items.count, since we haven't taken the seq yet.
  for (int i = 0; i < object->fields.capacity; i++) {
    Entry* entry = &object->fields.entries[i];
    if (!is_empty_internal(entry->key)) {
      items.values[items.count++] = entry->key;
    }
  }

  ObjSeq* seq = take_seq(&items);
  return seq_value(seq);
}

/**
 * TYPENAME_OBJ.values() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ of all values of an TYPENAME_OBJ.
 */
static Value obj_values(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER_INHERITS(vm.obj_class)

  ObjObject* object = AS_OBJECT(argv[0]);
  ValueArray items  = init_value_array_of_size(object->fields.count);

  // We can direclty manipulate items.count, since we haven't taken the seq yet.
  for (int i = 0; i < object->fields.capacity; i++) {
    Entry* entry = &object->fields.entries[i];
    if (!is_empty_internal(entry->key)) {
      items.values[items.count++] = entry->value;
    }
  }

  ObjSeq* seq = take_seq(&items);
  return seq_value(seq);
}
