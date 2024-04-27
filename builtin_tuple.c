#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_tuple_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_TUPLE, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, SP_METHOD_TO_STR, 0);

  BUILTIN_FINALIZE_CLASS(TYPENAME_TUPLE);
}

// Built-in tuple constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_TUPLE,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("len", TYPENAME_INT),
    /* Return Type */ TYPENAME_TUPLE,
    /* Description */
    "No-op constructor for " STR(TYPENAME_TUPLE) ".");
BUILTIN_METHOD_IMPL(TYPENAME_TUPLE, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a tuple via " STR(TYPENAME_TUPLE) "." STR(SP_METHOD_CTOR) ".");
  return NIL_VAL;
}

// Built-in method to convert a tuple to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_TUPLE,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_TUPLE,
    /* Description */ "Returns a " STR(TYPENAME_STRING) " representation of a " STR(TYPENAME_TUPLE) ".");
BUILTIN_METHOD_IMPL(TYPENAME_TUPLE, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(TUPLE)

  ObjTuple* tuple = AS_TUPLE(argv[0]);
  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);

  strcpy(chars, VALUE_STR_TUPLE_START);
  for (int i = 0; i < tuple->items.count; i++) {
    // Execute the to_str method on the item
    push(tuple->items.values[i]);  // Push the receiver (item at i) for to_str
    ObjString* item_str = AS_STRING(exec_callable(typeof(tuple->items.values[i])->__to_str, 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

    // Expand chars to fit the separator plus the next item
    size_t new_buf_size = strlen(chars) + item_str->length + (STR_LEN(VALUE_STR_TUPLE_DELIM)) +
                          (STR_LEN(VALUE_STR_TUPLE_END));  // Consider the closing bracket -  if we're done after this
                                                           // iteration we won't need to expand and can just slap it on there

    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size        = new_buf_size;
      size_t old_size = strlen(chars);
      chars           = realloc(chars, buf_size);
      chars[old_size] = '\0';  // Ensure null-termination at the end of the old string
    }

    // Append the string
    strcat(chars, item_str->chars);
    if (i < tuple->items.count - 1) {
      strcat(chars, VALUE_STR_TUPLE_DELIM);
    }
  }

  strcat(chars, VALUE_STR_TUPLE_END);

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
