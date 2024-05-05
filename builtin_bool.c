#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_bool_class() {
  BUILTIN_REGISTER_BASE_CLASS(TYPENAME_BOOL);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_TO_STR, 0);

  BUILTIN_FINALIZE_CLASS(TYPENAME_BOOL);
}

// Built-in bool constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_BOOL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, SP_METHOD_CTOR) {
  UNUSED(argc);
  if (is_falsey(argv[1])) {
    return BOOL_VAL(false);
  }

  return BOOL_VAL(true);
}

// Built-in method to convert a value to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_BOOL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(BOOL)

  if (AS_BOOL(argv[0])) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, STR_LEN(VALUE_STR_TRUE));
    return OBJ_VAL(str_obj);
  } else {
    ObjString* str_obj = copy_string(VALUE_STR_FALSE, STR_LEN(VALUE_STR_FALSE));
    return OBJ_VAL(str_obj);
  }
}
