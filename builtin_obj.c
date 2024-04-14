#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_obj_class() {
  // Create the object class
  vm.__builtin_Obj_class = new_class(copy_string(STR(TYPENAME_OBJ), STR_LEN(STR(TYPENAME_OBJ))), NULL);

  // Create the builtin obj instance
  vm.builtin = new_instance(vm.__builtin_Obj_class);

  define_obj(&vm.builtin->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);
  define_obj(&vm.builtin->fields, STR(TYPENAME_OBJ), (Obj*)vm.__builtin_Obj_class);

  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, SP_METHOD_LEN, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, hash, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, entries, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, values, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, keys, 0);
  BUILTIN_FINALIZE_CLASS(TYPENAME_OBJ);
}

// Built-in obj constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_OBJ,
    /* Description */ "Returns a new empty " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, SP_METHOD_CTOR) {
  UNUSED(argv);
  BUILTIN_ARGC_EXACTLY(0)

  HashTable fields;
  init_hashtable(&fields);
  ObjObject* object = take_object(&fields);
  return OBJ_VAL(object);
}

// Built-in method to return the hash of an object.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ hash,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the hash of the " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, hash) {
  BUILTIN_ARGC_EXACTLY(0)
  return NUMBER_VAL(hash_value(argv[0]));
}

static Value instance_object_to_str(int argc, Value* argv) {
  UNUSED(argc);

  ObjObject* object = AS_OBJECT(argv[0]);
  ObjString* name   = object->klass->name;
  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(OBJ_VAL(name));

  size_t buf_size = VALUE_STRFTM_INSTANCE_LEN + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFTM_INSTANCE, name->chars);
  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return OBJ_VAL(str_obj);
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
    if (IS_EMPTY_INTERNAL(object->fields.entries[i].key)) {
      continue;
    }

    // Execute the to_str method on the key
    push(object->fields.entries[i].key);  // Push the receiver (key at i) for to_str
    ObjString* key_str = AS_STRING(exec_fn(typeof(object->fields.entries[i].key)->__to_str, 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

    push(OBJ_VAL(key_str));  // GC Protection

    // Execute the to_str method on the value
    push(object->fields.entries[i].value);  // Push the receiver (value at i) for to_str
    ObjString* value_str = AS_STRING(exec_fn(typeof(object->fields.entries[i].value)->__to_str, 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

    pop();  // Key str

    // Expand chars to fit the separator, delimiter plus the next key and value
    size_t new_buf_size = strlen(chars) + strlen(key_str->chars) + strlen(value_str->chars) +
                          (STR_LEN(VALUE_STR_OBJECT_SEPARATOR) - 1) + (sizeof(VALUE_STR_OBJECT_DELIM)) +
                          (STR_LEN(VALUE_STR_OBJECT_END));  // Consider the closing bracket -
                                                            // if we're done after this
                                                            // iteration we won't need to
                                                            // expand and can just slap it on there

    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size = new_buf_size;
      chars    = realloc(chars, buf_size);
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
  return OBJ_VAL(str_obj);
}

// Built-in method to convert an object to a string. This one is special in that is is the toplevel to_str -
// there's no abstraction over it. This is why we have some special cases here for our internal types.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of the " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  // BUILTIN_CHECK_RECEIVER(OBJ) // This should accept any value

  // Handle anonymous objects and instance objects differently
  if (IS_OBJECT(argv[0])) {
    ObjObject* object = AS_OBJECT(argv[0]);
    if (OBJECT_IS_INSTANCE(object)) {
      return instance_object_to_str(argc, argv);
    } else {
      return anonymous_object_to_str(argc, argv);
    }
  }

  // This here is the catch-all for all values. We print the type-name and memory address of the value.
  ObjString* t_name = typeof(argv[0])->name;

  // Print the memory address of the object using (void*)AS_OBJ(argv[0]).
  // We need to know the size of the buffer to allocate, so we calculate it first.
  size_t adr_str_len = snprintf(NULL, 0, "%p", (void*)AS_OBJ(argv[0]));

  size_t buf_size = VALUE_STRFMT_OBJ_LEN + t_name->length + adr_str_len;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFMT_OBJ, t_name->chars, (void*)AS_OBJ(argv[0]));
  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in method to retrieve the length of an object.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ SP_METHOD_LEN,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, SP_METHOD_LEN) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(OBJ)

  int length = AS_OBJECT(argv[0])->fields.count;
  return NUMBER_VAL(length);
}

// Built-in method to retrieve all entries of an object.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ entries,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Returns a " STR(TYPENAME_SEQ) " of key-value pairs (which are " STR(TYPENAME_SEQ) "s of length 2 ) of a " STR(
        TYPENAME_OBJ) ", containing all entries.");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, entries) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(OBJ)

  ObjObject* object = AS_OBJECT(argv[0]);
  ObjSeq* seq       = prealloc_seq(object->fields.count);
  push(OBJ_VAL(seq));  // GC Protection

  int processed = 0;
  for (int i = 0; i < object->fields.capacity; i++) {
    Entry* entry = &object->fields.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      push(entry->key);
      push(entry->value);
      make_seq(2);                             // Leaves a seq with the key-value on the stack
      seq->items.values[processed++] = pop();  // The seq
    }
  }

  return pop();  // The seq
}

// Built-in method to retrieve all keys of an object.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ keys,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_SEQ) " of all keys of a " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, keys) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(OBJ)

  ObjObject* object = AS_OBJECT(argv[0]);
  ObjSeq* seq       = prealloc_seq(object->fields.count);
  push(OBJ_VAL(seq));  // GC Protection

  int processed = 0;
  for (int i = 0; i < object->fields.capacity; i++) {
    Entry* entry = &object->fields.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      seq->items.values[processed++] = entry->key;
    }
  }

  return pop();  // The seq
}

// Built-in method to retrieve all values of an object.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ values,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_SEQ) " of all values of a " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, values) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(OBJ)

  ObjObject* object = AS_OBJECT(argv[0]);
  ObjSeq* seq       = prealloc_seq(object->fields.count);
  push(OBJ_VAL(seq));  // GC Protection

  int processed = 0;
  for (int i = 0; i < object->fields.capacity; i++) {
    Entry* entry = &object->fields.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      seq->items.values[processed++] = entry->value;
    }
  }

  return pop();  // The seq
}
