#ifndef builtin_util_h
#define builtin_util_h

//
// Macros for built-in function documentation. Also for declaring and implementing built-in functions.
//

#define ___BUILTIN(a, b) __builtin_##a##_##b
#define ___BUILTIN_CLASS(class_name) ___BUILTIN(class_name, class)
#define ___BUILTIN_METHOD(class_name, method_name) ___BUILTIN(class_name, method_name)
#define ___BUILTIN_FN(name) ___BUILTIN(fn, name)

// Naming convention for a built-in class.
#define BUILTIN_CLASS(class_name) ___BUILTIN_CLASS(class_name)
// Naming convention for a built-in method.
#define BUILTIN_METHOD(class_name, method_name) ___BUILTIN_METHOD(class_name, method_name)
// Naming convention for a built-in function.
#define BUILTIN_FN(name) ___BUILTIN_FN(name)

// Declare a built-in method and its documentation string.
#define BUILTIN_DECLARE_METHOD(class_name, method_name) \
  extern Value BUILTIN_METHOD(class_name, method_name)(int argc, Value argv[]);
// Declare a built-in function and its documentation string.
#define BUILTIN_DECLARE_FN(name) extern Value BUILTIN_FN(name)(int argc, Value argv[]);

// Register a built-in method in the VM, by adding it to the built-in class's method table.
#define BUILTIN_REGISTER_METHOD(class_name, method_name, arity) \
  define_native(&vm.BUILTIN_CLASS(class_name)->methods, STR(method_name), BUILTIN_METHOD(class_name, method_name), arity);

// Register a built-in function in the VM, by adding it to the built-in object's field table.
#define BUILTIN_REGISTER_FN(instance, name, arity) define_native(&instance->fields, #name, BUILTIN_FN(name), arity);

// Finalize a built-in class by populating its special fields.
#define BUILTIN_FINALIZE_CLASS(class_name) finalize_new_class(vm.BUILTIN_CLASS(class_name))

// Define a built-in method.
#define BUILTIN_METHOD_IMPL(class_name, method_name) Value BUILTIN_METHOD(class_name, method_name)(int argc, Value argv[])

// Define a built-in function.
#define BUILTIN_FN_IMPL(name) Value BUILTIN_FN(name)(int argc, Value argv[])

//
// Macros for argument checking in built-in functions.
//
#define BUILTIN_CHECK_RECEIVER(uc_type)                                                                              \
  if (!IS_##uc_type(argv[0])) {                                                                                      \
    runtime_error("Expected receiver of type " STR(TYPENAME_##uc_type) " but got %s.", (argv[0]).type->name->chars); \
    return nil_value();                                                                                              \
  }

#define BUILTIN_CHECK_ARG_AT(index, uc_type)                                                         \
  if (!IS_##uc_type(argv[index])) {                                                                  \
    runtime_error("Expected argument %d of type " STR(TYPENAME_##uc_type) " but got %s.", index - 1, \
                  (argv[index]).type->name->chars);                                                  \
    return nil_value();                                                                              \
  }

#define BUILTIN_CHECK_ARG_AT_IS_CALLABLE(index)                                                                 \
  if (!IS_CALLABLE(argv[index])) {                                                                              \
    runtime_error("Expected argument %d to be callable but got %s.", index - 1, argv[index].type->name->chars); \
    return nil_value();                                                                                         \
  }

#endif