#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool num_get_prop(Value receiver, ObjString* name, Value* result);
static bool int_get_prop(Value receiver, ObjString* name, Value* result);
static bool float_get_prop(Value receiver, ObjString* name, Value* result);
NATIVE_SET_PROP_NOT_SUPPORTED()
NATIVE_GET_SUBS_NOT_SUPPORTED()
NATIVE_SET_SUBS_NOT_SUPPORTED()

static Value num_ctor(int argc, Value argv[]);
static Value int_ctor(int argc, Value argv[]);
static Value int_to_str(int argc, Value argv[]);
static Value float_ctor(int argc, Value argv[]);
static Value float_to_str(int argc, Value argv[]);

void finalize_native_num_class() {
  vm.num_class->get_property  = num_get_prop;
  vm.num_class->set_property  = set_prop_not_supported;  // Not supported
  vm.num_class->get_subscript = get_subs_not_supported;  // Not supported
  vm.num_class->set_subscript = set_subs_not_supported;  // Not supported

  define_native(&vm.num_class->methods, STR(SP_METHOD_CTOR), num_ctor, 1);
  finalize_new_class(vm.num_class);
}

void finalize_native_int_class() {
  vm.int_class->get_property  = int_get_prop;
  vm.int_class->set_property  = set_prop_not_supported;  // Not supported
  vm.int_class->get_subscript = get_subs_not_supported;  // Not supported
  vm.int_class->set_subscript = set_subs_not_supported;  // Not supported

  define_native(&vm.int_class->methods, STR(SP_METHOD_CTOR), int_ctor, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_TO_STR), int_to_str, 0);
  finalize_new_class(vm.int_class);
}

void finalize_native_float_class() {
  vm.float_class->get_property  = float_get_prop;
  vm.float_class->set_property  = set_prop_not_supported;  // Not supported
  vm.float_class->get_subscript = get_subs_not_supported;  // Not supported
  vm.float_class->set_subscript = set_subs_not_supported;  // Not supported

  define_native(&vm.float_class->methods, STR(SP_METHOD_CTOR), float_ctor, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_TO_STR), float_to_str, 0);
  finalize_new_class(vm.float_class);
}

static bool num_get_prop(Value receiver, ObjString* name, Value* result) {
  UNUSED(receiver);
  NATIVE_DEFAULT_GET_PROP_BODY(vm.num_class)
}

static bool int_get_prop(Value receiver, ObjString* name, Value* result) {
  UNUSED(receiver);
  NATIVE_DEFAULT_GET_PROP_BODY(vm.int_class)
}

static bool float_get_prop(Value receiver, ObjString* name, Value* result) {
  UNUSED(receiver);
  NATIVE_DEFAULT_GET_PROP_BODY(vm.float_class)
}

/**
 * TYPENAME_NUM.SP_METHOD_CTOR() -> TYPENAME_NUM
 * @brief No-op constructor for TYPENAME_NUM.
 */
static Value num_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a number via " STR(TYPENAME_NUM) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_CTOR() -> TYPENAME_INT
 * @brief No-op constructor for TYPENAME_INT.
 */
static Value int_ctor(int argc, Value argv[]) {
  UNUSED(argc);

  if (IS_INT(argv[1])) {
    return argv[1];
  }
  if (IS_FLOAT(argv[1])) {
    return int_value((long long)argv[1].as.float_);
  }
  if (IS_BOOL(argv[1])) {
    return int_value(argv[1].as.boolean ? 1 : 0);
  }
  if (IS_NIL(argv[1])) {
    return int_value(0);
  }
  if (IS_STRING(argv[1])) {
    ObjString* str = AS_STRING(argv[1]);
    return int_value((long long)string_to_double(str->chars, str->length));
  }

  return int_value(0);
}

/**
 * TYPENAME_INT.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of an TYPENAME_INT.
 */
static Value int_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(INT)

  char buffer[100];
  int len = snprintf(buffer, sizeof(buffer), VALUE_STR_INT, argv[0].as.integer);

  ObjString* str_obj = copy_string(buffer, len);
  return str_value(str_obj);
}

/**
 * TYPENAME_FLOAT.SP_METHOD_CTOR() -> TYPENAME_FLOAT
 * @brief No-op constructor for TYPENAME_FLOAT.
 */
static Value float_ctor(int argc, Value argv[]) {
  UNUSED(argc);

  if (IS_INT(argv[1])) {
    return float_value((double)argv[1].as.integer);
  }
  if (IS_FLOAT(argv[1])) {
    return argv[1];
  }
  if (IS_BOOL(argv[1])) {
    return float_value(argv[1].as.boolean ? 1.0 : 0.0);
  }
  if (IS_NIL(argv[1])) {
    return float_value(0.0);
  }
  if (IS_STRING(argv[1])) {
    ObjString* str = AS_STRING(argv[1]);
    return float_value(string_to_double(str->chars, str->length));
  }

  return float_value(0.0);
}

/**
 * TYPENAME_FLOAT.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of a TYPENAME_FLOAT.
 */
static Value float_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(FLOAT)

  char buffer[100];
  int len = snprintf(buffer, sizeof(buffer), VALUE_STR_FLOAT, argv[0].as.float_);

  // Remove trailing zeros. Ugh...
  // TODO (optimize): This is not very efficient, find a better way to do this
  while (buffer[len - 1] == '0') {
    buffer[--len] = '\0';
  }

  if (buffer[len - 1] == '.') {
    buffer[--len] = '\0';
  }

  ObjString* str_obj = copy_string(buffer, len);
  return str_value(str_obj);
}
