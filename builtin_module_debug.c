#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

BUILTIN_DECLARE_FN(stack);
BUILTIN_DECLARE_FN(version);

void register_builtin_debug_module() {
  ObjObject* debug_module = make_module(NULL, "Debug");
  define_obj(&vm.modules, "Debug", (Obj*)debug_module);

  BUILTIN_REGISTER_FN(debug_module, stack, 0);
  BUILTIN_REGISTER_FN(debug_module, version, 0);
}

// Native stack function.
BUILTIN_FN_DOC(
    /* Fn Name     */ stack,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Returns a " STR(TYPENAME_SEQ) " of all the values on the vms' stack. The top of the stack is the last element in the " STR(
        TYPENAME_SEQ) ".");
BUILTIN_FN_IMPL(stack) {
  BUILTIN_ARGC_EXACTLY(0)
  UNUSED(argv);

  int stack_size = (int)(vm.stack_top - vm.stack - 1);

  for (int i = 0; i < stack_size; i++) {
    push(vm.stack[i]);
  }

  make_seq(stack_size);
  return pop();
}

// Native version function.
BUILTIN_FN_DOC(
    /* Fn Name     */ version,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STR,
    /* Description */
    "Returns the version of the current running vm.");
BUILTIN_FN_IMPL(version) {
  BUILTIN_ARGC_EXACTLY(0)
  UNUSED(argv);

  return OBJ_VAL(copy_string(SLANG_VERSION, STR_LEN(SLANG_VERSION)));
}