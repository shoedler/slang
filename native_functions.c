#include <stdio.h>
#include <string.h>
#include <time.h>
#include "builtin.h"
#include "common.h"
#include "hashtable.h"
#include "value.h"
#include "vm.h"

void register_native_functions() {
  define_native(&vm.natives, "clock", native_clock, 0);
  define_native(&vm.natives, "log", native_log, -1);
  define_native(&vm.natives, "typeof", native_typeof, 1);
  define_native(&vm.natives, "cwd", native_cwd, 0);
}

Value native_clock(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  return float_value((double)clock() / CLOCKS_PER_SEC);
}

Value native_cwd(int argc, Value argv[]) {
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

Value native_log(int argc, Value argv[]) {
  // Since argv[0] contains the receiver or function, we start at 1 and run that, even if we have only one
  // arg. Basically, arguments are 1 indexed for native function
  for (int i = 1; i <= argc; i++) {
    // Execute the to_str method on the receiver
    push(argv[i]);  // Load the receiver onto the stack
    ObjString* str = AS_STR(exec_callable(fn_value(argv[i].type->__to_str), 0));
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
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

Value native_typeof(int argc, Value argv[]) {
  UNUSED(argc);
  ObjClass* klass = argv[1].type;
  return class_value(klass);
}
