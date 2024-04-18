#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

#define BUILTIN_CHECK_RECEIVER_IS_FN()                                                                     \
  if (!IS_FUNCTION(argv[0]) && !IS_CLOSURE(argv[0]) && !IS_BOUND_METHOD(argv[0]) && !IS_NATIVE(argv[0])) { \
    runtime_error("Expected receiver of type " STR(TYPENAME_FUNCTION) ", " STR(TYPENAME_CLOSURE) ", " STR( \
                      TYPENAME_NATIVE) " or " STR(TYPENAME_BOUND_METHOD) ", but got %s.",                  \
                  typeof(argv[0])->name->chars);                                                           \
    return NIL_VAL;                                                                                        \
  }

static bool prop_getter(Obj* self, ObjString* name, Value* result);
static bool prop_setter(Obj* self, ObjString* name, Value value);
static bool index_getter(Obj* self, Value index, Value* result);
static bool index_setter(Obj* self, Value index, Value value);

void register_builtin_fn_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_FUNCTION, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, bind, 1);

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_FUNCTION, prop_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_FUNCTION, prop_setter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_FUNCTION, index_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_FUNCTION, index_setter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_FUNCTION);
}

// Internal OP_GET_PROPERTY handler
static bool prop_getter(Obj* self, ObjString* name, Value* result) {
  if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    switch (self->type) {
      case OBJ_FUNCTION: *result = OBJ_VAL(((ObjFunction*)self)->name); return true;
      case OBJ_CLOSURE: *result = OBJ_VAL(((ObjClosure*)self)->function->name); return true;
      case OBJ_NATIVE: *result = OBJ_VAL(((ObjNative*)self)->name); return true;
      case OBJ_BOUND_METHOD: return prop_getter(((ObjBoundMethod*)self)->method, name, result);
      default: INTERNAL_ERROR("Invalid object type for " STR(TYPENAME_FUNCTION) " property getter."); return false;
    }
  }

  return false;
}

// Internal OP_SET_PROPERTY handler
static bool prop_setter(Obj* self, ObjString* name, Value value) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(value);
  return false;
}

// Internal OP_GET_INDEX handler
static bool index_getter(Obj* self, Value index, Value* result) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(result);
  return false;
}

// Internal OP_SET_INDEX handler
static bool index_setter(Obj* self, Value index, Value value) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(value);
  return false;
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
  BUILTIN_CHECK_RECEIVER_IS_FN()
  BUILTIN_ARGC_EXACTLY(0)

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
  BUILTIN_CHECK_RECEIVER_IS_FN()
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  // Should align with prop_getter
  if (AS_STRING(argv[1]) == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    return BOOL_VAL(true);
  }

  return BOOL_VAL(false);
}

// Built-in method to bind a function to a receiver
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_FUNCTION,
    /* Name        */ bind,
    /* Arguments   */ DOC_ARG("bind_target", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_BOUND_METHOD,
    /* Description */
    "Returns a new " STR(TYPENAME_BOUND_METHOD) " with the given bind_target (receiver) bound to the function.");
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, bind) {
  BUILTIN_CHECK_RECEIVER_IS_FN()
  BUILTIN_ARGC_EXACTLY(1)

  Value bind_target = argv[1];
  if (!IS_OBJ(bind_target)) {
    runtime_error("Expected bind_target to be an object, but got %s.", typeof(bind_target)->name->chars);
    return NIL_VAL;
  }

  return OBJ_VAL(new_bound_method(bind_target, AS_OBJ(argv[0])));
}
