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
  vm.__builtin_Nil_class->prop_getter  = prop_getter;
  vm.__builtin_Nil_class->prop_setter  = prop_setter;
  vm.__builtin_Nil_class->index_getter = index_getter;
  vm.__builtin_Nil_class->index_setter = index_setter;
  BUILTIN_FINALIZE_CLASS(TYPENAME_NIL);
}

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(result);
  return ACCESSOR_RESULT_PASS;  // Still allow to bind methods
}

static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value) {
  UNUSED(self);
  UNUSED(value);
  runtime_error("Cannot set property '%s' on " STR(TYPENAME_NIL) ".", name->chars);
  return ACCESSOR_RESULT_ERROR;
}

static NativeAccessorResult index_getter(Obj* self, Value index, Value* result) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(result);
  runtime_error("Cannot get index on " STR(TYPENAME_NIL) ".");
  return ACCESSOR_RESULT_ERROR;
}

static NativeAccessorResult index_setter(Obj* self, Value index, Value value) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(value);
  runtime_error("Cannot set index on " STR(TYPENAME_NIL) ".");
  return ACCESSOR_RESULT_ERROR;
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