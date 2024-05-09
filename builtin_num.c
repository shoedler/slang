#include "builtin.h"
#include "common.h"
#include "vm.h"

void finalize_builtin_num_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_NUM, SP_METHOD_CTOR, 1);
  BUILTIN_FINALIZE_CLASS(TYPENAME_NUM);
}

/**
 * TYPENAME_NUM.SP_METHOD_CTOR() -> TYPENAME_NUM
 * @brief No-op constructor for TYPENAME_NUM.
 */
BUILTIN_METHOD_IMPL(TYPENAME_NUM, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a number via " STR(TYPENAME_NUM) "." STR(SP_METHOD_CTOR) ".");
  return nil_value();
}

void finalize_builtin_int_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_INT, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_INT, SP_METHOD_TO_STR, 0);
  BUILTIN_FINALIZE_CLASS(TYPENAME_INT);
}

/**
 * TYPENAME_INT.SP_METHOD_CTOR() -> TYPENAME_INT
 * @brief No-op constructor for TYPENAME_INT.
 */
BUILTIN_METHOD_IMPL(TYPENAME_INT, SP_METHOD_CTOR) {
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
BUILTIN_METHOD_IMPL(TYPENAME_INT, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(INT)

  char buffer[100];
  int len = snprintf(buffer, sizeof(buffer), VALUE_STR_INT, argv[0].as.integer);

  ObjString* str_obj = copy_string(buffer, len);
  return str_value(str_obj);
}

void finalize_builtin_float_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_FLOAT, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_FLOAT, SP_METHOD_TO_STR, 0);
  BUILTIN_FINALIZE_CLASS(TYPENAME_FLOAT);
}

/**
 * TYPENAME_FLOAT.SP_METHOD_CTOR() -> TYPENAME_FLOAT
 * @brief No-op constructor for TYPENAME_FLOAT.
 */
BUILTIN_METHOD_IMPL(TYPENAME_FLOAT, SP_METHOD_CTOR) {
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
BUILTIN_METHOD_IMPL(TYPENAME_FLOAT, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(FLOAT)

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