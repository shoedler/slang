#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_num_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_NUMBER, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, __ctor);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, to_str);
}

// Built-in number constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Converts the first argument to a " STR(
        TYPENAME_NUMBER) ", if provided. Otherwise, returns a default " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, __ctor) {
  if (argc == 0) {
    return NUMBER_VAL(0);
  }

  BUILTIN_CHECK_ARGC_ONE()

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
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(NUMBER)

  double number = AS_NUMBER(argv[0]);
  char buffer[64];
  int integer;
  int len = 0;

  if (is_int(number, &integer)) {
    len = snprintf(buffer, sizeof(buffer), VALUE_STR_INT, integer);
  } else {
    len = snprintf(buffer, sizeof(buffer), VALUE_STR_FLOAT, number);
  }

  ObjString* str_obj = copy_string(buffer, len);
  return OBJ_VAL(str_obj);
}