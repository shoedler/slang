#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "builtin.h"
#include "compiler.h"
#include "file.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

#if defined(DEBUG_LOG_GC) || defined(DEBUG_LOG_GC_ALLOCATIONS) || defined(DEBUG_TRACE_EXECUTION)
#include "debug.h"
#endif

Vm vm;

// A call to a function or method must be handled differently depending on whether it's a:
// - Native function: Return a value "immediately".
// - Managed-code functions just update the frame and then the vm will execute the new frame.
//   meaning it will return a value "later".
// We need to differentiate between these two cases when calling a function internally.
typedef enum {
  CALL_FAILED,
  CALL_RETURNED,  // The call returned immediately
  CALL_RUNNING,   // The call is running and we need to execute the new frame
} CallResult;

static Value exit_with_runtime_error();
static Value run();
static Value peek(int distance);

static ObjString* fast_to_str(Value value);
static ObjClass* type_of(Value value);
static bool is_falsey(Value value);

BUILTIN_DECLARE_FN(clock);
BUILTIN_DECLARE_FN(log);
BUILTIN_DECLARE_FN(type_of);
BUILTIN_DECLARE_FN(type_name);
BUILTIN_DECLARE_FN(cwd);

BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, hash)

BUILTIN_DECLARE_METHOD(TYPENAME_BOOL, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_BOOL, to_str)

BUILTIN_DECLARE_METHOD(TYPENAME_NIL, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_NIL, to_str)

BUILTIN_DECLARE_METHOD(TYPENAME_NUMBER, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_NUMBER, to_str)

BUILTIN_DECLARE_METHOD(TYPENAME_STRING, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, len)

BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, len)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, push)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, pop)

BUILTIN_DECLARE_METHOD(TYPENAME_MAP, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, len)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, entries)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, values)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, keys)

static void reset_stack() {
  vm.pause_gc      = 0;
  vm.stack_top     = vm.stack;
  vm.frame_count   = 0;
  vm.open_upvalues = NULL;
}

// Prints a runtime error message including the stacktrace
static void runtime_error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  // Stacktrace
  for (int i = vm.frame_count - 1; i >= 0; i--) {
    CallFrame* frame      = &vm.frames[i];
    ObjFunction* function = frame->closure->function;
    size_t instruction    = frame->ip - function->chunk.code - 1;

    fprintf(stderr, "  at line %d ", function->chunk.lines[instruction]);

    Value module_name;
    if (!hashtable_get(&function->globals_context->fields, vm.cached_words[WORD_MODULE_NAME], &module_name)) {
      fprintf(stderr, "in \"%s\"\n", function->name->chars);
      break;
    }

    // If the module_name is the same ref as the current frame's function-name, then we know it's the
    // toplevel. because that's how we initialize it in the compiler.
    if (AS_STRING(module_name) == function->name) {
      fprintf(stderr, "at the toplevel of module \"%s\"\n", AS_CSTRING(module_name));
    } else {
      fprintf(stderr, "in \"%s\" in module \"%s\"\n", function->name->chars, AS_CSTRING(module_name));
    }
  }

  reset_stack();
}

// This is just a placeholder to find where we actually throw rt errors.
// I want another way to do this, but I don't know how yet.
static Value exit_with_runtime_error() {
  free_vm();
  // TODO (recovery): Find a way to progpagate errors */
  exit(70);
  return NIL_VAL;
}

// This is just a plcaeholder to find where we actually throw compile errors.
// I want another way to do this, but I don't know how yet.
static void exit_with_compile_error() {
  free_vm();
  // TODO (recovery): Find a way to progpagate errors */
  exit(65);
}

// Defines a native function in the given table.
static void define_native(HashTable* table, const char* name, NativeFn function, const char* doc) {
  Value key          = OBJ_VAL(copy_string(name, (int)strlen(name)));
  ObjString* doc_str = copy_string(doc, (int)strlen(doc));
  Value value        = OBJ_VAL(new_native(function, doc_str));
  push(key);
  push(value);
  push(OBJ_VAL(doc_str));
  hashtable_set(table, key, value);
  pop();
  pop();
  pop();
}

// Defines an object in the given table.
static void define_obj(HashTable* table, const char* name, Obj* obj) {
  // 🐛 Seemingly out of nowhere the pushed key and value get swapped on the stack...?! I don't know why.
  // Funnily enough, it only happens when we start a nested module. E.g. we're in "main" and import "std",
  // when the builtins get attached to the module instances field within the start_module function key and
  // value are swapped! Same goes for the name (WTF). I've found this out because in the stack trace we now
  // see the modules name and for nested modules it always printed __name. The current remedy is to just use
  // variables for "key" and "value" and just use these instead of pushing and peeking.
  Value key   = OBJ_VAL(copy_string(name, (int)strlen(name)));
  Value value = OBJ_VAL(obj);
  push(key);
  push(value);
  hashtable_set(table, key, value);
  pop();
  pop();
}

// Creates a sequence of length "count" from the top "count" values on the stack.
// The resulting sequence is pushed onto the stack.
static void make_seq(int count) {
  // Since we know the count, we can preallocate the value array for the list. This avoids
  // using write_value_array within the loop, which can trigger a GC due to growing the array
  // and free items in the middle of the loop. Also, it lets us pop the list items on the
  // stack, instead of peeking and then having to pop them later (Requiring us to loop over
  // the array twice)
  ValueArray items;
  init_value_array(&items);

  items.values   = RESIZE_ARRAY(Value, items.values, 0, count);
  items.capacity = count;
  items.count    = count;

  // Take the values array while before we start popping the stack, so the items are still seen by the GC as
  // we allocate the new seq object.
  ObjSeq* seq = take_seq(&items);

  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = pop();
  }

  push(OBJ_VAL(seq));
}

// Creates a map from the top "count" * 2 values on the stack.
// The resulting map is pushed onto the stack.
static void make_map(int count) {
  // Since we know the count, we can preallocate the hashtable for the map. This allows
  // using hashtable_set within the loop. We don't have to worry about it wanting to resize the hashtable,
  // which can trigger a GC and free items in the middle of the loop, because it already has enough capacity.
  // Also, it lets us pop the map items on the stack, instead of peeking and then having to pop them later
  // (Requiring us to loop over the keys and values twice)
  HashTable entries;
  init_hashtable(&entries);
  hashtable_preallocate(&entries, count);

  // Take the hashtable while before we start popping the stack, so the entries are still seen by the GC as
  // we allocate the new map object.
  ObjMap* map = take_map(&entries);

  // Use pop
  for (int i = 0; i < count; i++) {
    Value value = pop();
    Value key   = pop();

    hashtable_set(&map->entries, key, value);
  }

  push(OBJ_VAL(map));
}

