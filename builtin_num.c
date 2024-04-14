#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_num_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_NUMBER, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, __ctor, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, to_str, 0);
}

// Built-in number constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, __ctor) {
  BUILTIN_ARGC_EXACTLY(1)

  switch (argv[1].type) {
    case VAL_NUMBER: return argv[1];
    case VAL_BOOL: return AS_BOOL(argv[1]) ? NUMBER_VAL(1) : NUMBER_VAL(0);
    case VAL_NIL: return NUMBER_VAL(0);
    case VAL_OBJ: {
      switch (AS_OBJ(argv[1])->type) {
        case OBJ_STRING: {
          ObjString* str = AS_STRING(argv[1]);
          return NUMBER_VAL(string_to_double(str->chars, str->length));
        }
      }
    }
  }

  return NUMBER_VAL(0);
}

// Built-in method to convert a number to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, to_str) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(NUMBER)

  double number = AS_NUMBER(argv[0]);
  char buffer[64];
  long long integer;
  int len = 0;

  if (is_int(number, &integer)) {
    len = snprintf(buffer, sizeof(buffer), VALUE_STR_INT, integer);
  } else {
    len = snprintf(buffer, sizeof(buffer), VALUE_STR_FLOAT, number);

    // Remove trailing zeros. Ugh...
    // TODO (optimize): This is not very efficient, find a better way to do this
    while (buffer[len - 1] == '0') {
      buffer[--len] = '\0';
    }

    if (buffer[len - 1] == '.') {
      buffer[--len] = '\0';
    }
  }

  ObjString* str_obj = copy_string(buffer, len);
  return OBJ_VAL(str_obj);
}