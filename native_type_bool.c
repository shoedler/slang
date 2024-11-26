#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool bool_get_prop(Value receiver, ObjString* name, Value* result);
static bool bool_eq(Value self, Value other);
static uint64_t bool_hash(Value self);

static Value bool_ctor(int argc, Value argv[]);
static Value bool_to_str(int argc, Value argv[]);

ObjClass* partial_init_native_bool_class() {
  ObjClass* bool_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  bool_class->__get_prop = bool_get_prop;
  bool_class->__set_prop = native_set_prop_not_supported;  // Not supported
  bool_class->__get_subs = native_get_subs_not_supported;  // Not supported
  bool_class->__set_subs = native_set_subs_not_supported;  // Not supported
  bool_class->__equals   = bool_eq;
  bool_class->__hash     = bool_hash;

  return bool_class;
}

void finalize_native_bool_class() {
  define_native(&vm.bool_class->methods, STR(SP_METHOD_CTOR), bool_ctor, 1);
  define_native(&vm.bool_class->methods, STR(SP_METHOD_TO_STR), bool_to_str, 0);
  finalize_new_class(vm.bool_class);
}

static bool bool_eq(Value self, Value other) {
  return self.type == other.type && self.as.boolean == other.as.boolean;
}

static uint64_t bool_hash(Value self) {
  return self.as.boolean ? 977 : 479;  // Prime numbers. Selected based on trial and error.
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