void init_vm() {
  reset_stack();
  vm.objects         = NULL;
  vm.module          = NULL;  // No active module
  vm.bytes_allocated = 0;
  vm.next_gc         = GC_DEFAULT_THRESHOLD;
  vm.gray_count      = 0;
  vm.gray_capacity   = 0;
  vm.gray_stack      = NULL;
  vm.pause_gc        = 1;  // Pause while we initialize the vm.

  init_hashtable(&vm.strings);
  init_hashtable(&vm.modules);

  // Build the reserved words lookup table
  memset(vm.cached_words, 0, sizeof(vm.cached_words));
  vm.cached_words[WORD_CTOR]        = OBJ_VAL(copy_string(KEYWORD_CONSTRUCTOR, KEYWORD_CONSTRUCTOR_LEN));
  vm.cached_words[WORD_NAME]        = OBJ_VAL(copy_string(KEYWORD_NAME, KEYWORD_NAME_LEN));
  vm.cached_words[WORD_MODULE_NAME] = OBJ_VAL(copy_string(KEYWORD_MODULE_NAME, KEYWORD_MODULE_NAME_LEN));
  vm.cached_words[WORD_FILE_PATH]   = OBJ_VAL(copy_string(KEYWORD_FILE_PATH, KEYWORD_FILE_PATH_LEN));

  // Create the object class
  vm.__builtin_Obj_class = new_class(copy_string(STR(TYPENAME_OBJ), sizeof(STR(TYPENAME_OBJ)) - 1), NULL);

  // Create the builtin obj instance
  vm.builtin = new_instance(vm.__builtin_Obj_class);
  define_obj(&vm.builtin->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);

  // Create the builtin functions
  BUILTIN_REGISTER_FN(clock);
  BUILTIN_REGISTER_FN(log);
  BUILTIN_REGISTER_FN(type_of);
  BUILTIN_REGISTER_FN(type_name);
  BUILTIN_REGISTER_FN(cwd);

  define_obj(&vm.builtin->fields, STR(TYPENAME_OBJ), (Obj*)vm.__builtin_Obj_class);

  // ... and define the object methods
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, to_str)
  BUILTIN_REGISTER_METHOD(TYPENAME_OBJ, hash)

  // Create the string class
  BUILTIN_REGISTER_CLASS(TYPENAME_STRING, TYPENAME_OBJ)
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, __ctor)
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, to_str)
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, len)

  // Create the boolean class
  BUILTIN_REGISTER_CLASS(TYPENAME_BOOL, TYPENAME_OBJ)
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, __ctor)
  BUILTIN_REGISTER_METHOD(TYPENAME_BOOL, to_str)

  // Create the nil class
  BUILTIN_REGISTER_CLASS(TYPENAME_NIL, TYPENAME_OBJ)
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, __ctor)
  BUILTIN_REGISTER_METHOD(TYPENAME_NIL, to_str)

  // Create the number class
  BUILTIN_REGISTER_CLASS(TYPENAME_NUMBER, TYPENAME_OBJ)
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, __ctor)
  BUILTIN_REGISTER_METHOD(TYPENAME_NUMBER, to_str)

  // Create the sequence class
  BUILTIN_REGISTER_CLASS(TYPENAME_SEQ, TYPENAME_OBJ)
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, __ctor)
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, to_str)
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, len)
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, push)
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, pop)

  // Create the map class
  BUILTIN_REGISTER_CLASS(TYPENAME_MAP, TYPENAME_OBJ)
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, __ctor)
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, to_str)
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, len)
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, entries)
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, values)
  BUILTIN_REGISTER_METHOD(TYPENAME_MAP, keys)

  // Create the module class
  BUILTIN_REGISTER_CLASS(TYPENAME_MODULE, TYPENAME_OBJ)

  vm.pause_gc = 0;
  reset_stack();
}

void free_vm() {
  free_hashtable(&vm.strings);
  free_hashtable(&vm.modules);
  memset(vm.cached_words, 0, sizeof(vm.cached_words));
  free_objects();
}

void push(Value value) {
  *vm.stack_top = value;
  vm.stack_top++;
}

Value pop() {
  vm.stack_top--;
  return *vm.stack_top;
}

// Look at the value without popping it
static Value peek(int distance) {
  return vm.stack_top[-1 - distance];
}

// Retrieves the class of a value. Everything in Slang is an 'object', so this function will always return
// a class.
static ObjClass* type_of(Value value) {
  // Handle primitive and internal types which have a corresponding class
  if (!IS_INSTANCE(value)) {
    switch (value.type) {
      case VAL_NIL: return vm.__builtin_Nil_class;     // This field name was created via macro
      case VAL_BOOL: return vm.__builtin_Bool_class;   // This field name was created via macro
      case VAL_NUMBER: return vm.__builtin_Num_class;  // This field name was created via macro
      case VAL_OBJ: {
        switch (OBJ_TYPE(value)) {
          case OBJ_STRING: return vm.__builtin_Str_class;  // This field name was created via macro
          case OBJ_SEQ: return vm.__builtin_Seq_class;     // This field name was created via macro
          case OBJ_MAP: return vm.__builtin_Map_class;     // This field name was created via macro
        }
      }
    }
    return vm.__builtin_Obj_class;
  }

  // Instances are easy, just return the class. Most of them are probably managed-code classes.
  return AS_INSTANCE(value)->klass;
}

// Finds a method in the inheritance chain of a class. This is used to find methods in the class hierarchy.
// If the method is not found, NIL_VAL is returned.
static Value find_method_in_inheritance_chain(ObjClass* klass, ObjString* name) {
  while (klass != NULL) {
    Value method;
    if (hashtable_get(&klass->methods, OBJ_VAL(name), &method)) {
      return method;
    }
    klass = klass->base;
  }
  return NIL_VAL;
}

// Executes a call to a managed-code function or method by creating a new call frame and pushing it onto the
// frame stack.
// `Stack: ...[closure][arg0][arg1]...[argN]`
static CallResult call_managed(ObjClosure* closure, int arg_count) {
  if (arg_count != closure->function->arity) {
    runtime_error("Expected %d arguments but got %d.", closure->function->arity, arg_count);
    return CALL_FAILED;
  }

  if (vm.frame_count == FRAMES_MAX) {
    runtime_error("Stack overflow.");
    return CALL_FAILED;
  }

  CallFrame* frame = &vm.frames[vm.frame_count++];
  frame->closure   = closure;
  frame->ip        = closure->function->chunk.code;
  frame->slots     = vm.stack_top - arg_count -
                 1;  // -1 to account for either the function or the receiver preceeding the arguments.
  frame->globals = &closure->function->globals_context->fields;

  return CALL_RUNNING;
}

// Calls a native function with the given number of arguments (on the stack).
// `Stack: ...[native|receiver][arg0][arg1]...[argN]`
static CallResult call_native(NativeFn native, int arg_count) {
  Value* args  = vm.stack_top - arg_count - 1;
  Value result = native(arg_count, args);
  vm.stack_top -= arg_count + 1;  // Remove args + fn or receiver
  push(result);

  return CALL_RETURNED;
}

