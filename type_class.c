#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool class_get_prop(Value receiver, ObjString* name, Value* result);
NATIVE_SET_PROP_NOT_SUPPORTED()
NATIVE_GET_SUBS_NOT_SUPPORTED()
NATIVE_SET_SUBS_NOT_SUPPORTED()

static Value class_ctor(int argc, Value argv[]);
static Value class_to_str(int argc, Value argv[]);
static Value class_has(int argc, Value argv[]);

void finalize_native_class_class() {
  vm.class_class->get_property  = class_get_prop;
  vm.class_class->set_property  = set_prop_not_supported;  // Not supported
  vm.class_class->get_subscript = get_subs_not_supported;  // Not supported
  vm.class_class->set_subscript = set_subs_not_supported;  // Not supported

  define_native(&vm.class_class->methods, STR(SP_METHOD_CTOR), class_ctor, 1);
  define_native(&vm.class_class->methods, STR(SP_METHOD_TO_STR), class_to_str, 0);
  define_native(&vm.class_class->methods, STR(SP_METHOD_HAS), class_has, 1);
  finalize_new_class(vm.class_class);
}

static bool class_get_prop(Value receiver, ObjString* name, Value* result) {
  ObjClass* klass = AS_CLASS(receiver);
  if (hashtable_get_by_string(&klass->static_methods, name, result)) {
    return true;
  }
  // You can also get a class' ctor. Obviously, it'll not get bound to the class
  if (name == vm.special_method_names[SPECIAL_METHOD_CTOR]) {
    *result = klass->__ctor == NULL ? nil_value() : fn_value(klass->__ctor);
    return true;
  }
  if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
    *result = str_value(klass->name);
    return true;
  }
  NATIVE_DEFAULT_GET_PROP_BODY(vm.class_class)
}

/**
 * TYPENAME_CLASS.SP_METHOD_CTOR() -> TYPENAME_CLASS
 * @brief No-op constructor for TYPENAME_CLASS.
 */
static Value class_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a class via " STR(TYPENAME_CLASS) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_CLASS.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of a TYPENAME_CLASS.
 */
static Value class_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(CLASS)

  ObjClass* klass = AS_CLASS(argv[0]);
  ObjString* name = klass->name;
  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(str_value(name));

  size_t buf_size = VALUE_STRFMT_CLASS_LEN + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFMT_CLASS, name->chars);
  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return str_value(str_obj);
}

/**
 * TYPENAME_CLASS.SP_METHOD_HAS(name: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the class has a property with the given name, VALUE_STR_FALSE otherwise.
 */
static Value class_has(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(CLASS)
  NATIVE_CHECK_ARG_AT(1, STRING)

  ObjString* name = AS_STRING(argv[1]);

  // Execute the class_get_prop function to see if the class has the thing. We use this approach to make sure the two are
  // aligned and return the same result.
  Value result;
  if (class_get_prop(argv[0], name, &result)) {
    return bool_value(true);
  }
  clear_error();  // Clear the "Prop does not exist" error set by class_get_prop

  return bool_value(false);
}