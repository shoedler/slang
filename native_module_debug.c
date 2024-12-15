#include <stddef.h>
#include "common.h"
#include "hashtable.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static Value native_debug_stack(int argc, Value argv[]);
static Value native_debug_version(int argc, Value argv[]);
static Value native_debug_modules(int argc, Value argv[]);
static Value native_debug_heap(int argc, Value argv[]);

#define MODULE_NAME Debug

void native_register_debug_module() {
  ObjObject* debug_module = vm_make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(debug_module));

  define_native(&debug_module->fields, "stack", native_debug_stack, 0);
  define_native(&debug_module->fields, "version", native_debug_version, 0);
  define_native(&debug_module->fields, "modules", native_debug_modules, 0);
  define_native(&debug_module->fields, "heap", native_debug_heap, 0);
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
    vm_push(vm.stack[i]);
  }

  vm_make_seq(stack_size);
  return vm_pop();
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

/**
 * MODULE_NAME.modules() -> TYPENAME_OBJ
 * @brief Returns a copy of the vms' module cache as a TYPENAME_OBJ.
 * Keys are TYPENAME_STRINGs of the module name, values are TYPENAME_MODULEs.
 */
static Value native_debug_modules(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  HashTable copy;
  hashtable_init(&copy);

  hashtable_add_all(&vm.modules, &copy);
  ObjObject* copy_obj = take_object(&copy);

  return obj_value(copy_obj);
}

/**
 * MODULE_NAME.heap() -> nil
 * @brief Prints the heap linked list to the console.
 */
static Value native_debug_heap(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  Obj* object = vm.objects;
  while (object != NULL) {
    switch (object->type) {
      case OBJ_GC_BOUND_METHOD: printf(STR(TYPENAME_BOUND_METHOD)); break;
      case OBJ_GC_CLASS: printf(STR(TYPENAME_CLASS) " %s", ((ObjClass*)object)->name->chars); break;
      case OBJ_GC_CLOSURE: printf(STR(TYPENAME_CLOSURE) " %s", ((ObjClosure*)object)->function->name->chars); break;
      case OBJ_GC_FUNCTION: printf(STR(TYPENAME_FUNCTION) " %s", ((ObjFunction*)object)->name->chars); break;
      case OBJ_GC_NATIVE: printf(STR(TYPENAME_NATIVE) " %s", ((ObjNative*)object)->name->chars); break;
      case OBJ_GC_OBJECT: printf(STR(TYPENAME_OBJ)); break;
      case OBJ_GC_STRING: printf(STR(TYPENAME_STRING) " \"%s\"", ((ObjString*)object)->chars); break;
      case OBJ_GC_UPVALUE: printf(STR(TYPENAME_UPVALUE)); break;
      case OBJ_GC_SEQ: printf(STR(TYPENAME_SEQ)); break;
      case OBJ_GC_TUPLE: printf(STR(TYPENAME_TUPLE)); break;
      default: INTERNAL_ERROR("Unknown object type"); break;
    }
    printf("\n");
    object = object->next;
  }

  return nil_value();
}