#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

#define BUILTIN_CHECK_RECEIVER_IS_FN()                                                                     \
  if (!is_fn(argv[0])) {                                                                                   \
    runtime_error("Expected receiver of type " STR(TYPENAME_FUNCTION) ", " STR(TYPENAME_CLOSURE) ", " STR( \
                      TYPENAME_NATIVE) " or " STR(TYPENAME_BOUND_METHOD) ", but got %s.",                  \
                  argv[0].type->name->chars);                                                              \
    return nil_value();                                                                                    \
  }

void finalize_builtin_fn_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_FUNCTION, bind, 1);

  BUILTIN_FINALIZE_CLASS(TYPENAME_FUNCTION);
}

/**
 * TYPENAME_FUNCTION.SP_METHOD_CTOR() -> TYPENAME_FUNCTION
 * @brief No-op constructor for TYPENAME_FUNCTION.
 */
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a function via " STR(TYPENAME_FUNCTION) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_FUNCTION.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of a TYPENAME_FUNCTION.
 */
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER_IS_FN()

  Value fn = argv[0];

  ObjString* name = NULL;
  const char* fmt = VALUE_STRFMT_FUNCTION;
  size_t fmt_len  = VALUE_STRFMT_FUNCTION_LEN;

  // Bound methods can be closures or native functions
  if (IS_BOUND_METHOD(fn)) {
    fn = fn_value((Obj*)AS_BOUND_METHOD(fn)->method);
  }

  // Closures are functions
  if (IS_CLOSURE(fn)) {
    fn = fn_value((Obj*)AS_CLOSURE(fn)->function);
  }

  // Native functions
  if (IS_NATIVE(fn)) {
    name    = AS_NATIVE(fn)->name;
    fmt     = VALUE_STRFMT_NATIVE;
    fmt_len = VALUE_STRFMT_NATIVE_LEN;
  } else {
    name = AS_FUNCTION(fn)->name;
  }

  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(str_value(name));  // GC Protection

  size_t buf_size = fmt_len + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, fmt, name->chars);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return str_value(str_obj);
}

/**
 * TYPENAME_FUNCTION.SP_METHOD_HAS(name: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the function has a property with the given name, VALUE_STR_FALSE otherwise.
 */
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, SP_METHOD_HAS) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER_IS_FN()
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* name = AS_STRING(argv[1]);

  // Execute the value_get_property function to see if the fn has the thing. We use this approach to make sure the two are
  // aligned and return the same result.
  push(argv[0]);
  if (value_get_property(name)) {
    pop();  // The result
    return bool_value(true);
  }
  if (vm.flags & VM_FLAG_HAS_ERROR) {
    return nil_value();
  }

  pop();  // The fn

  return bool_value(false);
}

/**
 * TYPENAME_FUNCTION.bind(bind_target: OBJ) -> TYPENAME_BOUND_METHOD
 * @brief Returns a new TYPENAME_BOUND_METHOD with the given bind_target (receiver) bound to the function.
 */
BUILTIN_METHOD_IMPL(TYPENAME_FUNCTION, bind) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER_IS_FN()
  BUILTIN_CHECK_ARG_AT(1, OBJ)

  Value bind_target = argv[1];
  return fn_value((Obj*)new_bound_method(bind_target, argv[0].as.obj));
}
