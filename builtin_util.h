#ifndef builtin_util_h
#define builtin_util_h

//
// Macros for built-in function documentation. Also for declaring and implementing built-in functions.
//

#define ___BUILTIN_DOC_SIG_START(name) #name "("
#define ___BUILTIN_DOC_SIG_END(return_type, description) ") -> " STR(return_type) "\n" description "\n"

#define ___BUILTIN(a, b) __builtin_##a##_##b
#define ___BUILTIN_CLASS(class_name) ___BUILTIN(class_name, class)
#define ___BUILTIN_METHOD(class_name, method_name) ___BUILTIN(class_name, method_name)
#define ___BUILTIN_METHOD_DOC_STR(class_name, method_name) ___BUILTIN(class_name, method_name##_doc)
#define ___BUILTIN_FN(name) ___BUILTIN(fn, name)
#define ___BUILTIN_FN_DOC_STR(name) ___BUILTIN(fn, name##_doc)

#define DOC_ARG(name, type) name ": " STR(type)
#define DOC_ARG_SEP ", "
#define DOC_ARG_REST "..."

// Naming convention for a built-in class.
#define BUILTIN_CLASS(class_name) ___BUILTIN_CLASS(class_name)
// Naming convention for a built-in method.
#define BUILTIN_METHOD(class_name, method_name) ___BUILTIN_METHOD(class_name, method_name)
// Naming convention for a built-in method documentation string.
#define BUILTIN_METHOD_DOC_STR(class_name, method_name) ___BUILTIN_METHOD_DOC_STR(class_name, method_name)
// Naming convention for a built-in function.
#define BUILTIN_FN(name) ___BUILTIN_FN(name)
// Naming convention for a built-in function documentation string.
#define BUILTIN_FN_DOC_STR(name) ___BUILTIN_FN_DOC_STR(name)

// Declare a built-in method and its documentation string.
#define BUILTIN_DECLARE_METHOD(class_name, method_name)               \
  static const char* BUILTIN_METHOD_DOC_STR(class_name, method_name); \
  Value BUILTIN_METHOD(class_name, method_name)(int argc, Value argv[]);
// Declare a built-in function and its documentation string.
#define BUILTIN_DECLARE_FN(name)               \
  static const char* BUILTIN_FN_DOC_STR(name); \
  Value BUILTIN_FN(name)(int argc, Value argv[]);

// Register a built-in class in the VM, by adding it to the built-in object's field table.
#define BUILTIN_REGISTER_CLASS(class_name, base_class)                                                   \
  vm.##BUILTIN_CLASS(class_name) =                                                                       \
      new_class(copy_string(STR(class_name), STR_LEN(STR(class_name))), vm.##BUILTIN_CLASS(base_class)); \
  define_obj(&vm.builtin->fields, STR(class_name), (Obj*)vm.##BUILTIN_CLASS(class_name));
// Register a built-in base class in the VM, by adding it to the built-in object's field table.
#define BUILTIN_REGISTER_BASE_CLASS(class_name)                                                             \
  vm.##BUILTIN_CLASS(class_name) = new_class(copy_string(STR(class_name), STR_LEN(STR(class_name))), NULL); \
  define_obj(&vm.builtin->fields, STR(class_name), (Obj*)vm.##BUILTIN_CLASS(class_name));
// Register a built-in method in the VM, by adding it to the built-in class's method table.
#define BUILTIN_REGISTER_METHOD(class_name, method_name, arity)                                                      \
  define_native(&vm.##BUILTIN_CLASS(class_name)->methods, STR(method_name), BUILTIN_METHOD(class_name, method_name), \
                BUILTIN_METHOD_DOC_STR(class_name, method_name), arity);
// Register a built-in function in the VM, by adding it to the built-in object's field table.
#define BUILTIN_REGISTER_FN(instance, name, arity) \
  define_native(&instance->fields, #name, BUILTIN_FN(name), BUILTIN_FN_DOC_STR(name), arity);
// Finalize a built-in class by populating its special fields.
#define BUILTIN_FINALIZE_CLASS(class_name) finalize_new_class(vm.##BUILTIN_CLASS(class_name))

// Define a built-in method's documentation string.
#define BUILTIN_METHOD_DOC(class_name, method_name, args, return_type, description) \
  static const char* BUILTIN_METHOD_DOC_STR(class_name, method_name) =              \
      STR(class_name) "." ___BUILTIN_DOC_SIG_START(method_name) args ___BUILTIN_DOC_SIG_END(return_type, description)

// Define a built-in method's documentation overload.
#define BUILTIN_METHOD_DOC_OVERLOAD(class_name, method_name, args, return_type, description) \
  "\n" STR(class_name) "." ___BUILTIN_DOC_SIG_START(method_name) args ___BUILTIN_DOC_SIG_END(return_type, description)

// Define a built-in method.
#define BUILTIN_METHOD_IMPL(class_name, method_name) Value BUILTIN_METHOD(class_name, method_name)##(int argc, Value argv[])

// Define a built-in function's documentation string.
#define BUILTIN_FN_DOC(name, args, return_type, description) \
  static const char* BUILTIN_FN_DOC_STR(name) =              \
      "fn " ___BUILTIN_DOC_SIG_START(name) args ___BUILTIN_DOC_SIG_END(return_type, description)

// Define a built-in function.
#define BUILTIN_FN_IMPL(name) Value BUILTIN_FN(name)(int argc, Value argv[])

//
// Macros for argument checking in built-in functions.
//
#define BUILTIN_ARGC_AT_LEAST(min)                                          \
  if (argc < min) {                                                         \
    runtime_error("Expected at least %d arguments but got %d.", min, argc); \
    return NIL_VAL;                                                         \
  }

#define BUILTIN_ARGC_EXACTLY(count)                                  \
  if (argc != count) {                                               \
    runtime_error("Expected %d arguments but got %d.", count, argc); \
    return NIL_VAL;                                                  \
  }

#define BUILTIN_CHECK_RECEIVER(type)                                                                                \
  if (!IS_##type(argv[0])) {                                                                                        \
    runtime_error("Expected receiver of type "##STR(TYPENAME_##type) " but got %s.", typeof(argv[0])->name->chars); \
    return NIL_VAL;                                                                                                 \
  }

#define BUILTIN_CHECK_ARG_AT(index, type)                                                          \
  if (!IS_##type(argv[index])) {                                                                   \
    runtime_error("Expected argument %d of type "##STR(TYPENAME_##type) " but got %s.", index - 1, \
                  typeof(argv[index])->name->chars);                                               \
    return NIL_VAL;                                                                                \
  }

#define BUILTIN_CHECK_ARG_AT_IS_CALLABLE(index)                                                                    \
  if (!IS_CALLABLE(argv[index])) {                                                                                 \
    runtime_error("Expected argument %d to be callable but got %s.", index - 1, typeof(argv[index])->name->chars); \
    return NIL_VAL;                                                                                                \
  }

#endif