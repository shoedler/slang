#include <string.h>
#include <time.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_functions() {
  BUILTIN_REGISTER_FN(clock, 0);
  BUILTIN_REGISTER_FN(log, -1);
  BUILTIN_REGISTER_FN(type_of, 1);
  BUILTIN_REGISTER_FN(type_name, 1);
  BUILTIN_REGISTER_FN(cwd, 0);
}

// Native clock function.
BUILTIN_FN_DOC(
    /* Fn Name     */ clock,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the current execution time in miliseconds.");
BUILTIN_FN_IMPL(clock) {
  UNUSED(argc);
  UNUSED(argv);

  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

// Native cwd function.
BUILTIN_FN_DOC(
    /* Fn Name     */ cwd,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Returns the current working directory or " STR(TYPENAME_NIL) " if no module is active.");
BUILTIN_FN_IMPL(cwd) {
  UNUSED(argc);
  UNUSED(argv);

  if (vm.module == NULL) {
    return NIL_VAL;
  }

  Value cwd;
  if (!hashtable_get(&vm.module->fields, vm.cached_words[WORD_FILE_PATH], &cwd)) {
    return NIL_VAL;
  }

  return cwd;
}

// Native print function.
BUILTIN_FN_DOC(
    /* Fn Name     */ log,
    /* Arguments   */ DOC_ARG("arg1", TYPENAME_OBJ) DOC_ARG_SEP DOC_ARG_REST,
    /* Return Type */ TYPENAME_NIL,
    /* Description */ "Prints the arguments to the console.");
BUILTIN_FN_IMPL(log) {
  // Since argv[0] contains the receiver or function, we start at 1 and run that, even if we have only one
  // arg. Basically, arguments are 1 indexed for native function
  for (int i = 1; i <= argc; i++) {
    push(argv[i]);  // Load the receiver onto the stack
    ObjString* str = AS_STRING(exec_method(copy_string("to_str", 6), 0));
    printf("%s", str->chars);
    if (i <= argc - 1) {
      printf(" ");
    }
  }
  printf("\n");
  return NIL_VAL;
}

// Native type of.
BUILTIN_FN_DOC(
    /* Fn Name     */ type_of,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_CLASS,
    /* Description */ "Returns the class of the value.");
BUILTIN_FN_IMPL(type_of) {
  UNUSED(argc);

  ObjClass* klass = type_of(argv[1]);
  return OBJ_VAL(klass);
}

// Native type name.
BUILTIN_FN_DOC(
    /* Fn Name     */ type_name,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns the name of the value's type.");
BUILTIN_FN_IMPL(type_name) {
  UNUSED(argc);

  const char* t_name = type_name(argv[1]);
  ObjString* str_obj = copy_string(t_name, (int)strlen(t_name));
  return OBJ_VAL(str_obj);
}