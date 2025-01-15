#include <math.h>
#include <stdio.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "parser.h"
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
static Value int_add(int argc, Value argv[]);
static Value int_sub(int argc, Value argv[]);
static Value int_mul(int argc, Value argv[]);
static Value int_div(int argc, Value argv[]);
static Value int_mod(int argc, Value argv[]);
static Value int_lt(int argc, Value argv[]);
static Value int_gt(int argc, Value argv[]);
static Value int_lteq(int argc, Value argv[]);
static Value int_gteq(int argc, Value argv[]);

static Value float_ctor(int argc, Value argv[]);
static Value float_to_str(int argc, Value argv[]);
static Value float_add(int argc, Value argv[]);
static Value float_sub(int argc, Value argv[]);
static Value float_mul(int argc, Value argv[]);
static Value float_div(int argc, Value argv[]);
static Value float_mod(int argc, Value argv[]);
static Value float_lt(int argc, Value argv[]);
static Value float_gt(int argc, Value argv[]);
static Value float_lteq(int argc, Value argv[]);
static Value float_gteq(int argc, Value argv[]);

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
  // We do need a "ctor" which is used to throw an error when trying to instantiate an "abstract" class:
  define_native(&vm.num_class->methods, STR(SP_METHOD_CTOR), num_ctor, 1);

  // These are also not available for TYPENAME_INT and TYPENAME_FLOAT:
  define_native(&vm.num_class->methods, STR(SP_METHOD_HAS), native___has_not_supported, 1);
  define_native(&vm.num_class->methods, STR(SP_METHOD_SLICE), native___slice_not_supported, 2);

  // No need for any other SP_METHODs, because TYPENAME_NUM is an abstract class and TYPENAME_INT and TYPENAME_FLOAT should
  // implement them.

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
  define_native(&vm.int_class->methods, STR(SP_METHOD_ADD), int_add, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_SUB), int_sub, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_MUL), int_mul, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_DIV), int_div, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_MOD), int_mod, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_LT), int_lt, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_GT), int_gt, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_LTEQ), int_lteq, 1);
  define_native(&vm.int_class->methods, STR(SP_METHOD_GTEQ), int_gteq, 1);

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
  define_native(&vm.float_class->methods, STR(SP_METHOD_ADD), float_add, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_SUB), float_sub, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_MUL), float_mul, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_DIV), float_div, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_MOD), float_mod, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_LT), float_lt, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_GT), float_gt, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_LTEQ), float_lteq, 1);
  define_native(&vm.float_class->methods, STR(SP_METHOD_GTEQ), float_gteq, 1);

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
  if (self.type == other.type) {
    return self.as.integer == other.as.integer;
  }
  if (is_float(other)) {
    return (double)self.as.integer == other.as.float_;
  }
  return false;
}

