#include <stdio.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool num_get_prop(Value receiver, ObjString* name, Value* result);

static bool int_get_prop(Value receiver, ObjString* name, Value* result);
static bool int_eq(Value self, Value other);
static uint64_t int_hash(Value self);

static bool float_get_prop(Value receiver, ObjString* name, Value* result);
static bool float_eq(Value self, Value other);
static uint64_t float_hash(Value self);

static Value num_ctor(int argc, Value argv[]);

static Value int_ctor(int argc, Value argv[]);
static Value int_to_str(int argc, Value argv[]);

static Value float_ctor(int argc, Value argv[]);
static Value float_to_str(int argc, Value argv[]);

ObjClass* native_num_class_partial_init() {
  ObjClass* num_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  num_class->__get_prop = num_get_prop;
  num_class->__set_prop = native_set_prop_not_supported;  // Not supported
  num_class->__get_subs = native_get_subs_not_supported;  // Not supported
  num_class->__set_subs = native_set_subs_not_supported;  // Not supported
  num_class->__equals   = native_equals_not_supported;    // Not supported
  num_class->__hash     = native_hash_not_supported;      // Not supported

  return num_class;
}

void native_num_class_finalize() {
  define_native(&vm.num_class->methods, STR(SP_METHOD_CTOR), num_ctor, 1);
  finalize_new_class(vm.num_class);
}

ObjClass* native_int_class_partial_init(ObjClass* num_base_class) {
  ObjClass* int_class = new_class(NULL, num_base_class);  // Names are null because hashtables are not yet initialized

  int_class->__get_prop = int_get_prop;
  int_class->__set_prop = native_set_prop_not_supported;  // Not supported
  int_class->__get_subs = native_get_subs_not_supported;  // Not supported
  int_class->__set_subs = native_set_subs_not_supported;  // Not supported
  int_class->__equals   = int_eq;
  int_class->__hash     = int_hash;

  return int_class;
}

void native_int_class_finalize() {
  define_native(&vm.int_class->methods, STR(SP_METHOD_CTOR), int_ctor, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_TO_STR), int_to_str, 0);
  finalize_new_class(vm.int_class);
}

ObjClass* native_float_class_partial_init(ObjClass* num_base_class) {
  ObjClass* float_class = new_class(NULL, num_base_class);  // Names are null because hashtables are not yet initialized

  float_class->__get_prop = float_get_prop;
  float_class->__set_prop = native_set_prop_not_supported;  // Not supported
  float_class->__get_subs = native_get_subs_not_supported;  // Not supported
  float_class->__set_subs = native_set_subs_not_supported;  // Not supported
  float_class->__equals   = float_eq;
  float_class->__hash     = float_hash;

  return float_class;
}

void native_float_class_finalize() {
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

static bool int_eq(Value self, Value other) {
  return self.type == other.type && self.as.integer == other.as.integer;
}

static bool float_eq(Value self, Value other) {
  return self.type == other.type && self.as.float_ == other.as.float_;
}

static uint64_t int_hash(Value self) {
  return (uint64_t)self.as.integer;  // Bc of 2's complement, directly casting to uint64_t should ensure unique hash values.
}

static uint64_t float_hash(Value self) {
  // Hashes a double. Borrowed from Lua.
  union BitCast {
    double source;
    uint64_t target;
  };

  union BitCast cast;
  cast.source = (self.as.float_) + 1.0;
  return cast.target;
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

  if (is_int(argv[1])) {
    return argv[1];
  }
  if (is_float(argv[1])) {
    return int_value((long long)argv[1].as.float_);
  }
  if (is_bool(argv[1])) {
    return int_value(argv[1].as.boolean ? 1 : 0);
  }
  if (is_nil(argv[1])) {
    return int_value(0);
  }
  if (is_str(argv[1])) {
    ObjString* str = AS_STR(argv[1]);
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
  NATIVE_CHECK_RECEIVER(vm.int_class)

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

  if (is_int(argv[1])) {
    return float_value((double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return argv[1];
  }
  if (is_bool(argv[1])) {
    return float_value(argv[1].as.boolean ? 1.0 : 0.0);
  }
  if (is_nil(argv[1])) {
    return float_value(0.0);
  }
  if (is_str(argv[1])) {
    ObjString* str = AS_STR(argv[1]);
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
  NATIVE_CHECK_RECEIVER(vm.float_class)

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
