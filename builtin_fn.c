#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_fn_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_FUNCTION, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_CTOR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_TO_STR, 0);
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