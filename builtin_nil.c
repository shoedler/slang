#include "builtin.h"
#include "common.h"
#include "vm.h"

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result);
static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value);
static NativeAccessorResult index_getter(Obj* self, Value index, Value* result);
static NativeAccessorResult index_setter(Obj* self, Value index, Value value);

void register_builtin_nil_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_NIL, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, SP_METHOD_HAS, 1);

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_NIL, prop_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_NIL, prop_setter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_NIL, index_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_NIL, index_setter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_NIL);
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

// Built-in nil constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NIL,
    /* Description */
    "Returns " STR(TYPENAME_NIL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_CTOR) {
  BUILTIN_ARGC_EXACTLY(0)
  UNUSED(argv);
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
    "<Not supported>");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, SP_METHOD_HAS) {
  BUILTIN_ARGC_EXACTLY(1)
  // Should align with prop_getter
  runtime_error("Type " STR(TYPENAME_NIL) " does not support '" STR(SP_METHOD_HAS) "'.");
  return NIL_VAL;
}