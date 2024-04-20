#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_nil_class() {
  BUILTIN_REGISTER_BASE_CLASS(TYPENAME_NIL);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_TO_STR, 0);

  BUILTIN_FINALIZE_CLASS(TYPENAME_NIL);
}

// Built-in nil constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NIL,
    /* Description */
    "<Not supported>");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate " VALUE_STR_NIL " via " STR(TYPENAME_NIL) "." STR(SP_METHOD_CTOR) ".");
  return NIL_VAL;
}

// Built-in method to convert a nil to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_NIL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(NIL)

  ObjString* str_obj = copy_string(VALUE_STR_NIL, STR_LEN(VALUE_STR_NIL));
  return OBJ_VAL(str_obj);
}