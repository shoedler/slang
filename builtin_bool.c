#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_bool_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_BOOL, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, __ctor, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, to_str, 0);
}

// Built-in bool constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_BOOL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, __ctor) {
  BUILTIN_ARGC_EXACTLY(1)

  if (is_falsey(argv[1])) {
    return BOOL_VAL(false);
  }

  return BOOL_VAL(true);
}

// Built-in method to convert a value to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_BOOL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, to_str) {
  BUILTIN_ARGC_EXACTLY(0)

  BUILTIN_CHECK_RECEIVER(BOOL)

  if (AS_BOOL(argv[0])) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, sizeof(VALUE_STR_TRUE) - 1);
    return OBJ_VAL(str_obj);
  } else {
    ObjString* str_obj = copy_string(VALUE_STR_FALSE, sizeof(VALUE_STR_FALSE) - 1);
    return OBJ_VAL(str_obj);
  }
}