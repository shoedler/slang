#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result);
static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value);
static NativeAccessorResult index_getter(Obj* self, Value index, Value* result);
static NativeAccessorResult index_setter(Obj* self, Value index, Value value);

void register_builtin_fn_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_FUNCTION, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_HAS, 1);
  vm.__builtin_Fn_class->prop_getter  = prop_getter;
  vm.__builtin_Fn_class->prop_setter  = prop_setter;
  vm.__builtin_Fn_class->index_getter = index_getter;
  vm.__builtin_Fn_class->index_setter = index_setter;
  BUILTIN_FINALIZE_CLASS(TYPENAME_FUNCTION);
}

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result) {
  if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    switch (self->type) {
      case OBJ_FUNCTION: *result = OBJ_VAL(((ObjFunction*)self)->name); return ACCESSOR_RESULT_OK;
      case OBJ_CLOSURE: *result = OBJ_VAL(((ObjClosure*)self)->function->name); return ACCESSOR_RESULT_OK;
      case OBJ_NATIVE: *result = OBJ_VAL(((ObjNative*)self)->name); return ACCESSOR_RESULT_OK;
      case OBJ_BOUND_METHOD: return prop_getter(((ObjBoundMethod*)self)->method, name, result);
      default:
        INTERNAL_ERROR("Invalid object type for " STR(TYPENAME_FUNCTION) " property getter.");
        return ACCESSOR_RESULT_ERROR;
    }
  }

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

// Built-in fn constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_FUNCTION,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_FUNCTION,
    /* Description */
    "Returns " STR(TYPENAME_FUNCTION) ".");
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a function via " STR(TYPENAME_FUNCTION) "." STR(SP_METHOD_CTOR) ".");
  return NIL_VAL;
}

// Built-in method to convert a fn to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_FUNCTION,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_FUNCTION) ".");
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  // BUILTIN_CHECK_RECEIVER(FUNCTION) Doesn't work here, because we need to check multiple types

  if (!IS_FUNCTION(argv[0]) && !IS_CLOSURE(argv[0]) && !IS_BOUND_METHOD(argv[0]) && !IS_NATIVE(argv[0])) {
    runtime_error("Expected receiver of type " STR(TYPENAME_FUNCTION) ", " STR(TYPENAME_CLOSURE) ", " STR(
                      TYPENAME_NATIVE) " or " STR(TYPENAME_BOUND_METHOD) ", but got %s.",
                  typeof(argv[0])->name->chars);
    return NIL_VAL;
  }

  Value fn = argv[0];

  // Bound methods can be closures or native functions
  if (IS_BOUND_METHOD(fn)) {
    fn = OBJ_VAL(AS_BOUND_METHOD(fn)->method);
  }

  // Closures are functions
  if (IS_CLOSURE(fn)) {
    fn = OBJ_VAL(AS_CLOSURE(fn)->function);
  }

  // Native functions
  if (IS_NATIVE(fn)) {
    return OBJ_VAL(copy_string(VALUE_STR_NATIVE, STR_LEN(VALUE_STR_NATIVE)));
  }

  // It's a function
  ObjFunction* function = AS_FUNCTION(fn);
  ObjString* name       = function->name;
  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(OBJ_VAL(name));  // GC Protection

  size_t buf_size = VALUE_STRFMT_FUNCTION_LEN + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFMT_FUNCTION, name->chars);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return OBJ_VAL(str_obj);
}

// Built-in method to check if a value has a property
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_FUNCTION,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("name", TYPENAME_STRING),
    /* Return Type */ TYPENAME_FUNCTION,
    /* Description */
    "<Not supported>");
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_HAS) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)
  // BUILTIN_CHECK_RECEIVER(FUNCTION) Doesn't work here, because we need to check multiple types
  if (!IS_FUNCTION(argv[0]) && !IS_CLOSURE(argv[0]) && !IS_BOUND_METHOD(argv[0]) && !IS_NATIVE(argv[0])) {
    runtime_error("Expected receiver of type " STR(TYPENAME_FUNCTION) ", " STR(TYPENAME_CLOSURE) ", " STR(
                      TYPENAME_NATIVE) " or " STR(TYPENAME_BOUND_METHOD) ", but got %s.",
                  typeof(argv[0])->name->chars);
    return NIL_VAL;
  }

  // Should align with prop_getter
  if (AS_STRING(argv[1]) == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    return BOOL_VAL(true);
  }

  return BOOL_VAL(false);
}