// Calls a callable managed or native value (function, method, class (__ctor), etc.) with the given number of
// arguments on the stack.
// `Stack: ...[callable][arg0][arg1]...[argN]`.
// Not used for invocation of methods (no receiver). But it can handle bound methods and constructors - in
// which case it will put the appropriate receiver on the stack.
static CallResult call_value(Value callable, int arg_count) {
  if (IS_OBJ(callable)) {
    switch (OBJ_TYPE(callable)) {
      case OBJ_BOUND_METHOD: {
        ObjBoundMethod* bound = AS_BOUND_METHOD(callable);
        // Load the receiver onto the stack, just before the arguments, overriding the bound method on the
        // stack.
        vm.stack_top[-arg_count - 1] = bound->receiver;

        switch (bound->method->type) {
          case OBJ_CLOSURE: return call_managed((ObjClosure*)bound->method, arg_count);
          case OBJ_NATIVE: return call_native(((ObjNative*)bound->method)->function, arg_count);
          default: break;  // Non-callable object type.
        }
      }
      case OBJ_CLASS: {
        ObjClass* klass = AS_CLASS(callable);

        // Construct a new instance of the class.
        // We just replace the class on the stack (callable) with an instance of it.
        // This also happens for primitive types. In their constructors, we replace this fresh instance with
        // an actual primitive value.
        vm.stack_top[-arg_count - 1] = OBJ_VAL(new_instance(klass));

        // Check if the class has a constructor. 'ctor' is actually a stupid name, because it doesn't
        // construct anything. As you can see, the instance already exists. It's actually more like an 'init'
        // method. It's perfectly valid to have no ctor - you'll also end up with a valid instance on the
        // stack.
        Value ctor;
        if (hashtable_get(&klass->methods, vm.cached_words[WORD_CTOR], &ctor)) {
          switch (AS_OBJ(ctor)->type) {
            case OBJ_CLOSURE: return call_managed(AS_CLOSURE(ctor), arg_count);
            case OBJ_NATIVE: return call_native(AS_NATIVE(ctor), arg_count);
            default: {
              runtime_error("Cannot invoke ctor of type '%s'", type_name(ctor));
              return CALL_FAILED;
            }
          }
        } else if (arg_count != 0) {
          runtime_error("Expected 0 arguments but got %d.", arg_count);
          return CALL_FAILED;
        }
        return CALL_RETURNED;
      }
      case OBJ_CLOSURE: return call_managed(AS_CLOSURE(callable), arg_count);
      case OBJ_NATIVE: return call_native(AS_NATIVE(callable), arg_count);
      default: break;  // Non-callable object type.
    }
  }

  runtime_error("Attempted to call non-callable value of type %s.", type_name(callable));
  return CALL_FAILED;
}

// Invokes a method on a class by looking up the method in the class' method
// table and calling it. (Combines "get-property" and "call" opcodes)
// `Stack: ...[receiver][arg0][arg1]...[argN]`
static CallResult invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) {
  Value method = find_method_in_inheritance_chain(klass, name);

  if (IS_NIL(method)) {
    runtime_error("Undefined method '%s' in '%s' or any of its parent classes", name->chars,
                  klass->name->chars);
    return CALL_FAILED;
  }

  switch (AS_OBJ(method)->type) {
    case OBJ_CLOSURE: return call_managed(AS_CLOSURE(method), arg_count);
    case OBJ_NATIVE: return call_native(AS_NATIVE(method), arg_count);
    default: {
      runtime_error("Cannot invoke method of type '%s' on class", type_name(method));
      return CALL_FAILED;
    }
  }
}

// Invokes a managed-code or native method with receiver and arguments on the top of the stack.
// `Stack: ...[receiver][arg0][arg1]...[argN]`
static CallResult invoke(ObjString* name, int arg_count) {
  Value receiver  = peek(arg_count);
  ObjClass* klass = type_of(receiver);

  // Handle primitive and internal types which have a corresponding class
  // Everything which is not an instance qualifies as such.
  if (!IS_INSTANCE(receiver)) {
    return invoke_from_class(klass, name, arg_count);
  }

  // Handle managed-code class instances
  ObjInstance* instance = AS_INSTANCE(receiver);

  // It could be a field which is a function, we need to check that first
  Value function;
  if (hashtable_get(&instance->fields, OBJ_VAL(name), &function)) {
    vm.stack_top[-arg_count - 1] = function;
    return call_value(function, arg_count);
  }

  return invoke_from_class(klass, name, arg_count);
}

// Executes a callframe by running the bytecode until it returns a value or an error occurs.
static Value run_frame() {
  int exit_on_frame = vm.exit_on_frame;
  vm.exit_on_frame  = vm.frame_count - 1;

  Value result = run();

  vm.exit_on_frame = exit_on_frame;
  return result;
}

// Internal function to execute a call to a managed-code OR native function.
// Arguments are expected to be on the stack, with the function preceding them.
// Returns the value returned by the function, or NIL_VAL if the call failed.
//
// This is pretty similar to call_value, but it's intended to also EXECUTE the function. For native functions,
// the result will be available "immediately", but for managed_code we have to execute the new call frame
// (provided by call_managed) to get to the result. That's why we have this function - so we can execute any
// callable value internally
static Value exec_fn(Obj* callable, int arg_count) {
  CallResult result = CALL_FAILED;
  switch (callable->type) {
    case OBJ_CLOSURE: result = call_managed((ObjClosure*)callable, arg_count); break;
    case OBJ_NATIVE: result = call_native(((ObjNative*)callable)->function, arg_count); break;
  }

  if (result == CALL_RETURNED) {
    return pop();
  } else if (result == CALL_RUNNING) {
    return run_frame();
  }

  return NIL_VAL;
}

// Internal function to execute a method on a value. This is similar to exec_fn, but it's expected that
// there's a receiver on the stack before the arguments.
// `Stack: ...[receiver][arg0][arg1]...[argN]`
static Value exec_method(ObjString* name, int arg_count) {
  Value receiver    = peek(arg_count);
  ObjClass* klass   = type_of(receiver);
  CallResult result = invoke_from_class(klass, name, arg_count);

  if (result == CALL_RETURNED) {
    return pop();
  } else if (result == CALL_RUNNING) {
    return run_frame();
  }

  return NIL_VAL;
}

// Function to convert any value to a string.
// This is faster than using the "to_str" method, but it's less flexible.
// Should probably test how much faster it is, because I don't like it and want to remove it.
// I'd like to just call a values to_str method - but there's still a bug in there somewhere, we'll leave it
// for now.
static ObjString* fast_to_str(Value value) {
  switch (value.type) {
    case VAL_BOOL: return AS_STRING(BUILTIN_METHOD(TYPENAME_BOOL, to_str)(0, &value));
    case VAL_NIL: return AS_STRING(BUILTIN_METHOD(TYPENAME_NIL, to_str)(0, &value));
    case VAL_NUMBER: return AS_STRING(BUILTIN_METHOD(TYPENAME_NUMBER, to_str)(0, &value));
    case VAL_OBJ: {
      switch (OBJ_TYPE(value)) {
        case OBJ_STRING: return AS_STRING(BUILTIN_METHOD(TYPENAME_STRING, to_str)(0, &value));
        case OBJ_SEQ: return AS_STRING(BUILTIN_METHOD(TYPENAME_SEQ, to_str)(0, &value));
        case OBJ_MAP: return AS_STRING(BUILTIN_METHOD(TYPENAME_MAP, to_str)(0, &value));
        default: break;
      }
      break;
    }
    default: break;
  }

  return AS_STRING(BUILTIN_METHOD(TYPENAME_OBJ, to_str)(0, &value));
}

