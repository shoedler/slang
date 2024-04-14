#include <string.h>
#include <time.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_functions() {
  BUILTIN_REGISTER_FN(vm.builtin, clock, 0);
  BUILTIN_REGISTER_FN(vm.builtin, log, -1);
  BUILTIN_REGISTER_FN(vm.builtin, typeof, 1);
  BUILTIN_REGISTER_FN(vm.builtin, cwd, 0);
}

// Native clock function.
BUILTIN_FN_DOC(
    /* Fn Name     */ clock,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the current execution time in seconds.");
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
  if (!hashtable_get_by_string(&vm.module->fields, vm.special_field_names[SPECIAL_PROP_FILE_PATH], &cwd)) {
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
    // Execute the to_str method on the receiver
    push(argv[i]);  // Load the receiver onto the stack
    ObjString* str = AS_STRING(exec_fn((Obj*)vm.special_field_names[SPECIAL_METHOD_TO_STR], 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

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
    /* Fn Name     */ typeof,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_CLASS,
    /* Description */ "Returns the class of the value.");
BUILTIN_FN_IMPL(typeof) {
  UNUSED(argc);

  ObjClass* klass = typeof(argv[1]);
  return OBJ_VAL(klass);
}