#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_class_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_CLASS, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, SP_METHOD_HAS, 1);

  BUILTIN_FINALIZE_CLASS(TYPENAME_CLASS);
}

// Built-in class constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_CLASS,
    /* Description */
    "No-op constructor for " STR(TYPENAME_CLASS) ".");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, SP_METHOD_CTOR) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a class via " STR(TYPENAME_CLASS) "." STR(SP_METHOD_CTOR) ".");
  return NIL_VAL;
}

// Built-in method to convert a class to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_CLASS) ".");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(CLASS)

  ObjClass* klass = AS_CLASS(argv[0]);
  ObjString* name = klass->name;
  if (name == NULL || name->chars == NULL) {
    name = copy_string("???", 3);
  }
  push(OBJ_VAL(name));

  size_t buf_size = VALUE_STRFMT_CLASS_LEN + name->length;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFMT_CLASS, name->chars);
  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  pop();  // Name str
  return OBJ_VAL(str_obj);
}

// Built-in method to check if a class has a method
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("name", TYPENAME_STRING),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " STR(TYPENAME_TRUE) " if the class has a property with the given name, " STR(TYPENAME_FALSE) " otherwise.");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, SP_METHOD_HAS) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(CLASS)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* name = AS_STRING(argv[1]);
  ObjClass* klass = AS_CLASS(argv[0]);

  // Execute the value_get_property function to see if the class has the thing. We use this approach to make sure the two are
  // aligned and return the same result.
  push(OBJ_VAL(klass));
  if (value_get_property(name)) {
    pop();  // The result
    return BOOL_VAL(true);
  }
  if (vm.flags & VM_FLAG_HAS_ERROR) {
    return NIL_VAL;
  }

  pop();  // The class

  return BOOL_VAL(false);
}