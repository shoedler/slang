#include "builtin.h"
#include "common.h"
#include "vm.h"

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result);
static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value);
static NativeAccessorResult index_getter(Obj* self, Value index, Value* result);
static NativeAccessorResult index_setter(Obj* self, Value index, Value value);

void register_builtin_num_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_NUMBER, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, SP_METHOD_HAS, 1);
  vm.__builtin_Num_class->prop_getter  = prop_getter;
  vm.__builtin_Num_class->prop_setter  = prop_setter;
  vm.__builtin_Num_class->index_getter = index_getter;
  vm.__builtin_Num_class->index_setter = index_setter;
  BUILTIN_FINALIZE_CLASS(TYPENAME_NUMBER);
}

static NativeAccessorResult prop_getter(Obj* self, ObjString* name, Value* result) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(result);
  return ACCESSOR_RESULT_PASS;
}

static NativeAccessorResult prop_setter(Obj* self, ObjString* name, Value value) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(value);
  return ACCESSOR_RESULT_PASS;
}

static NativeAccessorResult index_getter(Obj* self, Value index, Value* result) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(result);
  return ACCESSOR_RESULT_PASS;
}

static NativeAccessorResult index_setter(Obj* self, Value index, Value value) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(value);
  return ACCESSOR_RESULT_PASS;
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
    "<Not supported>");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, SP_METHOD_HAS) {
  BUILTIN_ARGC_EXACTLY(1)
  // Should align with prop_getter
  runtime_error("Type " STR(TYPENAME_NUMBER) " does not support '" STR(SP_METHOD_HAS) "'.");
  return NIL_VAL;
}