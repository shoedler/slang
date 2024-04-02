#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_nil_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_NIL, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, __ctor, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, to_str, 0);
}

// Built-in nil constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ __ctor,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NIL,
    /* Description */
    "Returns " STR(TYPENAME_NIL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, __ctor) {
  UNUSED(argc);
  UNUSED(argv);
  return NIL_VAL;
}

// Built-in method to convert a nil to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_NIL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, to_str) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(NIL)

  ObjString* str_obj = copy_string(VALUE_STR_NIL, sizeof(VALUE_STR_NIL) - 1);
  return OBJ_VAL(str_obj);
}