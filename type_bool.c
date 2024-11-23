#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool bool_get_prop(Value receiver, ObjString* name, Value* result);
NATIVE_SET_PROP_NOT_SUPPORTED()
NATIVE_GET_SUBS_NOT_SUPPORTED()
NATIVE_SET_SUBS_NOT_SUPPORTED()

static Value bool_ctor(int argc, Value argv[]);
static Value bool_to_str(int argc, Value argv[]);

void finalize_native_bool_class() {
  vm.bool_class->__get_prop = bool_get_prop;
  vm.bool_class->__set_prop = set_prop_not_supported;  // Not supported
  vm.bool_class->__get_subs = get_subs_not_supported;  // Not supported
  vm.bool_class->__set_subs = set_subs_not_supported;  // Not supported

  define_native(&vm.bool_class->methods, STR(SP_METHOD_CTOR), bool_ctor, 1);
  define_native(&vm.bool_class->methods, STR(SP_METHOD_TO_STR), bool_to_str, 0);
  finalize_new_class(vm.bool_class);
}

static bool bool_get_prop(Value receiver, ObjString* name, Value* result) {
  UNUSED(receiver);
  NATIVE_DEFAULT_GET_PROP_BODY(vm.bool_class)
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
  NATIVE_CHECK_RECEIVER(vm.bool_class)

  if (argv[0].as.boolean) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, STR_LEN(VALUE_STR_TRUE));
    return str_value(str_obj);
  }

  ObjString* str_obj = copy_string(VALUE_STR_FALSE, STR_LEN(VALUE_STR_FALSE));
  return str_value(str_obj);
}
