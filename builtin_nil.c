#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool prop_getter(Obj* self, ObjString* name, Value* result);

void register_builtin_nil_class() {
  BUILTIN_REGISTER_BASE_CLASS(TYPENAME_NIL);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_HAS, 1);

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_NIL, prop_getter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_NIL);
}

// Internal OP_GET_PROPERTY handler
static bool prop_getter(Obj* self, ObjString* name, Value* result) {
  UNUSED(self);
  if (bind_method(vm.__builtin_Nil_class, name, result)) {
    return true;
  }
  return false;
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

// Built-in method to check if a value has a property
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("name", TYPENAME_STRING),
    /* Return Type */ TYPENAME_NIL,
    /* Description */
    "Returns " VALUE_STR_TRUE " if the " STR(TYPENAME_NIL) " class has a method with the given name, otherwise " VALUE_STR_FALSE
                                                           ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_HAS) {
  BUILTIN_CHECK_RECEIVER(NIL)
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  // Should align with prop_getter
  ObjString* name = AS_STRING(argv[1]);
  Value discard;
  if (hashtable_get_by_string(&vm.__builtin_Nil_class->methods, name, &discard)) {
    return BOOL_VAL(true);
  }

  return BOOL_VAL(false);
}