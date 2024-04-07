#include <stdlib.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_class_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_CLASS, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, __ctor, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_CLASS, to_str, 0);
}

// Built-in class constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ __ctor,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_CLASS,
    /* Description */
    "Returns " STR(TYPENAME_CLASS) ".");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, __ctor) {
  UNUSED(argc);
  UNUSED(argv);
  runtime_error("Cannot instantiate a class via " STR(TYPENAME_CLASS) " " KEYWORD_CONSTRUCTOR ".");
  return NIL_VAL;
}

// Built-in method to convert a class to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_CLASS,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_CLASS) ".");
BUILTIN_METHOD_IMPL(TYPENAME_CLASS, to_str) {
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