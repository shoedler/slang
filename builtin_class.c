#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result);
static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value);
static NativeAccessorResult index_getter(Obj* self, Value index, Value* result);
static NativeAccessorResult index_setter(Obj* self, Value index, Value value);

void register_builtin_class_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_CLASS, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, SP_METHOD_HAS, 1);
  vm.__builtin_Class_class->prop_getter  = prop_getter;
  vm.__builtin_Class_class->prop_setter  = prop_setter;
  vm.__builtin_Class_class->index_getter = index_getter;
  vm.__builtin_Class_class->index_setter = index_setter;
  BUILTIN_FINALIZE_CLASS(TYPENAME_CLASS);
}

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result) {
  ObjClass* klass = (ObjClass*)self;
  if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    *result = OBJ_VAL(klass->name);
    return ACCESSOR_RESULT_OK;
  }

  if (name == vm.special_method_names[SPECIAL_METHOD_CTOR]) {
    *result = OBJ_VAL(klass->__ctor);
    return ACCESSOR_RESULT_OK;
  }

  // We do not bind the method, because it's a static method.
  if (hashtable_get_by_string(&klass->static_methods, name, result)) {
    return ACCESSOR_RESULT_OK;
  }

  // TODO: Check methods here (bind method)
  return ACCESSOR_RESULT_PASS;
}

static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(value);
  return ACCESSOR_RESULT_PASS;
}

static NativeAccessorResult index_getter(Obj* self, Value index, Value* result) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(result);
  return ACCESSOR_RESULT_PASS;
}

static NativeAccessorResult index_setter(Obj* self, Value index, Value value) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(value);
  return ACCESSOR_RESULT_PASS;
}

// Built-in class constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_CLASS,
    /* Description */
    "Returns " STR(TYPENAME_CLASS) ".");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a class via " STR(TYPENAME_CLASS) "." STR(SP_METHOD_CTOR) ".");
  return NIL_VAL;
}

// Built-in method to convert a class to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_CLASS) ".");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, SP_METHOD_TO_STR) {
  BUILTIN_CHECK_RECEIVER(CLASS)
  BUILTIN_ARGC_EXACTLY(0)

  ObjClass* klass = AS_CLASS(argv[0]);
  ObjString* name = klass->name;
  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(OBJ_VAL(name));

  size_t buf_size = VALUE_STRFMT_CLASS_LEN + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFMT_CLASS, name->chars);
  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return OBJ_VAL(str_obj);
}

// Built-in method to check if a class has a method
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("name", TYPENAME_STRING),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " STR(TYPENAME_TRUE) " if the class has a method or static method with the given name, " STR(
        TYPENAME_FALSE) " otherwise.");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, SP_METHOD_HAS) {
  BUILTIN_CHECK_RECEIVER(CLASS)
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* name = AS_STRING(argv[1]);
  ObjClass* klass = AS_CLASS(argv[0]);
  Value result;

  // Should align with prop_getter
  if (hashtable_get_by_string(&klass->methods, name, &result)) {
    return BOOL_VAL(true);
  }
  if (hashtable_get_by_string(&klass->static_methods, name, &result)) {
    return BOOL_VAL(true);
  }

  return BOOL_VAL(false);
}