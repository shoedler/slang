#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

static Value native_debug_stack(int argc, Value argv[]);
static Value native_debug_version(int argc, Value argv[]);

#define MODULE_NAME Debug

void register_native_debug_module() {
  ObjObject* debug_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(debug_module));

  define_native(&debug_module->fields, "stack", native_debug_stack, 0);
  define_native(&debug_module->fields, "version", native_debug_version, 0);
}

/**
 * MODULE_NAME.stack() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ of all the values on the vms' stack. The top of the stack is the last element in the
 * TYPENAME_SEQ.
 */
static Value native_debug_stack(int argc, Value argv[]) {
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
static Value native_debug_version(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  return str_value(copy_string(SLANG_VERSION, STR_LEN(SLANG_VERSION)));
}