//
// Native functions. These are functions that are implemented in C and are available to user code.
// Natives do not do anything with argv[0] - that's reserved for the receiver (or the function itself).
//

// Native clock function.
BUILTIN_FN_DOC(
    /* Fn Name     */ clock,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the current execution time in miliseconds.");
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
  if (!hashtable_get(&vm.module->fields, vm.cached_words[WORD_FILE_PATH], &cwd)) {
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
    ObjString* str = fast_to_str(argv[i]);
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
    /* Fn Name     */ type_of,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_CLASS,
    /* Description */ "Returns the class of the value.");
BUILTIN_FN_IMPL(type_of) {
  BUILTIN_CHECK_ARGC_ONE()

  ObjClass* klass = type_of(argv[1]);
  return OBJ_VAL(klass);
}

// Native type name.
BUILTIN_FN_DOC(
    /* Fn Name     */ type_name,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns the name of the value's type.");
BUILTIN_FN_IMPL(type_name) {
  BUILTIN_CHECK_ARGC_ONE()

  const char* t_name = type_name(argv[1]);
  ObjString* str_obj = copy_string(t_name, (int)strlen(t_name));
  return OBJ_VAL(str_obj);
}

//
// Built-in methods of built-in classes
//

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

// Built-in function to convert an object to a string. This one is special in that is is the toplevel to_str -
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
        ObjString* str_obj = copy_string(chars, (int)buf_size);

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
        ObjString* str_obj = copy_string(chars, (int)buf_size);

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
        ObjString* str_obj = copy_string(chars, (int)buf_size);

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
  ObjString* str_obj = copy_string(chars, (int)buf_size);

  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in function to convert a nil to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of " STR(TYPENAME_NIL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(NIL)

  ObjString* str_obj = copy_string(VALUE_STR_NIL, sizeof(VALUE_STR_NIL) - 1);
  return OBJ_VAL(str_obj);
}

// Built-in function to convert a value to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_BOOL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(BOOL)

  if (AS_BOOL(argv[0])) {
    ObjString* str_obj = copy_string(VALUE_STR_TRUE, sizeof(VALUE_STR_TRUE) - 1);
    return OBJ_VAL(str_obj);
  } else {
    ObjString* str_obj = copy_string(VALUE_STR_FALSE, sizeof(VALUE_STR_FALSE) - 1);
    return OBJ_VAL(str_obj);
  }
}

// Built-in function to convert a number to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(NUMBER)

  double number = AS_NUMBER(argv[0]);
  char buffer[64];
  int integer;
  int len = 0;

  if (is_int(number, &integer)) {
    len = snprintf(buffer, sizeof(buffer), VALUE_STR_INT, integer);
  } else {
    len = snprintf(buffer, sizeof(buffer), VALUE_STR_FLOAT, number);
  }

  ObjString* str_obj = copy_string(buffer, len);
  return OBJ_VAL(str_obj);
}

// Built-in function to convert a string to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(STRING)

  return argv[0];
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NUMBER,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Converts the first argument to a " STR(
        TYPENAME_NUMBER) ", if provided. Otherwise, returns a default " STR(TYPENAME_NUMBER) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NUMBER, __ctor) {
  if (argc == 0) {
    return NUMBER_VAL(0);
  }

  BUILTIN_CHECK_ARGC_ONE()

  switch (argv[1].type) {
    case VAL_NUMBER: return argv[1];
    case VAL_BOOL: return AS_BOOL(argv[1]) ? NUMBER_VAL(1) : NUMBER_VAL(0);
    case VAL_NIL: return NUMBER_VAL(0);
    case VAL_OBJ: {
      switch (AS_OBJ(argv[1])->type) {
        case OBJ_STRING: {
          ObjString* str = AS_STRING(argv[1]);
          return NUMBER_VAL(string_to_double(str->chars, str->length));
        }
      }
    }
  }

  return NUMBER_VAL(0);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_BOOL,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_BOOL) ", if provided. Otherwise, returns a default " STR(
        TYPENAME_BOOL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_BOOL, __ctor) {
  if (argc == 0) {
    return BOOL_VAL(false);
  }

  BUILTIN_CHECK_ARGC_ONE()

  if (is_falsey(argv[1])) {
    return BOOL_VAL(false);
  }

  return BOOL_VAL(true);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_NIL,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_NIL,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_NIL) ", if provided. Otherwise, returns a default " STR(
        TYPENAME_NIL) ".");
BUILTIN_METHOD_IMPL(TYPENAME_NIL, __ctor) {
  UNUSED(argv);

  if (argc == 0) {
    return NIL_VAL;
  }

  BUILTIN_CHECK_ARGC_ONE()

  return NIL_VAL;
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Converts the first argument to a " STR(
        TYPENAME_STRING) ", if provided. Otherwise, returns a default " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, __ctor) {
  if (argc == 0) {
    return OBJ_VAL(copy_string("", 0));
  }

  BUILTIN_CHECK_ARGC_ONE()

  push(argv[1]);  // Push the receiver for to_str, which is the ctors' argument
  return exec_method(copy_string("to_str", 6), 0);  // Convert to string
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("len", TYPENAME_NUMBER),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Creates a new " STR(TYPENAME_NIL) "-initialized " STR(
        TYPENAME_SEQ) " of length 'len', if provided. Otherwise, creates a new empty " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, __ctor) {
  if (argc == 0) {
    ValueArray items;
    init_value_array(&items);
    ObjSeq* seq = take_seq(&items);
    return OBJ_VAL(seq);
  }

  BUILTIN_CHECK_ARGC_ONE()
  BUILTIN_CHECK_ARG_AT(1, NUMBER)

  ValueArray items;
  init_value_array(&items);
  ObjSeq* seq = take_seq(&items);
  push(OBJ_VAL(seq));  // GC Protection

  int count = (int)AS_NUMBER(argv[1]);
  for (int i = 0; i < count; i++) {
    write_value_array(&seq->items, NIL_VAL);
  }

  pop();  // The seq
  return OBJ_VAL(seq);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ __ctor,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_MAP,
    /* Description */ "Returns a new empty " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, __ctor) {
  UNUSED(argv);
  BUILTIN_CHECK_ARGC_ZERO()

  HashTable entries;
  init_hashtable(&entries);
  ObjMap* map = take_map(&entries);
  return OBJ_VAL(map);
}

