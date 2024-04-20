#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

#define BUILTIN_CHECK_RECEIVER_IS_FN()                                                                     \
  if (!is_callable(argv[0])) {                                                                             \
    runtime_error("Expected receiver of type " STR(TYPENAME_FUNCTION) ", " STR(TYPENAME_CLOSURE) ", " STR( \
                      TYPENAME_NATIVE) " or " STR(TYPENAME_BOUND_METHOD) ", but got %s.",                  \
                  typeof(argv[0])->name->chars);                                                           \
    return NIL_VAL;                                                                                        \
  }

void register_builtin_fn_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_FUNCTION, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, bind, 1);

  BUILTIN_FINALIZE_CLASS(TYPENAME_FUNCTION);
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
    "Returns " STR(TYPENAME_TRUE) " if the fn has a property with the given name, " STR(TYPENAME_FALSE) " otherwise.");
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_HAS) {
  BUILTIN_CHECK_RECEIVER_IS_FN()
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* name = AS_STRING(argv[1]);
  Obj* fn         = AS_OBJ(argv[0]);

  // Execute the value_get_property function to see if the fn has the thing. We use this approach to make sure the two are
  // aligned and return the same result.
  push(OBJ_VAL(fn));
  if (value_get_property(name)) {
    pop();  // The result
    return BOOL_VAL(true);
  }
  if (vm.flags & VM_FLAG_HAS_ERROR) {
    return NIL_VAL;
  }

  pop();  // The fn

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
