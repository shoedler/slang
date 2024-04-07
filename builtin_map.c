#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_map_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_MAP, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, __ctor, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, to_str, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, len, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, entries, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, values, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, keys, 0);
}

// Built-in map constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ __ctor,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_MAP,
    /* Description */ "Returns a new empty " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, __ctor) {
  UNUSED(argv);
  UNUSED(argc);

  HashTable entries;
  init_hashtable(&entries);
  ObjMap* map = take_map(&entries);
  return OBJ_VAL(map);
}

// Built-in method to convert a map to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_MAP,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, to_str) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map     = AS_MAP(argv[0]);
  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);
  int processed   = 0;  // Keep track of how many non-EMPTY entries we've processed to know when to skip the
                        // last delimiter

  strcpy(chars, VALUE_STR_MAP_START);
  for (int i = 0; i < map->entries.capacity; i++) {
    if (IS_EMPTY_INTERNAL(map->entries.entries[i].key)) {
      continue;
    }

    // Execute the to_str method on the key
    push(map->entries.entries[i].key);  // Push the receiver (key at i) for to_str
    ObjString* key_str = AS_STRING(exec_method(copy_string("to_str", 6), 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

    push(OBJ_VAL(key_str));  // GC Protection

    // Execute the to_str method on the value
    push(map->entries.entries[i].value);  // Push the receiver (value at i) for to_str
    ObjString* value_str = AS_STRING(exec_method(copy_string("to_str", 6), 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

    pop();  // Key str

    // Expand chars to fit the separator, delimiter plus the next key and value
    size_t new_buf_size = strlen(chars) + strlen(key_str->chars) + strlen(value_str->chars) +
                          (sizeof(VALUE_STR_MAP_SEPARATOR) - 1) + (sizeof(VALUE_STR_MAP_DELIM) - 1) +
                          (sizeof(VALUE_STR_MAP_END) - 1);  // Consider the closing bracket -
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
    strcat(chars, VALUE_STR_MAP_SEPARATOR);
    strcat(chars, value_str->chars);
    if (processed < map->entries.count - 1) {
      strcat(chars, VALUE_STR_MAP_DELIM);
    }
    processed++;
  }

  strcat(chars, VALUE_STR_MAP_END);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(
      chars,
      (int)strlen(chars));  // TODO (optimize): Use buf_size here, but
                            // we need to make sure that the string is
                            // null-terminated. Also, if it's < 64 chars long, we need to shorten the length.
  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in method to retrieve the length of a map
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ len,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, len) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(MAP)

  int length = AS_MAP(argv[0])->entries.count;
  return NUMBER_VAL(length);
}

// Built-in method to retrieve all entries of a map
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ entries,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Returns a " STR(TYPENAME_SEQ) " of key-value pairs (which are " STR(
        TYPENAME_SEQ) "s of length 2 ) of a " STR(TYPENAME_MAP) ", containing all entries.");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, entries) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map = AS_MAP(argv[0]);
  ObjSeq* seq = prealloc_seq(map->entries.count);
  push(OBJ_VAL(seq));  // GC Protection

  int processed = 0;
  for (int i = 0; i < map->entries.capacity; i++) {
    Entry* entry = &map->entries.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      push(entry->key);
      push(entry->value);
      make_seq(2);                             // Leaves a seq with the key-value on the stack
      seq->items.values[processed++] = pop();  // The seq
    }
  }

  return pop();  // The seq
}

// Built-in method to retrieve all keys of a map
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ keys,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_SEQ) " of all keys of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, keys) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map = AS_MAP(argv[0]);
  ObjSeq* seq = prealloc_seq(map->entries.count);
  push(OBJ_VAL(seq));  // GC Protection

  int processed = 0;
  for (int i = 0; i < map->entries.capacity; i++) {
    Entry* entry = &map->entries.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      seq->items.values[processed++] = entry->key;
    }
  }

  return pop();  // The seq
}

// Built-in method to retrieve all values of a map
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ values,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_SEQ) " of all values of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, values) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map = AS_MAP(argv[0]);
  ObjSeq* seq = prealloc_seq(map->entries.count);
  push(OBJ_VAL(seq));  // GC Protection

  int processed = 0;
  for (int i = 0; i < map->entries.capacity; i++) {
    Entry* entry = &map->entries.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      seq->items.values[processed++] = entry->value;
    }
  }

  return pop();  // The seq
}
