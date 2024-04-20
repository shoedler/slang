#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_num_class() {
  BUILTIN_REGISTER_BASE_CLASS(TYPENAME_NUMBER);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, SP_METHOD_HAS, 1);

  BUILTIN_FINALIZE_CLASS(TYPENAME_NUMBER);
}

// Built-in number constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, SP_METHOD_CTOR) {
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
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, SP_METHOD_TO_STR) {
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

// Built-in method to check if a value has a property
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("name", TYPENAME_STRING),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Returns " VALUE_STR_TRUE
    " if the " STR(TYPENAME_NUMBER) " class has a method with the given name, otherwise " VALUE_STR_FALSE ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, SP_METHOD_HAS) {
  BUILTIN_CHECK_RECEIVER(NUMBER)
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  // Should align with prop_getter
  ObjString* name = AS_STRING(argv[1]);
  Value discard;
  if (hashtable_get_by_string(&vm.__builtin_Num_class->methods, name, &discard)) {
    return BOOL_VAL(true);
  }

  return BOOL_VAL(false);
}