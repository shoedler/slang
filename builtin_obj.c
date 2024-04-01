#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_obj_class() {
  // Create the object class
  vm.__builtin_Obj_class = new_class(copy_string(STR(TYPENAME_OBJ), sizeof(STR(TYPENAME_OBJ)) - 1), NULL);

  // Create the builtin obj instance
  vm.builtin = new_instance(vm.__builtin_Obj_class);

  define_obj(&vm.builtin->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);
  // TODO: Remove this comment. Builtin functions were registered here.
  define_obj(&vm.builtin->fields, STR(TYPENAME_OBJ), (Obj*)vm.__builtin_Obj_class);

  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, to_str)
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, hash)
}

// Built-in method to return the hash of an object.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ hash,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the hash of the " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, hash) {
  BUILTIN_CHECK_ARGC_ZERO()

  return NUMBER_VAL(hash_value(argv[0]));
}

// Built-in method to convert an object to a string. This one is special in that is is the toplevel to_str -
// there's no abstraction over it. This is why we have some special cases here for our internal types.
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_OBJ,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of the " STR(TYPENAME_OBJ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_OBJ, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()

  if (IS_OBJ(argv[0])) {
    switch (AS_OBJ(argv[0])->type) {
      case OBJ_NATIVE: return OBJ_VAL(copy_string(VALUE_STR_NATIVE, sizeof(VALUE_STR_NATIVE) - 1));
      case OBJ_CLOSURE:
        return BUILTIN_METHOD(TYPENAME_OBJ, to_str)(argc, &OBJ_VAL(AS_CLOSURE(argv[0])->function));
      case OBJ_BOUND_METHOD:
        return BUILTIN_METHOD(TYPENAME_OBJ, to_str)(argc, &OBJ_VAL(AS_BOUND_METHOD(argv[0])->method));
      case OBJ_CLASS: {
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
      case OBJ_INSTANCE: {
        ObjInstance* instance = AS_INSTANCE(argv[0]);
        ObjString* name       = instance->klass->name;
        if (name == NULL || name->chars == NULL) {
          name = copy_string("???", 3);
        }
        push(OBJ_VAL(name));

        size_t buf_size = VALUE_STRFTM_INSTANCE_LEN + name->length;
        char* chars     = malloc(buf_size);
        snprintf(chars, buf_size, VALUE_STRFTM_INSTANCE, name->chars);
        // Intuitively, you'd expect to use take_string here, but we don't know where malloc
        // allocates the memory - we don't want this block in our own memory pool.
        ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

        free(chars);
        pop();  // Name str
        return OBJ_VAL(str_obj);
      }
      case OBJ_FUNCTION: {
        ObjFunction* function = AS_FUNCTION(argv[0]);
        ObjString* name       = function->name;
        if (name == NULL || name->chars == NULL) {
          name = copy_string("???", 3);
        }
        push(OBJ_VAL(name));  // GC Protection

        size_t buf_size = VALUE_STRFMT_FUNCTION_LEN + name->length;
        char* chars     = malloc(buf_size);
        snprintf(chars, buf_size, VALUE_STRFMT_FUNCTION, name->chars);

        // Intuitively, you'd expect to use take_string here, but we don't know where malloc
        // allocates the memory - we don't want this block in our own memory pool.
        ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

        free(chars);
        pop();  // Name str
        return OBJ_VAL(str_obj);
      }
    }
  }

  // This here is the catch-all for all values. We print the type-name and memory address of the value.
  const char* t_name = type_name(argv[0]);

  // Print the memory address of the object using (void*)AS_OBJ(argv[0]).
  // We need to know the size of the buffer to allocate, so we calculate it first.
  size_t adr_str_len = snprintf(NULL, 0, "%p", (void*)AS_OBJ(argv[0]));

  size_t buf_size = VALUE_STRFMT_OBJ_LEN + strlen(t_name) + adr_str_len;
  char* chars     = malloc(buf_size);
  snprintf(chars, buf_size, VALUE_STRFMT_OBJ, t_name, (void*)AS_OBJ(argv[0]));
  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size - 1);

  free(chars);
  return OBJ_VAL(str_obj);
}