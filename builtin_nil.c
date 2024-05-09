#include "builtin.h"
#include "common.h"
#include "vm.h"

static Value nil_ctor(int argc, Value argv[]);
static Value nil_to_str(int argc, Value argv[]);

void finalize_native_nil_class() {
  define_native(&vm.nil_class->methods, STR(SP_METHOD_CTOR), nil_ctor, 1);
  define_native(&vm.nil_class->methods, STR(SP_METHOD_TO_STR), nil_to_str, 0);
  finalize_new_class(vm.nil_class);
}

/**
 * TYPENAME_NIL.SP_METHOD_CTOR() -> TYPENAME_NIL
 * @brief No-op constructor for TYPENAME_NIL.
 */
static Value nil_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate " VALUE_STR_NIL " via " STR(TYPENAME_NIL) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_NIL.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of TYPENAME_NIL.
 */
static Value nil_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(NIL)

  ObjString* str_obj = copy_string(VALUE_STR_NIL, STR_LEN(VALUE_STR_NIL));
  return str_value(str_obj);
}