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

/**
 * clock() -> TYPENAME_FLOAT
 * @brief Returns the current execution time in seconds.
 */
BUILTIN_FN_IMPL(clock) {
  UNUSED(argc);
  UNUSED(argv);

  return float_value((double)clock() / CLOCKS_PER_SEC);
}

/**
 * cwd() -> TYPENAME_STRING
 * @brief Returns the current working directory or TYPENAME_NIL if no module is active.
 */
BUILTIN_FN_IMPL(cwd) {
  UNUSED(argc);
  UNUSED(argv);

  if (vm.module == NULL) {
    return nil_value();
  }

  Value cwd;
  if (!hashtable_get_by_string(&vm.module->fields, vm.special_prop_names[SPECIAL_PROP_FILE_PATH], &cwd)) {
    return nil_value();
  }

  return cwd;
}

/**
 * log(arg1: TYPENAME_VALUE, ...args: TYPENAME_VALUE) -> TYPENAME_NIL
 * @brief Prints the arguments to the console.
 */
BUILTIN_FN_IMPL(log) {
  // Since argv[0] contains the receiver or function, we start at 1 and run that, even if we have only one
  // arg. Basically, arguments are 1 indexed for native function
  for (int i = 1; i <= argc; i++) {
    // Execute the to_str method on the receiver
    push(argv[i]);  // Load the receiver onto the stack
    ObjString* str = AS_STRING(exec_callable(fn_value(argv[i].type->__to_str), 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return nil_value();
    }

    printf("%s", str->chars);
    if (i <= argc - 1) {
      printf(" ");
    }
  }
  printf("\n");
  return nil_value();
}

/**
 * typeof(value: TYPENAME_VALUE) -> TYPENAME_CLASS
 * @brief Returns the class of the value.
 */
BUILTIN_FN_IMPL(typeof) {
  UNUSED(argc);

  ObjClass* klass = argv[1].type;
  return class_value(klass);
}