// Built-in function to convert a sequence to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);

  strcpy(chars, VALUE_STR_SEQ_START);
  for (int i = 0; i < seq->items.count; i++) {
    // Use the default to-string method of the value to convert the item to a string
    push(seq->items.values[i]);  // Push the receiver (item at i) for to_str
    ObjString* item_str = AS_STRING(exec_method(copy_string("to_str", 6), 0));

    // Expand chars to fit the separator plus the next item
    size_t new_buf_size =
        strlen(chars) + strlen(item_str->chars) + (sizeof(VALUE_STR_SEQ_DELIM) - 1) +
        (sizeof(VALUE_STR_SEQ_END) - 1);  // Consider the closing bracket -  if we're done after this
                                          // iteration we won't need to expand and can just slap it on there
    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size = new_buf_size;
      chars    = realloc(chars, buf_size);
      if (chars == NULL) {
        return OBJ_VAL(copy_string("[???]", 5));
      }
    }

    // Append the string
    strcat(chars, item_str->chars);
    if (i < seq->items.count - 1) {
      strcat(chars, VALUE_STR_SEQ_DELIM);
    }
  }

  strcat(chars, VALUE_STR_SEQ_END);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size);
  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in function to convert a map to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_MAP,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map     = AS_MAP(argv[0]);
  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);
  int processed   = 0;  // Keep track of how many non-EMPTY entries we've processed to know when skip the last
                        // delimiter

  strcpy(chars, VALUE_STR_MAP_START);
  for (int i = 0; i < map->entries.capacity; i++) {
    if (IS_EMPTY_INTERNAL(map->entries.entries[i].key)) {
      continue;
    }

    // Use the default to-string methods for key and value
    push(map->entries.entries[i].key);  // Push the receiver (key at i) for to_str
    ObjString* key_str = AS_STRING(exec_method(copy_string("to_str", 6), 0));
    push(OBJ_VAL(key_str));               // GC Protection
    push(map->entries.entries[i].value);  // Push the receiver (value at i) for to_str
    ObjString* value_str = AS_STRING(exec_method(copy_string("to_str", 6), 0));
    pop();  // Key str

    // Expand chars to fit the separator, delimiter plus the next key and value
    size_t new_buf_size = strlen(chars) + strlen(key_str->chars) + strlen(value_str->chars) +
                          (sizeof(VALUE_STR_MAP_SEPARATOR) - 1) + (sizeof(VALUE_STR_MAP_DELIM) - 1) +
                          (sizeof(VALUE_STR_MAP_END) - 1);  // Consider the closing bracket -
                                                            // if we're done after this
                                                            // iteration we won't need to
                                                            // expand and can just slap it on there
    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size = new_buf_size;
      chars    = realloc(chars, buf_size);
      if (chars == NULL) {
        return OBJ_VAL(copy_string("{???}", 5));
      }
    }

    // Append the strings
    strcat(chars, key_str->chars);
    strcat(chars, VALUE_STR_MAP_SEPARATOR);
    strcat(chars, value_str->chars);
    if (processed < map->entries.count - 1) {
      strcat(chars, VALUE_STR_MAP_DELIM);
    }
    processed++;
  }

  strcat(chars, VALUE_STR_MAP_END);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)buf_size);
  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in function to retrieve the length of a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ len,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, len) {
  BUILTIN_CHECK_ARGC_ZERO()

  int length = AS_STRING(argv[0])->length;
  return NUMBER_VAL(length);
}

// Built-in function to retrieve the length of a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ len,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, len) {
  BUILTIN_CHECK_ARGC_ZERO()

  int length = AS_SEQ(argv[0])->items.count;
  return NUMBER_VAL(length);
}

// Built-in function to retrieve the length of a map
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ len,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, len) {
  BUILTIN_CHECK_ARGC_ZERO()

  int length = AS_MAP(argv[0])->entries.count;
  return NUMBER_VAL(length);
}

// Built-in function to push an arbitrary amount of values onto a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ push,
    /* Arguments   */ DOC_ARG("arg1", TYPENAME_OBJ) DOC_ARG_SEP DOC_ARG_REST,
    /* Return Type */ TYPENAME_NIL,
    /* Description */ "Pushes one or many values to a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, push) {
  BUILTIN_CHECK_ARGC_AT_LEAST(1)
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  for (int i = 1; i <= argc; i++) {
    write_value_array(&seq->items, argv[i]);
  }
  return NIL_VAL;
}

// Built-in function to pop a value from a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ pop,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_OBJ,
    /* Description */
    "Pops and returns the last item of a " STR(TYPENAME_SEQ) ". Returns " STR(
        TYPENAME_NIL) " if it is empty.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, pop) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  return pop_value_array(&seq->items);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ entries,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Returns a " STR(TYPENAME_SEQ) " of key-value pairs (which are " STR(
        TYPENAME_SEQ) "s of length 2 ) of a " STR(TYPENAME_MAP) ", containing all entries.");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, entries) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map = AS_MAP(argv[0]);

  int processed = 0;
  for (int i = 0; i < map->entries.capacity; i++) {
    Entry* entry = &map->entries.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      push(entry->key);
      push(entry->value);
      make_seq(2);  // Leaves a seq with the two values on the stack
      processed++;
    }
  }

  make_seq(processed);
  Value seq = pop();  // The seq made by make_seq

  return seq;
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ keys,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_SEQ) " of all keys of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, keys) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map = AS_MAP(argv[0]);

  int processed = 0;
  for (int i = 0; i < map->entries.capacity; i++) {
    Entry* entry = &map->entries.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      push(entry->key);
      processed++;
    }
  }

  make_seq(processed);
  Value seq = pop();  // The seq made by make_seq

  return seq;
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_MAP,
    /* Name        */ values,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_SEQ) " of all values of a " STR(TYPENAME_MAP) ".");
BUILTIN_METHOD_IMPL(TYPENAME_MAP, values) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(MAP)

  ObjMap* map = AS_MAP(argv[0]);

  int processed = 0;
  for (int i = 0; i < map->entries.capacity; i++) {
    Entry* entry = &map->entries.entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      push(entry->value);
      processed++;
    }
  }

  make_seq(processed);
  Value seq = pop();  // The seq made by make_seq

  return seq;
}

// Binds a method to an instance by creating a new bound method object and
// pushing it onto the stack.
static bool bind_method(ObjClass* klass, ObjString* name) {
  Value method = find_method_in_inheritance_chain(klass, name);
  if (IS_NIL(method)) {
    return false;
  }

  ObjBoundMethod* bound = new_bound_method(peek(0), AS_OBJ(method));
  pop();
  push(OBJ_VAL(bound));
  return true;
}

