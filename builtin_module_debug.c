#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

BUILTIN_DECLARE_FN(stack);
BUILTIN_DECLARE_FN(version);

#define MODULE_NAME Debug

void register_builtin_debug_module() {
  ObjObject* debug_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(debug_module));

  BUILTIN_REGISTER_FN(debug_module, stack, 0);
  BUILTIN_REGISTER_FN(debug_module, version, 0);
}

/**
 * MODULE_NAME.stack() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ of all the values on the vms' stack. The top of the stack is the last element in the
 * TYPENAME_SEQ.
 */
BUILTIN_FN_IMPL(stack) {
  UNUSED(argc);
  UNUSED(argv);

  int stack_size = (int)(vm.stack_top - vm.stack - 1);

  for (int i = 0; i < stack_size; i++) {
    push(vm.stack[i]);
  }

  make_seq(stack_size);
  return pop();
}

/**
 * MODULE_NAME.version() -> TYPENAME_STRING
 * @brief Returns the version of the current running vm.
 */
BUILTIN_FN_IMPL(version) {
  UNUSED(argc);
  UNUSED(argv);

  return str_value(copy_string(SLANG_VERSION, STR_LEN(SLANG_VERSION)));
}