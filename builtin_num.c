#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_num_class() {
  BUILTIN_REGISTER_BASE_CLASS(TYPENAME_NUM);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUM, SP_METHOD_CTOR, 0);
  BUILTIN_FINALIZE_CLASS(TYPENAME_NUM);
}

// Built-in Num constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUM,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUM,
    /* Description */
    "No-op constructor for " STR(TYPENAME_NUM) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUM, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a number via " STR(TYPENAME_NUM) "." STR(SP_METHOD_CTOR) ".");
  return NIL_VAL;
}

void register_builtin_int_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_INT, TYPENAME_NUM);
  BUILTIN_REGISTER_METHOD(TYPENAME_INT, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_INT, SP_METHOD_TO_STR, 0);
  BUILTIN_FINALIZE_CLASS(TYPENAME_INT);
}

// Built-in number constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_INT,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_INT,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_INT) ".");
BUILTIN_METHOD_IMPL(TYPENAME_INT, SP_METHOD_CTOR) {
  BUILTIN_ARGC_EXACTLY(1)

  switch (argv[1].type) {
    case VAL_INT: return argv[1];
    case VAL_FLOAT: return INT_VAL((long long)AS_FLOAT(argv[1]));
    case VAL_BOOL: return AS_BOOL(argv[1]) ? INT_VAL(1) : INT_VAL(0);
    case VAL_NIL: return INT_VAL(0);
    case VAL_OBJ: {
      switch (AS_OBJ(argv[1])->type) {
        case OBJ_STRING: {
          ObjString* str = AS_STRING(argv[1]);
          return INT_VAL((long long)string_to_double(str->chars, str->length));
        }
      }
    }
  }

  return INT_VAL(0);
}

// Built-in method to convert a number to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_INT,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_INT) ".");
BUILTIN_METHOD_IMPL(TYPENAME_INT, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(INT)

  char buffer[100];
  int len = snprintf(buffer, sizeof(buffer), VALUE_STR_INT, AS_INT(argv[0]));

  ObjString* str_obj = copy_string(buffer, len);
  return OBJ_VAL(str_obj);
}

void register_builtin_float_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_FLOAT, TYPENAME_NUM);
  BUILTIN_REGISTER_METHOD(TYPENAME_FLOAT, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_FLOAT, SP_METHOD_TO_STR, 0);
  BUILTIN_FINALIZE_CLASS(TYPENAME_FLOAT);
}

// Built-in number constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_FLOAT,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_FLOAT,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_FLOAT) ".");
BUILTIN_METHOD_IMPL(TYPENAME_FLOAT, SP_METHOD_CTOR) {
  BUILTIN_ARGC_EXACTLY(1)

  switch (argv[1].type) {
    case VAL_INT: return FLOAT_VAL((double)AS_INT(argv[1]));
    case VAL_FLOAT: return argv[1];
    case VAL_BOOL: return AS_BOOL(argv[1]) ? FLOAT_VAL(1) : FLOAT_VAL(0);
    case VAL_NIL: return FLOAT_VAL(0);
    case VAL_OBJ: {
      switch (AS_OBJ(argv[1])->type) {
        case OBJ_STRING: {
          ObjString* str = AS_STRING(argv[1]);
          return FLOAT_VAL(string_to_double(str->chars, str->length));
        }
      }
    }
  }

  return FLOAT_VAL(0);
}

// Built-in method to convert a number to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_FLOAT,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_FLOAT) ".");
BUILTIN_METHOD_IMPL(TYPENAME_FLOAT, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(FLOAT)

  char buffer[100];
  int len = snprintf(buffer, sizeof(buffer), VALUE_STR_FLOAT, AS_FLOAT(argv[0]));

  // Remove trailing zeros. Ugh...
  // TODO (optimize): This is not very efficient, find a better way to do this
  while (buffer[len - 1] == '0') {
    buffer[--len] = '\0';
  }

  if (buffer[len - 1] == '.') {
    buffer[--len] = '\0';
  }

  ObjString* str_obj = copy_string(buffer, len);
  return OBJ_VAL(str_obj);
}