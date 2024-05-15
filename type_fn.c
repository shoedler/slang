#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

#define NATIVE_CHECK_RECEIVER_IS_FN()                                                                      \
  if (!is_fn(argv[0])) {                                                                                   \
    runtime_error("Expected receiver of type " STR(TYPENAME_FUNCTION) ", " STR(TYPENAME_CLOSURE) ", " STR( \
                      TYPENAME_NATIVE) " or " STR(TYPENAME_BOUND_METHOD) ", but got %s.",                  \
                  argv[0].type->name->chars);                                                              \
    return nil_value();                                                                                    \
  }

static bool fn_get_prop(Value receiver, ObjString* name, Value* result);
NATIVE_SET_PROP_NOT_SUPPORTED()
NATIVE_GET_SUBS_NOT_SUPPORTED()
NATIVE_SET_SUBS_NOT_SUPPORTED()

static Value fn_ctor(int argc, Value argv[]);
static Value fn_to_str(int argc, Value argv[]);
static Value fn_has(int argc, Value argv[]);
static Value fn_bind(int argc, Value argv[]);

void finalize_native_fn_class() {
  vm.fn_class->get_property  = fn_get_prop;
  vm.fn_class->set_property  = set_prop_not_supported;  // Not supported
  vm.fn_class->get_subscript = get_subs_not_supported;  // Not supported
  vm.fn_class->set_subscript = set_subs_not_supported;  // Not supported

  define_native(&vm.fn_class->methods, STR(SP_METHOD_CTOR), fn_ctor, 1);
  define_native(&vm.fn_class->methods, STR(SP_METHOD_TO_STR), fn_to_str, 0);
  define_native(&vm.fn_class->methods, STR(SP_METHOD_HAS), fn_has, 1);
  define_native(&vm.fn_class->methods, "bind", fn_bind, 1);
  finalize_new_class(vm.fn_class);
}

static bool fn_get_prop(Value receiver, ObjString* name, Value* result) {
  if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    *result = str_value(fn_get_name(receiver));
    return true;
  }
  NATIVE_DEFAULT_GET_PROP_BODY(vm.fn_class)
}

/**
 * TYPENAME_FUNCTION.SP_METHOD_CTOR() -> TYPENAME_FUNCTION
 * @brief No-op constructor for TYPENAME_FUNCTION.
 */
static Value fn_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a function via " STR(TYPENAME_FUNCTION) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_FUNCTION.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of a TYPENAME_FUNCTION.
 */
static Value fn_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER_IS_FN()

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
static Value fn_has(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER_IS_FN()
  NATIVE_CHECK_ARG_AT(1, STRING)

  ObjString* name = AS_STRING(argv[1]);

  // Execute the fn_get_prop function to see if the fn has the thing. We use this approach to make sure the two are
  // aligned and return the same result.
  Value result;
  if (fn_get_prop(argv[0], name, &result)) {
    return bool_value(true);
  }
  clear_error();  // Clear the "Prop does not exist" error set by fn_get_prop

  return bool_value(false);
}

/**
 * TYPENAME_FUNCTION.bind(bind_target: OBJ) -> TYPENAME_BOUND_METHOD
 * @brief Returns a new TYPENAME_BOUND_METHOD with the given bind_target (receiver) bound to the function.
 */
static Value fn_bind(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER_IS_FN()
  NATIVE_CHECK_ARG_AT(1, OBJ)

  Value bind_target = argv[1];
  return fn_value((Obj*)new_bound_method(bind_target, argv[0].as.obj));
}