static bool float_eq(Value self, Value other) {
  if (self.type == other.type) {
    return self.as.float_ == other.as.float_;
  }
  if (is_int(other)) {
    return self.as.float_ == (double)other.as.integer;
  }
  return false;
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
  vm_error("Cannot instantiate a number via " STR(TYPENAME_NUM) "." STR(SP_METHOD_CTOR) ".");
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
    Value num      = parse_number(str->chars, str->length);
    if (!is_int(num)) {
      // Then it must be a float, which, in this case, we cast to an int.
      double float_val = num.as.float_;
      return int_value((long long)float_val);
    }
    return num;
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
 * TYPENAME_INT.SP_METHOD_ADD(other: TYPENAME_NUM) -> TYPENAME_NUM
 * @brief Adds a TYPENAME_NUM to a TYPENAME_INT.
 */
static Value int_add(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return int_value(argv[0].as.integer + argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return float_value((double)argv[0].as.integer + argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(+)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_SUB(other: TYPENAME_NUM) -> TYPENAME_NUM
 * @brief Subtracts a TYPENAME_NUM from a TYPENAME_INT.
 */
static Value int_sub(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return int_value(argv[0].as.integer - argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return float_value((double)argv[0].as.integer - argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(-)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_MUL(other: TYPENAME_NUM) -> TYPENAME_NUM
 * @brief Multiplies a TYPENAME_INT by a TYPENAME_NUM.
 */
static Value int_mul(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return int_value(argv[0].as.integer * argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return float_value((double)argv[0].as.integer * argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(*)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_DIV(other: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Divides a TYPENAME_INT by a TYPENAME_NUM.
 */
static Value int_div(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    if (argv[1].as.integer == 0) {
      vm_error("Division by zero.");
      return nil_value();
    }
    return float_value((double)argv[0].as.integer / (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    if (argv[1].as.float_ == 0.0) {
      vm_error("Division by zero.");
      return nil_value();
    }
    return float_value((double)argv[0].as.integer / argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(/)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_MOD(other: TYPENAME_NUM) -> TYPENAME_NUM
 * @brief Modulos a TYPENAME_INT by a TYPENAME_NUM.
 */
static Value int_mod(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    if (argv[1].as.integer == 0) {
      vm_error("Modulo by zero.");
      return nil_value();
    }
    return int_value(argv[0].as.integer % argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    if (argv[1].as.float_ == 0.0) {
      vm_error("Modulo by zero.");
      return nil_value();
    }
    return float_value(fmod((double)argv[0].as.integer, argv[1].as.float_));
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(%)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_LT(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_INT is less than a TYPENAME_NUM.
 */
static Value int_lt(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.integer < argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value((double)argv[0].as.integer < argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(<)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_GT(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_INT is greater than a TYPENAME_NUM.
 */
static Value int_gt(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.integer > argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value((double)argv[0].as.integer > argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(>)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_LTEQ(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_INT is less than or equal to a TYPENAME_NUM.
 */
static Value int_lteq(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.integer <= argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value((double)argv[0].as.integer <= argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(<=)
  return nil_value();
}

/**
 * TYPENAME_INT.SP_METHOD_GTEQ(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_INT is greater than or equal to a TYPENAME_NUM.
 */
static Value int_gteq(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.int_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.integer >= argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value((double)argv[0].as.integer >= argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(>=)
  return nil_value();
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
    Value num      = parse_number(str->chars, str->length);
    if (!is_float(num)) {
      // Then it must be an int, which, in this case, we cast to a float.
      long long int_val = num.as.integer;
      return float_value((double)int_val);
    }
    return num;
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

/**
 * TYPENAME_FLOAT.SP_METHOD_ADD(other: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Adds a TYPENAME_NUM to a TYPENAME_FLOAT.
 */
static Value float_add(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return float_value(argv[0].as.float_ + (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return float_value(argv[0].as.float_ + argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(+)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_SUB(other: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Subtracts a TYPENAME_NUM from a TYPENAME_FLOAT.
 */
static Value float_sub(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return float_value(argv[0].as.float_ - (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return float_value(argv[0].as.float_ - argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(-)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_MUL(other: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Multiplies a TYPENAME_FLOAT by a TYPENAME_NUM.
 */
static Value float_mul(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return float_value(argv[0].as.float_ * (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return float_value(argv[0].as.float_ * argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(*)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_DIV(other: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Divides a TYPENAME_FLOAT by a TYPENAME_NUM.
 */
static Value float_div(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    if (argv[1].as.integer == 0) {
      vm_error("Division by zero.");
      return nil_value();
    }
    return float_value(argv[0].as.float_ / (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    if (argv[1].as.float_ == 0.0) {
      vm_error("Division by zero.");
      return nil_value();
    }
    return float_value(argv[0].as.float_ / argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(/)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_MOD(other: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Modulos a TYPENAME_FLOAT by a TYPENAME_NUM.
 */
static Value float_mod(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    if (argv[1].as.integer == 0) {
      vm_error("Modulo by zero.");
      return nil_value();
    }
    return float_value(fmod(argv[0].as.float_, (double)argv[1].as.integer));
  }
  if (is_float(argv[1])) {
    if (argv[1].as.float_ == 0.0) {
      vm_error("Modulo by zero.");
      return nil_value();
    }
    return float_value(fmod(argv[0].as.float_, argv[1].as.float_));
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(%)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_LT(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_FLOAT is less than a TYPENAME_NUM.
 */
static Value float_lt(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.float_ < (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value(argv[0].as.float_ < argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(<)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_GT(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_FLOAT is greater than a TYPENAME_NUM.
 */
static Value float_gt(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.float_ > (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value(argv[0].as.float_ > argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(>)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_LTEQ(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_FLOAT is less than or equal to a TYPENAME_NUM.
 */
static Value float_lteq(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.float_ <= (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value(argv[0].as.float_ <= argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(<=)
  return nil_value();
}

/**
 * TYPENAME_FLOAT.SP_METHOD_GTEQ(other: TYPENAME_NUM) -> TYPENAME_BOOL
 * @brief Checks if a TYPENAME_FLOAT is greater than or equal to a TYPENAME_NUM.
 */
static Value float_gteq(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.float_class)
  if (is_int(argv[1])) {
    return bool_value(argv[0].as.float_ >= (double)argv[1].as.integer);
  }
  if (is_float(argv[1])) {
    return bool_value(argv[0].as.float_ >= argv[1].as.float_);
  }
  NATIVE_BIN_OP_ILLEGAL_TYPES(>=)
  return nil_value();
}