// Creates a new upvalue and inserts it into the linked list of open upvalues.
// Before inserting, it checks whether there is already an upvalue for the local
// variable. If there is, it returns that one instead.
static ObjUpvalue* capture_upvalue(Value* local) {
  ObjUpvalue* prev_upvalue = NULL;
  ObjUpvalue* upvalue      = vm.open_upvalues;

  // Are there any open upvalues from start (actually, start = the top of the
  // list, e.g. "last" in terms of reading a file with your eyes) to local?
  while (upvalue != NULL && upvalue->location > local) {
    prev_upvalue = upvalue;
    upvalue      = upvalue->next;
  }

  // If there is one, return it
  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  // Otherwise, create one and insert it into the list
  ObjUpvalue* created_upvalue = new_upvalue(local);
  created_upvalue->next       = upvalue;

  if (prev_upvalue == NULL) {
    vm.open_upvalues = created_upvalue;
  } else {
    prev_upvalue->next = created_upvalue;
  }

  return created_upvalue;
}

// Closes every upvalue until the given stack slot is reached.
// Closing upvalues moves them from the stack to the heap.
static void close_upvalues(Value* last) {
  while (vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
    ObjUpvalue* upvalue = vm.open_upvalues;
    upvalue->closed     = *upvalue->location;  // Move the value (via location pointer) from the
                                               // stack to the heap (closed field)
    upvalue->location = &upvalue->closed;      // Point to ourselves for the value
    vm.open_upvalues  = upvalue->next;
  }
}

// Adds a method to the class on top of the stack.
// The methods closure is on top of the stack, the class is one below that.
static void define_method(ObjString* name) {
  Value method    = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));  // We trust the compiler that this value
                                        // is actually a class
  hashtable_set(&klass->methods, OBJ_VAL(name), method);
  pop();
}

// Determines whether a value is falsey. We consider nil and false to be falsey,
// and everything else to be truthy.
static bool is_falsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

// Imports a module by name and pushes it onto the stack. If the module was already imported, it is loaded
// from cache. If the module was not imported yet, it is loaded from the file system and then cached.
// If module_path is NULL, the module is expected to be in the same directory as the
// importing module.
static void import_module(ObjString* module_name, ObjString* module_path) {
  Value module;

  // Check if we have already imported the module
  if (hashtable_get(&vm.modules, OBJ_VAL(module_name), &module)) {
    push(module);
    return;
  }

  // Not cached, so we need to load it. First, we need to get the current working directory
  Value cwd = BUILTIN_FN(cwd)(0, NULL);
  if (IS_NIL(cwd)) {
    runtime_error(
        "Could not import module '%s'. Could not get current working directory, because there is no "
        "active module or it is not a file.",
        module_name->chars);
    exit_with_runtime_error();
    return;
  }

  char* module_to_load_path;

  // Either we have a module path, or we check the current working directory
  if (module_path == NULL) {
    // Just slap the module name + extension onto the cwd
    char* module_file_name = ensure_slang_extension(module_name->chars);
    module_to_load_path    = join_path(AS_STRING(cwd)->chars, module_file_name);
    free(module_file_name);
  } else {
    // It's a probably realtive path, we add the extension to the provided path and prepend the cwd
    char* module_path_  = ensure_slang_extension(module_path->chars);
    module_to_load_path = join_path(AS_STRING(cwd)->chars, module_path_);
    free(module_path_);

    if (!file_exists(module_to_load_path)) {
      // Clearly, it's not a relative path.
      free(module_to_load_path);

      // We assume it is an absolute path insted, we infer that it has the extension already
      module_to_load_path = malloc(module_path->length + 1);
      strcpy(module_to_load_path, module_path->chars);
    }
  }

  if (module_to_load_path == NULL) {
    INTERNAL_ERROR(
        "Could not produce a valid module path for module '%s'. Cwd is '%s', additional path is "
        "'%s'",
        module_name->chars, AS_STRING(cwd)->chars, module_path == NULL ? "NULL" : module_path->chars);
    exit(74);
  }

  if (!file_exists(module_to_load_path)) {
    runtime_error("Could not import module '%s'. File '%s' does not exist.", module_name->chars,
                  module_to_load_path);
    exit_with_runtime_error();
    return;
  }

  // Load the module by running the file
  int previous_exit_frame = vm.exit_on_frame;
  vm.exit_on_frame        = vm.frame_count;
  module                  = run_file(module_to_load_path, module_name->chars);
  vm.exit_on_frame        = previous_exit_frame;

  // Check if the module is actually a module
  if (!IS_OBJ(module) && !IS_INSTANCE(module) && !(AS_INSTANCE(module)->klass == vm.__builtin_Module_class)) {
    free(module_to_load_path);
    runtime_error("Could not import module '%s'. Expected module type", module_name->chars);
    exit_with_runtime_error();
    return;
  }

  push(module);  // Show ourselves to the GC before we put it in the hashtable
  hashtable_set(&vm.modules, OBJ_VAL(module_name), module);

  free(module_to_load_path);
}

// Concatenates two strings on the stack (pops them) into a new string and pushes it onto the stack
static void concatenate() {
  ObjString* b = AS_STRING(peek(0));  // Peek, so it doesn't get freed by the GC
  ObjString* a = AS_STRING(peek(1));  // Peek, so it doesn't get freed by the GC

  int length  = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = take_string(chars, length);
  pop();  // Pop, because we only peeked
  pop();  // Pop, because we only peeked
  push(OBJ_VAL(result));
}

// This function represents the main loop of the virtual machine.
// It fetches the next instruction, decodes it, and dispatches it
static Value run() {
  CallFrame* frame = &vm.frames[vm.frame_count - 1];

// Read a single piece of data from the current instruction pointer and advance
// it
#define READ_ONE() (*frame->ip++)

// Read a constant from the constant pool. This consumes one piece of data
// on the stack, which is the index of the constant to read
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_ONE()])

// Read a string from the constant pool.
#define READ_STRING() AS_STRING(READ_CONSTANT())

// Perform a binary operation on the top two values on the stack.
// This consumes two pieces of data from the stack, and pushes the result
// value_type is the type of the result value, op is the operator to use
#define BINARY_OP(value_type, op)                                                               \
  do {                                                                                          \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                                           \
      runtime_error("Operands must be numbers. Left was %s, right was %s.", type_name(peek(1)), \
                    type_name(peek(0)));                                                        \
      return exit_with_runtime_error();                                                         \
    }                                                                                           \
    double b = AS_NUMBER(pop());                                                                \
    double a = AS_NUMBER(pop());                                                                \
    push(value_type(a op b));                                                                   \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    disassemble_instruction(&frame->closure->function->chunk,
                            (int)(frame->ip - frame->closure->function->chunk.code));

    printf(ANSI_CYAN_STR(" Stack "));
    for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
      printf(ANSI_CYAN_STR("["));
      print_value_safe(stdout, *slot);
      printf(ANSI_CYAN_STR("]"));
    }
    printf("\n");
