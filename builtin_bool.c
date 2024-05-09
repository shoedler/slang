#include "builtin.h"
#include "common.h"
#include "vm.h"

static Value bool_ctor(int argc, Value argv[]);
static Value bool_to_str(int argc, Value argv[]);

void finalize_native_bool_class() {
  define_native(&vm.bool_class->methods, STR(SP_METHOD_CTOR), bool_ctor, 1);
  define_native(&vm.bool_class->methods, STR(SP_METHOD_TO_STR), bool_to_str, 0);
  finalize_new_class(vm.bool_class);
}

/**
 * TYPENAME_BOOL.SP_METHOD_CTOR(value: TYPENAME_VALUE) -> TYPENAME_BOOL
 * @brief Converts the first argument to a TYPENAME_BOOL.
 */
static Value bool_ctor(int argc, Value argv[]) {
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
static Value bool_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(BOOL)

  if (argv[0].as.boolean) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, STR_LEN(VALUE_STR_TRUE));
    return str_value(str_obj);
  } else {
    ObjString* str_obj = copy_string(VALUE_STR_FALSE, STR_LEN(VALUE_STR_FALSE));
    return str_value(str_obj);
  }
}
