#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool prop_getter(Obj* self, ObjString* name, Value* result);

void register_builtin_bool_class() {
  BUILTIN_REGISTER_BASE_CLASS(TYPENAME_BOOL);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_HAS, 1);

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_BOOL, prop_getter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_BOOL);
}

// Internal OP_GET_PROPERTY handler
static bool prop_getter(Obj* self, ObjString* name, Value* result) {
  UNUSED(self);
  if (bind_method(vm.__builtin_Bool_class, name, result)) {
    return true;
  }
  return false;
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
  BUILTIN_ARGC_EXACTLY(1)

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
  BUILTIN_CHECK_RECEIVER(BOOL)
  BUILTIN_ARGC_EXACTLY(0)

  if (AS_BOOL(argv[0])) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, STR_LEN(VALUE_STR_TRUE));
    return OBJ_VAL(str_obj);
  } else {
    ObjString* str_obj = copy_string(VALUE_STR_FALSE, STR_LEN(VALUE_STR_FALSE));
    return OBJ_VAL(str_obj);
  }
}

// Built-in method to check if a value has a property
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("name", TYPENAME_STRING),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE " if the " STR(TYPENAME_BOOL) " class has a method with the given name, otherwise " VALUE_STR_FALSE
                                                            ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, SP_METHOD_HAS) {
  BUILTIN_CHECK_RECEIVER(BOOL)
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  // Should align with prop_getter
  ObjString* name = AS_STRING(argv[1]);
  Value discard;
  if (hashtable_get_by_string(&vm.__builtin_Bool_class->methods, name, &discard)) {
    return BOOL_VAL(true);
  }

  return BOOL_VAL(false);
}