#endif

    uint16_t instruction;
    switch (instruction = READ_ONE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_NIL: push(NIL_VAL); break;
      case OP_TRUE: push(BOOL_VAL(true)); break;
      case OP_FALSE: push(BOOL_VAL(false)); break;
      case OP_POP: pop(); break;
      case OP_GET_LOCAL: {
        uint16_t slot = READ_ONE();
        push(frame->slots[slot]);
        break;
      }
      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!hashtable_get(frame->globals, OBJ_VAL(name), &value)) {
          if (!hashtable_get(&vm.builtin->fields, OBJ_VAL(name), &value)) {
            runtime_error("Undefined variable '%s'.", name->chars);
            return exit_with_runtime_error();
          }
        }
        push(value);
        break;
      }
      case OP_GET_UPVALUE: {
        uint16_t slot = READ_ONE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }
      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        hashtable_set(frame->globals, OBJ_VAL(name), peek(0));
        pop();
        break;
      }
      case OP_SET_LOCAL: {
        uint16_t slot      = READ_ONE();
        frame->slots[slot] = peek(0);  // peek, because assignment is an expression!
        break;
      }
      case OP_SET_GLOBAL: {
        ObjString* name = READ_STRING();
        if (hashtable_set(frame->globals, OBJ_VAL(name),
                          peek(0))) {  // peek, because assignment is an expression!
          hashtable_delete(frame->globals, OBJ_VAL(name));
          runtime_error("Undefined variable '%s'.", name->chars);
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SET_UPVALUE: {
        uint16_t slot                             = READ_ONE();
        *frame->closure->upvalues[slot]->location = peek(0);  // peek, because assignment is an expression!
        break;
      }
      case OP_GET_INDEX: {
        // We don't trigger the GC in this block, so we can just pop the stuff
        Value index   = pop();
        Value indexee = pop();

        if (IS_STRING(indexee)) {
          if (!IS_NUMBER(index)) {
            runtime_error(STR(TYPENAME_STRING) " indices must be " STR(TYPENAME_NUMBER) "s, but got %s.",
                          type_name(index));
            return exit_with_runtime_error();
          }

          double i_raw = AS_NUMBER(index);
          int i;
          if (!is_int(i_raw, &i)) {
            push(NIL_VAL);
            break;
          }

          ObjString* string = AS_STRING(indexee);
          if (i < 0 || i >= string->length) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          ObjString* char_str = copy_string(AS_CSTRING(indexee) + i, 1);
          push(OBJ_VAL(char_str));
          break;
        } else if (IS_SEQ(indexee)) {
          if (!IS_NUMBER(index)) {
            runtime_error(STR(TYPENAME_SEQ) " indices must be " STR(TYPENAME_NUMBER) "s, but got %s.",
                          type_name(index));
            return exit_with_runtime_error();
          }

          double i_raw = AS_NUMBER(index);
          int i;
          if (!is_int(i_raw, &i)) {
            push(NIL_VAL);
            break;
          }

          ObjSeq* seq = AS_SEQ(indexee);
          if (i < 0 || i >= seq->items.count) {
            runtime_error("Index out of bounds. Was %d, but this " STR(TYPENAME_SEQ) " has length %d.", i,
                          seq->items.count);
            return exit_with_runtime_error();
          }

          push(seq->items.values[i]);
        } else if (IS_MAP(indexee)) {
          ObjMap* map = AS_MAP(indexee);
          Value value;
          if (hashtable_get(&map->entries, index, &value)) {
            push(value);
          } else {
            push(NIL_VAL);
          }
        } else {
          runtime_error("%s cannot be get-indexed.", type_name(indexee));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SET_INDEX: {
        // Some of the code in this block might trigger the GC, so we need to peek
        Value assignee = peek(2);  // Peek(0) is the value, Peek(1) is the index, Peek(2) is the assignee

        if (IS_SEQ(assignee)) {
          // We can pop, bc we don't trigger the GC in this block
          Value value = pop();  // value
          Value index = pop();  // index
          pop();                // assignee

          if (!IS_NUMBER(index)) {
            runtime_error(STR(TYPENAME_SEQ) " indices must be " STR(TYPENAME_NUMBER) "s, but got %s.",
                          type_name(index));
            return exit_with_runtime_error();
          }

          double i_raw = AS_NUMBER(index);
          int i;
          if (!is_int(i_raw, &i)) {
            push(NIL_VAL);
            break;
          }

          ObjSeq* seq = AS_SEQ(assignee);

          if (i < 0 || i >= seq->items.count) {
            runtime_error("Index out of bounds. Was %d, but this " STR(TYPENAME_SEQ) " has length %d.", i,
                          seq->items.count);
            return exit_with_runtime_error();
          }

          seq->items.values[i] = value;
          push(value);
        } else if (IS_MAP(assignee)) {
          // We peek, because we might trigger the GC
          Value value = peek(0);  // value
          Value index = peek(1);  // index

          ObjMap* map = AS_MAP(assignee);
          hashtable_set(&map->entries, index, value);
          pop();        // value
          pop();        // index
          pop();        // assignee
          push(value);  // Push back onto the stack, because assignment is an expression
        } else {
          runtime_error("%s cannot be set-indexed.", type_name(assignee));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_GET_PROPERTY: {
        ObjString* name = READ_STRING();
        switch (peek(0).type) {
          case VAL_OBJ: {
            switch (OBJ_TYPE(peek(0))) {
              case OBJ_NATIVE: {
                ObjNative* native = ((ObjNative*)AS_OBJ(peek(0)));
                if (values_equal(OBJ_VAL(name), OBJ_VAL(copy_string("doc", 3)))) {
                  pop();  // Pop the native
                  push(OBJ_VAL(native->doc));
                  goto done_getting_property;
                }
                break;
              }
              case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(peek(0));
                if (values_equal(OBJ_VAL(name), vm.cached_words[WORD_NAME])) {
                  pop();  // Pop the class
                  push(OBJ_VAL(klass->name));
                  goto done_getting_property;
                } else if (values_equal(OBJ_VAL(name), vm.cached_words[WORD_CTOR])) {
                  pop();  // Pop the class
                  Value ctor;
                  if (hashtable_get(&klass->methods, OBJ_VAL(name), &ctor)) {
                    push(ctor);
                  } else {
                    push(NIL_VAL);
                  }
                  goto done_getting_property;
                }
                break;
              }
              case OBJ_INSTANCE: {
                ObjInstance* instance = AS_INSTANCE(peek(0));
                Value value;
                if (hashtable_get(&instance->fields, OBJ_VAL(name), &value)) {
                  pop();  // Instance.
                  push(value);
                  goto done_getting_property;
                }
                break;
              }
            }
            break;
          }
        }

        ObjClass* klass = type_of(peek(0));
        if (bind_method(klass, name)) {
          goto done_getting_property;
        }

        runtime_error("Property '%s' does not exist on type %s.", name->chars, type_name(peek(0)));
        return exit_with_runtime_error();

      done_getting_property:
        break;
      }
      case OP_SET_PROPERTY: {
        Value obj       = peek(1);
        ObjString* name = READ_STRING();

        if (!IS_INSTANCE(obj)) {
          runtime_error("Cannot set field '%s' on value of type %s.", name->chars, type_name(peek(1)));
          return exit_with_runtime_error();
        }

        ObjInstance* instance = AS_INSTANCE(obj);

        // TODO (robust): Not all cached words are actually **reserved**. We should make an enum for
        // reserved words and check against that instead.
        for (int i = 0; i < WORD_MAX; i++) {
          if (strcmp(name->chars, AS_STRING(vm.cached_words[i])->chars) == 0) {
            runtime_error("Cannot set reserved field '%s'.", name->chars);
            return exit_with_runtime_error();
          }
        }

        hashtable_set(&instance->fields, OBJ_VAL(name), peek(0));  // Create or update
        Value value = pop();
        pop();
        push(value);
        break;
      }
      case OP_IMPORT_FROM: {
        ObjString* name = READ_STRING();
        ObjString* from = READ_STRING();
        import_module(name, from);
        break;
      }
      case OP_IMPORT: {
        ObjString* name = READ_STRING();
        import_module(name, NULL);
        break;
      }
      case OP_GET_BASE_METHOD: {
        ObjString* name     = READ_STRING();
        ObjClass* baseclass = AS_CLASS(pop());

        if (!bind_method(baseclass, name)) {
          runtime_error("Property '%s' does not exist in '%s'.", name->chars, baseclass->name->chars);
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_EQ: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(values_equal(a, b)));
        break;
      }
      case OP_NEQ: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(!values_equal(a, b)));
        break;
      }
      case OP_GT: BINARY_OP(BOOL_VAL, >); break;
      case OP_LT: BINARY_OP(BOOL_VAL, <); break;
      case OP_GTEQ: BINARY_OP(BOOL_VAL, >=); break;
      case OP_LTEQ: BINARY_OP(BOOL_VAL, <=); break;
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtime_error(
              "Operands must be two numbers or two strings. Left was "
              "%s, right was %s.",
              type_name(peek(1)), type_name(peek(0)));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
      case OP_NOT: push(BOOL_VAL(is_falsey(pop()))); break;
      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          runtime_error("Operand must be a number. Was %s.", type_name(peek(0)));
          return exit_with_runtime_error();
        }
        push(NUMBER_VAL(-AS_NUMBER(pop())));
        break;
      case OP_PRINT: {
        ObjString* str = fast_to_str(peek(0));
        printf("%s\n", str->chars);
        pop();
        break;
      }
      case OP_SEQ_LITERAL: {
        int count = READ_ONE();
        make_seq(count);
        break;
      }
      case OP_MAP_LITERAL: {
        int count = READ_ONE();
        make_map(count);
        break;
      }
      case OP_JUMP: {
        uint16_t offset = READ_ONE();
        frame->ip += offset;
        break;
      }
      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_ONE();
        if (is_falsey(peek(0)))
          frame->ip += offset;
        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_ONE();
        frame->ip -= offset;
        break;
      }
      case OP_CALL: {
        int arg_count = READ_ONE();
        if (call_value(peek(arg_count), arg_count) == CALL_FAILED) {
          return exit_with_runtime_error();
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_INVOKE: {
        ObjString* method = READ_STRING();
        int arg_count     = READ_ONE();
        if (invoke(method, arg_count) == CALL_FAILED) {
          return exit_with_runtime_error();
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_BASE_INVOKE: {
        ObjString* method   = READ_STRING();
        int arg_count       = READ_ONE();
        ObjClass* baseclass = AS_CLASS(pop());
        if (invoke_from_class(baseclass, method, arg_count) == CALL_FAILED) {
          return exit_with_runtime_error();
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure   = new_closure(function);
        push(OBJ_VAL(closure));

        // Bring closure to life
        for (int i = 0; i < closure->upvalue_count; i++) {
          uint16_t is_local = READ_ONE();
          uint16_t index    = READ_ONE();
          if (is_local) {
            closure->upvalues[i] = capture_upvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }
      case OP_CLOSE_UPVALUE:
        close_upvalues(vm.stack_top - 1);
        pop();
        break;
      case OP_RETURN: {
        Value result = pop();
        close_upvalues(frame->slots);
        vm.frame_count--;
        if (vm.frame_count == 0) {
          return pop();  // Return the toplevel function - used for modules.
        }

        vm.stack_top = frame->slots;
        if (vm.exit_on_frame == vm.frame_count) {
          return result;
        }
        push(result);
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_CLASS:
        // Initially, a class always inherits from Object
        push(OBJ_VAL(new_class(READ_STRING(), vm.__builtin_Obj_class)));
        break;
      case OP_INHERIT: {
        Value baseclass    = peek(1);
        ObjClass* subclass = AS_CLASS(peek(0));
        if (!IS_CLASS(baseclass)) {
          runtime_error("Base class must be a class. Was %s.", type_name(baseclass));
          return exit_with_runtime_error();
        }
        hashtable_add_all(&AS_CLASS(baseclass)->methods, &subclass->methods);
        subclass->base = AS_CLASS(baseclass);
        pop();  // Subclass.
        break;
      }
      case OP_METHOD: define_method(READ_STRING()); break;
    }
  }

#undef READ_ONE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

void start_module(const char* source_path, const char* module_name) {
  ObjInstance* module = new_instance(vm.__builtin_Module_class);
  vm.module           = module;

  char* base_dir_path = base(source_path);

  define_obj(&module->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);
  define_obj(&module->fields, KEYWORD_MODULE_NAME,
             (Obj*)(copy_string(module_name, (int)strlen(module_name))));
  define_obj(&module->fields, KEYWORD_FILE_PATH,
             (Obj*)(copy_string(base_dir_path, (int)strlen(base_dir_path))));

  free(base_dir_path);
}

Value interpret(const char* source, const char* source_path, const char* module_name) {
  ObjInstance* enclosing_module = vm.module;
  bool is_module                = module_name != NULL && source_path != NULL;

  if (is_module) {
    start_module(source_path, module_name);
  }

  ObjFunction* function = compile_module(source);
  if (function == NULL) {
    exit_with_compile_error();
    return NIL_VAL;
  }

  push(OBJ_VAL(function));
  ObjClosure* closure = new_closure(function);
  pop();
  push(OBJ_VAL(closure));
  call_value(OBJ_VAL(closure), 0);

  Value result = run();

  if (is_module) {
    Value out = OBJ_VAL(vm.module);
    vm.module = enclosing_module;
    return out;
  }

  return result;
}

Value run_file(const char* path, const char* module_name) {
#ifdef DEBUG_TRACE_EXECUTION
  printf("\n");
  printf(ANSI_CYAN_STR("Running file: %s\n"), path);
#endif
  const char* name = module_name == NULL ? path : module_name;
  char* source     = read_file(path);

  if (source == NULL) {
    free(source);
    return NIL_VAL;
  }

  Value result = interpret(source, path, name);
  free(source);

#ifdef DEBUG_TRACE_EXECUTION
  printf(ANSI_CYAN_STR("Done running file: %s\n"), path);
  printf("\n");
#endif

  return result;
}
