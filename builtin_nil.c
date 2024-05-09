#include "builtin.h"
#include "common.h"
#include "vm.h"

void finalize_builtin_nil_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_TO_STR, 0);

  BUILTIN_FINALIZE_CLASS(TYPENAME_NIL);
}

/**
 * TYPENAME_NIL.SP_METHOD_CTOR() -> TYPENAME_NIL
 * @brief No-op constructor for TYPENAME_NIL.
 */
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate " VALUE_STR_NIL " via " STR(TYPENAME_NIL) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_NIL.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of TYPENAME_NIL.
 */
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(NIL)

  ObjString* str_obj = copy_string(VALUE_STR_NIL, STR_LEN(VALUE_STR_NIL));
  return str_value(str_obj);
}