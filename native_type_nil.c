#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool nil_get_prop(Value receiver, ObjString* name, Value* result);
static bool nil_eq(Value self, Value other);
static uint64_t nil_hash(Value self);

static Value nil_ctor(int argc, Value argv[]);
static Value nil_to_str(int argc, Value argv[]);

ObjClass* native_nil_class_partial_init() {
  ObjClass* nil_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  nil_class->__get_prop = nil_get_prop;
  nil_class->__set_prop = native_set_prop_not_supported;  // Not supported
  nil_class->__get_subs = native_get_subs_not_supported;  // Not supported
  nil_class->__set_subs = native_set_subs_not_supported;  // Not supported
  nil_class->__equals   = nil_eq;
  nil_class->__hash     = nil_hash;

  return nil_class;
}

void native_nil_class_finalize() {
  define_native(&vm.nil_class->methods, STR(SP_METHOD_CTOR), nil_ctor, 1);
  define_native(&vm.nil_class->methods, STR(SP_METHOD_TO_STR), nil_to_str, 0);
  finalize_new_class(vm.nil_class);
}

static bool nil_eq(Value self, Value other) {
  return self.type == other.type && true;  // Nil is always equal to nil
}

static uint64_t nil_hash(Value self) {
  UNUSED(self);
  return 2;  // A prime number. Selected based on trial and error.
}

static bool nil_get_prop(Value receiver, ObjString* name, Value* result) {
  UNUSED(receiver);
  NATIVE_DEFAULT_GET_PROP_BODY(vm.nil_class)
}

/**
 * TYPENAME_NIL.SP_METHOD_CTOR() -> TYPENAME_NIL
 * @brief No-op constructor for TYPENAME_NIL.
 */
static Value nil_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  vm_error("Cannot instantiate " VALUE_STR_NIL " via " STR(TYPENAME_NIL) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_NIL.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of TYPENAME_NIL.
 */
static Value nil_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.nil_class)

  ObjString* str_obj = copy_string(VALUE_STR_NIL, STR_LEN(VALUE_STR_NIL));
  return str_value(str_obj);
}
