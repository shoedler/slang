#include "builtin.h"
#include "common.h"
#include "vm.h"

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result);
static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value);
static NativeAccessorResult index_getter(Obj* self, Value index, Value* result);
static NativeAccessorResult index_setter(Obj* self, Value index, Value value);

void register_builtin_bool_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_BOOL, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, SP_METHOD_HAS, 1);

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_BOOL, prop_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_BOOL, prop_setter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_BOOL, index_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_BOOL, index_setter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_BOOL);
}

// Internal OP_GET_PROPERTY handler
static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(result);
  return ACCESSOR_RESULT_PASS;
}

// Internal OP_SET_PROPERTY handler
static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(value);
  return ACCESSOR_RESULT_PASS;
}

// Internal OP_GET_INDEX handler
static NativeAccessorResult index_getter(Obj* self, Value index, Value* result) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(result);
  return ACCESSOR_RESULT_PASS;
}

// Internal OP_SET_INDEX handler
static NativeAccessorResult index_setter(Obj* self, Value index, Value value) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(value);
  return ACCESSOR_RESULT_PASS;
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
  BUILTIN_ARGC_EXACTLY(0)

  BUILTIN_CHECK_RECEIVER(BOOL)

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
    "<Not supported>");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, SP_METHOD_HAS) {
  BUILTIN_ARGC_EXACTLY(1)
  // Should align prop_getter
  runtime_error("Type " STR(TYPENAME_BOOL) " does not support '" STR(SP_METHOD_HAS) "'.");
  return NIL_VAL;
}