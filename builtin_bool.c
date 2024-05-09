#include "builtin.h"
#include "common.h"
#include "vm.h"

void finalize_builtin_bool_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_TO_STR, 0);

  BUILTIN_FINALIZE_CLASS(TYPENAME_BOOL);
}

/**
 * TYPENAME_BOOL.SP_METHOD_CTOR(value: TYPENAME_VALUE) -> TYPENAME_BOOL
 * @brief Converts the first argument to a TYPENAME_BOOL.
 */
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, SP_METHOD_CTOR) {
  UNUSED(argc);
  if (is_falsey(argv[1])) {
    return bool_value(false);
  }

  return bool_value(true);
}

/**
 * TYPENAME_BOOL.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of a TYPENAME_BOOL.
 */
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(BOOL)

  if (argv[0].as.boolean) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, STR_LEN(VALUE_STR_TRUE));
    return str_value(str_obj);
  } else {
    ObjString* str_obj = copy_string(VALUE_STR_FALSE, STR_LEN(VALUE_STR_FALSE));
    return str_value(str_obj);
  }
}
