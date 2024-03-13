#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compiler.h"
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

static Value __builtin_obj_to_str(int argc, Value argv[]);
static Value __builtin_bool_to_str(int argc, Value argv[]);
static Value __builtin_nil_to_str(int argc, Value argv[]);
static Value __builtin_number_to_str(int argc, Value argv[]);
static Value __builtin_class_to_str(int argc, Value argv[]);
static Value __builtin_function_to_str(int argc, Value argv[]);
static Value __builtin_closure_to_str(int argc, Value argv[]);
static Value __builtin_bound_method_to_str(int argc, Value argv[]);
static Value __builtin_instance_to_str(int argc, Value argv[]);
static Value __builtin_native_to_str(int argc, Value argv[]);
static Value __builtin_upvalue_to_str(int argc, Value argv[]);
static Value __builtin_string_to_str(int argc, Value argv[]);
static Value __builtin_seq_to_str(int argc, Value argv[]);
static ObjString* to_str(Value value);

static Value __builint_hash_value(int argc, Value argv[]);
static Value __builtin_get_typename(int argc, Value argv[]);
static Value __builtin_str_len(int argc, Value argv[]);
static Value __builtin_seq_len(int argc, Value argv[]);

static Value native_clock(int arg_count, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value native_print(int arg_count, Value* args) {
  for (int i = 0; i < arg_count; i++) {
    ObjString* str = to_str(args[i]);
    printf("%s", str->chars);
    if (i < arg_count - 1) {
      printf(" ");
    }
  }
  printf("\n");
  return NIL_VAL;
}

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
    fprintf(stderr, "\tat line %d ", function->chunk.lines[instruction]);
    fprintf(stderr, "in \"%s\"\n", function->name->chars);
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
static void define_native(HashTable* table, const char* name, NativeFn function) {
  push(OBJ_VAL(copy_string(name, (int)strlen(name))));
  push(OBJ_VAL(new_native(function)));
  hashtable_set(table, vm.stack[0], vm.stack[1]);
  pop();
  pop();
}

// Defines an object in the given table.
static void define_obj(HashTable* table, const char* name, Obj* obj) {
  push(OBJ_VAL(copy_string(name, (int)strlen(name))));
  push(OBJ_VAL(obj));
  hashtable_set(table, vm.stack[0], vm.stack[1]);
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

  items.values   = GROW_ARRAY(Value, items.values, 0, count);
  items.capacity = count;
  items.count    = count;

  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = pop();
  }

  ObjSeq* seq = take_seq(&items);
  push(OBJ_VAL(seq));
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

  // Create the reserved method names
  memset(vm.reserved_method_names, 0, sizeof(vm.reserved_method_names));
  vm.reserved_method_names[METHOD_CTOR] = OBJ_VAL(copy_string(KEYWORD_CONSTRUCTOR, KEYWORD_CONSTRUCTOR_LEN));
  vm.reserved_method_names[METHOD_NAME] = OBJ_VAL(copy_string(KEYWORD_NAME, KEYWORD_NAME_LEN));

  // Create the object class
  vm.object_class = new_class(copy_string("Object", 6), NULL);

  // Create the builtin obj instance
  vm.builtin = new_instance(vm.object_class);
  define_obj(&vm.builtin->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);
  define_native(&vm.builtin->fields, "clock", native_clock);
  define_native(&vm.builtin->fields, "log", native_print);

  // ... and define the object methods
  define_obj(&vm.builtin->fields, "Object", (Obj*)vm.object_class);
  define_native(&vm.object_class->methods, "str", __builtin_obj_to_str);
  define_native(&vm.object_class->methods, "hash", __builint_hash_value);
  define_native(&vm.object_class->methods, "type_name", __builtin_get_typename);

  // Create the module class
  vm.module_class = new_class(copy_string("Module", 6), vm.object_class);
  define_obj(&vm.builtin->fields, "Module", (Obj*)vm.module_class);

  // Create the string class
  vm.string_class = new_class(copy_string(TYPENAME_STRING, sizeof(TYPENAME_STRING) - 1), vm.object_class);
  define_obj(&vm.builtin->fields, TYPENAME_STRING, (Obj*)vm.string_class);
  define_native(&vm.string_class->methods, "str", __builtin_string_to_str);
  define_native(&vm.string_class->methods, "len", __builtin_str_len);

  // Create the number class
  vm.number_class = new_class(copy_string(TYPENAME_NUMBER, sizeof(TYPENAME_NUMBER) - 1), vm.object_class);
  define_obj(&vm.builtin->fields, TYPENAME_NUMBER, (Obj*)vm.number_class);
  define_native(&vm.number_class->methods, "str", __builtin_number_to_str);

  // Create the boolean class
  vm.bool_class = new_class(copy_string(TYPENAME_BOOL, sizeof(TYPENAME_BOOL) - 1), vm.object_class);
  define_obj(&vm.builtin->fields, TYPENAME_BOOL, (Obj*)vm.bool_class);
  define_native(&vm.bool_class->methods, "str", __builtin_bool_to_str);

  // Create the nil class
  vm.nil_class = new_class(copy_string(TYPENAME_NIL, sizeof(TYPENAME_NIL) - 1), vm.object_class);
  define_obj(&vm.builtin->fields, TYPENAME_NIL, (Obj*)vm.nil_class);
  define_native(&vm.nil_class->methods, "str", __builtin_nil_to_str);

  // Create the sequence class
  vm.seq_class = new_class(copy_string(TYPENAME_SEQ, sizeof(TYPENAME_SEQ) - 1), vm.object_class);
  define_obj(&vm.builtin->fields, TYPENAME_SEQ, (Obj*)vm.seq_class);
  define_native(&vm.seq_class->methods, "str", __builtin_seq_to_str);
  define_native(&vm.seq_class->methods, "len", __builtin_seq_len);

  vm.pause_gc = 0;
  reset_stack();
}

void free_vm() {
  free_hashtable(&vm.strings);
  memset(vm.reserved_method_names, 0, sizeof(vm.reserved_method_names));
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
static ObjClass* value_get_class(Value value) {
  // Handle primitive and internal types which have a corresponding class
  if (!IS_INSTANCE(value)) {
    switch (value.type) {
      case VAL_NIL: return vm.nil_class;
      case VAL_BOOL: return vm.bool_class;
      case VAL_NUMBER: return vm.number_class;
      case VAL_OBJ: {
        switch (OBJ_TYPE(value)) {
          case OBJ_STRING: return vm.string_class;
          case OBJ_SEQ: return vm.seq_class;
        }
      }
    }
    return vm.object_class;
  }

  // Instances are easy, just return the class. Most of them are probably managed-code classes.
  return AS_INSTANCE(value)->klass;
}

// Executes a call to a managed-code function or method by creating a new call frame and pushing it onto the
// frame stack. Returns true if the call succeeded, false otherwise.
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
  frame->slots     = vm.stack_top - arg_count - 1;
  frame->globals   = &closure->function->globals_context->fields;

  return CALL_RUNNING;
}

// Calls a native function with the given number of arguments (on the stack).
// Returns true if the call succeeded, false otherwise.
// If has_receiver is true then it is invoked as if it was a class method. Meaning, it assumes that before the
// arguments on the stack is the receiver (acting as an "instance") of the method.
static CallResult call_native(NativeFn native, int arg_count, bool has_receiver) {
  Value* args  = vm.stack_top - arg_count - (has_receiver ? 1 : 0);
  Value result = native(arg_count, args);
  vm.stack_top -= arg_count + 1;
  push(result);

  return CALL_RETURNED;
}

// Calls a callable value (function, method, class (ctor), etc.)
// Returns true if the call succeeded, false otherwise.
static CallResult call_value(Value callee, int arg_count) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_BOUND_METHOD: {
        ObjBoundMethod* bound        = AS_BOUND_METHOD(callee);
        vm.stack_top[-arg_count - 1] = bound->receiver;  // Slot 0, so we can access "this"
        return call_value(OBJ_VAL(bound->method), arg_count);
      }
      case OBJ_CLASS: {
        ObjClass* klass = AS_CLASS(callee);
        vm.stack_top[-arg_count - 1] =
            OBJ_VAL(new_instance(klass));  // This is the actual "this", e.g. new instance
        Value ctor;                        // The 'ctor' is actually optional

        if (hashtable_get(&klass->methods, vm.reserved_method_names[METHOD_CTOR], &ctor)) {
          return call_managed(AS_CLOSURE(ctor), arg_count);
        } else if (arg_count != 0) {
          runtime_error("Expected 0 arguments but got %d.", arg_count);
          return CALL_FAILED;
        }
        return CALL_RETURNED;
      }
      case OBJ_CLOSURE: return call_managed(AS_CLOSURE(callee), arg_count);
      case OBJ_FUNCTION: return call_managed(AS_FUNCTION(callee), arg_count);
      case OBJ_NATIVE:
        return call_native(AS_NATIVE(callee), arg_count, false /* no receiver, just a plain call */);
      default: break;  // Non-callable object type.
    }
  }
  runtime_error("Attempted to call non-callable value of type %s.", type_name(callee));
  return CALL_FAILED;
}

// Invokes a method on a class by looking up the method in the class' method
// table and calling it. (Combines "get-property" and "call" opcodes)
// Scans upwards through the class hierarchy to find the method.
static CallResult invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) {
  ObjClass* source_klass = klass;

  while (source_klass != NULL) {
    Value method;
    if (hashtable_get(&source_klass->methods, OBJ_VAL(name), &method)) {
      switch (AS_OBJ(method)->type) {
        case OBJ_CLOSURE: return call_managed(AS_CLOSURE(method), arg_count);
        case OBJ_NATIVE: return call_native(AS_NATIVE(method), arg_count, true /* has receiver */);
        default: {
          runtime_error("Cannot invoke method of type '%s' on class", type_name(method));
          return CALL_FAILED;
        }
      }
    }
    source_klass = source_klass->base;
  }
  runtime_error("Undefined property '%s' in '%s' or any of its parent classes", name->chars,
                klass->name->chars);
  return CALL_FAILED;
}

// Invokes a method on the top of the stack.
static CallResult invoke(ObjString* name, int arg_count) {
  Value receiver  = peek(arg_count);
  ObjClass* klass = value_get_class(receiver);

  // Handle primitive and internal types which have a corresponding class
  if (!IS_INSTANCE(receiver)) {
    return invoke_from_class(klass, name, arg_count);
  }

  // Handle managed-code class instances
  ObjInstance* instance = AS_INSTANCE(receiver);
  Value value;

  // It could be a field which is a function, we need to check that first
  if (hashtable_get(&instance->fields, OBJ_VAL(name), &value)) {
    vm.stack_top[-arg_count - 1] = value;
    return call_value(value, arg_count);
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
static Value exec_fn(Obj* callee, int arg_count) {
  CallResult result = CALL_FAILED;
  switch (callee->type) {
    case OBJ_CLOSURE: result = call_managed((ObjClosure*)callee, arg_count); break;
    case OBJ_NATIVE:
      result = call_native((ObjNative*)callee, arg_count, false /* no receiver, just a plain call */);
      break;
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
static Value exec_method(ObjString* name, int arg_count) {
  Value receiver    = peek(arg_count);
  ObjClass* klass   = value_get_class(receiver);
  CallResult result = invoke_from_class(klass, name, arg_count);

  if (result == CALL_RETURNED) {
    return pop();
  } else if (result == CALL_RUNNING) {
    return run_frame();
  }

  return NIL_VAL;
}

// Built-in function to retrieve the hash value of a value
static Value __builint_hash_value(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 argument but got %d.", argc);
    return exit_with_runtime_error();
  }

  return NUMBER_VAL(hash_value(argv[0]));
}

// Built-in function to retrieve the type of a value in the form of a string
static Value __builtin_get_typename(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 argument but got %d.", argc);
    return exit_with_runtime_error();
  }

  const char* TYPENAME_ = type_name(argv[0]);
  int length            = (int)strlen(TYPENAME_);
  ObjString* str_obj    = copy_string(TYPENAME_, strlen(TYPENAME_));
  return OBJ_VAL(str_obj);
}

// Function to convert any value to a string.
// This is faster than using the "str" method, but it's less flexible.
// Should probably test how much faster it is, because I don't like it.
static ObjString* to_str(Value value) {
  switch (value.type) {
    case VAL_BOOL: return AS_STRING(__builtin_bool_to_str(0, &value));
    case VAL_NIL: return AS_STRING(__builtin_nil_to_str(0, &value));
    case VAL_NUMBER: return AS_STRING(__builtin_number_to_str(0, &value));
    case VAL_OBJ: {
      switch (OBJ_TYPE(value)) {
        case OBJ_CLASS: return AS_STRING(__builtin_class_to_str(0, &value));
        case OBJ_CLOSURE: return AS_STRING(__builtin_closure_to_str(0, &value));
        case OBJ_BOUND_METHOD: return AS_STRING(__builtin_bound_method_to_str(0, &value));
        case OBJ_INSTANCE: return AS_STRING(__builtin_instance_to_str(0, &value));
        case OBJ_NATIVE: return AS_STRING(__builtin_native_to_str(0, &value));
        case OBJ_STRING: return AS_STRING(__builtin_string_to_str(0, &value));
        case OBJ_SEQ: return AS_STRING(__builtin_seq_to_str(0, &value));
        case OBJ_FUNCTION: return AS_STRING(__builtin_function_to_str(0, &value));
        case OBJ_UPVALUE: return AS_STRING(__builtin_upvalue_to_str(0, &value));
        default: break;
      }
      break;
    }
    default: break;
  }

  return copy_string("???", 3);
}

// Built-in function to convert an object to a string
static Value __builtin_obj_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 argument but got %d.", argc);
    return exit_with_runtime_error();
  }

  return OBJ_VAL(copy_string("Object", 6));
}

// Built-in function to convert a nil to a string
static Value __builtin_nil_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_NIL(argv[0])) {
    ObjString* str_obj = copy_string("nil", 3);
    return OBJ_VAL(str_obj);
  }

  runtime_error("Expected nil but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a value to a string
static Value __builtin_bool_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_BOOL(argv[0])) {
    if (AS_BOOL(argv[0])) {
      ObjString* str_obj = copy_string("true", 4);
      return OBJ_VAL(str_obj);
    } else {
      ObjString* str_obj = copy_string("false", 5);
      return OBJ_VAL(str_obj);
    }
  }

  runtime_error("Expected a boolean but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a number to a string
static Value __builtin_number_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_NUMBER(argv[0])) {
    double number = AS_NUMBER(argv[0]);
    char buffer[64];
    int integer;
    int len = 0;

    if (is_int(number, &integer)) {
      len = snprintf(buffer, sizeof(buffer), "%d", integer);
    } else {
      len = snprintf(buffer, sizeof(buffer), "%f", number);
    }

    ObjString* str_obj = copy_string(buffer, len);
    return OBJ_VAL(str_obj);
  }

  runtime_error("Expected a number but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in functiont to convert a class to a string
static Value __builtin_class_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_CLASS(argv[0])) {
    ObjClass* klass = AS_CLASS(argv[0]);
    ObjString* name = klass->name;
    if (name == NULL || name->chars == NULL) {
      name = copy_string("???", 3);
    }
    push(OBJ_VAL(name));

    size_t buf_size = sizeof("<Class >") + name->length;
    char* chars     = malloc(buf_size);
    snprintf(chars, buf_size, "<Class %s>", name->chars);
    // Intuitively, you'd expect to use take_string here, but we don't know where malloc
    // allocates the memory - we don't want this block in our own memory pool.
    ObjString* str_obj = copy_string(chars, buf_size);

    free(chars);
    pop();
    return OBJ_VAL(str_obj);
  }

  runtime_error("Expected a class but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a function to a string
static Value __builtin_function_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_FUNCTION(argv[0])) {
    ObjFunction* function = AS_FUNCTION(argv[0]);
    ObjString* name       = function->name;
    if (name == NULL || name->chars == NULL) {
      name = copy_string("???", 3);
    }
    push(OBJ_VAL(name));

    size_t buf_size = sizeof("<Fn >") + name->length;
    char* chars     = malloc(buf_size);
    snprintf(chars, buf_size, "<Fn %s>", name->chars);
    // Intuitively, you'd expect to use take_string here, but we don't know where malloc
    // allocates the memory - we don't want this block in our own memory pool.
    ObjString* str_obj = copy_string(chars, buf_size);

    free(chars);
    pop();
    return OBJ_VAL(str_obj);
  }

  runtime_error("Expected a function but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a closure to a string
static Value __builtin_closure_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_CLOSURE(argv[0])) {
    ObjFunction* fn = AS_CLOSURE(argv[0])->function;
    return __builtin_function_to_str(0, &OBJ_VAL(fn));
  }

  runtime_error("Expected a closure but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a bound method to a string
static Value __builtin_bound_method_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_BOUND_METHOD(argv[0])) {
    ObjBoundMethod* bound = AS_BOUND_METHOD(argv[0]);
    if (bound->method->type == OBJ_CLOSURE) {
      return __builtin_function_to_str(0, &OBJ_VAL(((ObjClosure*)bound->method)->function));
    }

    return __builtin_native_to_str(0, &OBJ_VAL((ObjNative*)bound->method));
  }

  runtime_error("Expected a bound method but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert an instance to a string
static Value __builtin_instance_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_INSTANCE(argv[0])) {
    ObjInstance* instance = AS_INSTANCE(argv[0]);
    ObjString* name       = instance->klass->name;
    if (name == NULL || name->chars == NULL) {
      name = copy_string("???", 3);
    }
    push(OBJ_VAL(name));

    size_t buf_size = sizeof("<Instance >") + name->length;
    char* chars     = malloc(buf_size);
    snprintf(chars, buf_size, "<Instance %s>", name->chars);
    // Intuitively, you'd expect to use take_string here, but we don't know where malloc
    // allocates the memory - we don't want this block in our own memory pool.
    ObjString* str_obj = copy_string(chars, buf_size);

    free(chars);
    pop();
    return OBJ_VAL(str_obj);
  }

  runtime_error("Expected an instance but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a native function to a string
static Value __builtin_native_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_NATIVE(argv[0])) {
    return OBJ_VAL(copy_string("<Native Fn>", 11));
  }

  runtime_error("Expected a native fn but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert an upvalue to a string
static Value __builtin_upvalue_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  return OBJ_VAL(copy_string("<Upvalue>", 9));
}

// Built-in function to convert a string to a string
static Value __builtin_string_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_STRING(argv[0])) {
    return argv[0];
  }

  runtime_error("Expected a string but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
}

// Built-in function to convert a sequence to a string
static Value __builtin_seq_to_str(int argc, Value argv[]) {
#define SEQ_SEPARATOR ", "
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  if (IS_SEQ(argv[0])) {
    ObjSeq* seq     = AS_SEQ(argv[0]);
    size_t buf_size = 64;  // Start with a reasonable size
    char* chars     = malloc(buf_size);

    strcpy(chars, "[");
    for (int i = 0; i < seq->items.count; i++) {
      // Use the default to-string method of the value to convert the item to a string
      push(seq->items.values[i]);
      ObjString* item_str = AS_STRING(exec_method(copy_string("str", 3), 0));  // Convert to string

      // Expand chars to fit the separator plus the next item
      size_t new_buf_size = strlen(chars) + strlen(item_str->chars) + sizeof(SEQ_SEPARATOR) +
                            2;  // +2 so we are able to add the closing bracket if we're done
                                // after this iteration.
      if (new_buf_size > buf_size) {
        buf_size = new_buf_size;
        chars    = realloc(chars, buf_size);
        if (chars == NULL) {
          return OBJ_VAL(copy_string("[???]", 5));
        }
      }

      strcat(chars, item_str->chars);
      if (i < seq->items.count - 1) {
        strcat(chars, SEQ_SEPARATOR);
      }
    }

    strcat(chars, "]");

    // Intuitively, you'd expect to use take_string here, but we don't know where malloc
    // allocates the memory - we don't want this block in our own memory pool.
    ObjString* str_obj = copy_string(chars, buf_size);
    free(chars);
    return OBJ_VAL(str_obj);
  }

  runtime_error("Expected a sequence but got %s.", type_name(argv[0]));
  return exit_with_runtime_error();
#undef SEQ_SEPARATOR
}

// Built-in function to retrieve the length of a string
static Value __builtin_str_len(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  int length = AS_STRING(argv[0])->length;
  return NUMBER_VAL(length);
}

// Built-in function to retrieve the length of a sequence
static Value __builtin_seq_len(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  int length = AS_SEQ(argv[0])->items.count;
  return NUMBER_VAL(length);
}

// Binds a method to an instance by creating a new bound method object and
// pushing it onto the stack.
static bool bind_method(ObjClass* klass, ObjString* name) {
  Value method;
  if (!hashtable_get(&klass->methods, OBJ_VAL(name), &method)) {
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
        Value index    = pop();
        Value assignee = pop();

        if (!IS_NUMBER(index)) {
          push(NIL_VAL);
          break;
        }

        double i_raw = AS_NUMBER(index);
        int i;
        if (!is_int(i_raw, &i)) {
          push(NIL_VAL);
          break;
        }

        if (IS_STRING(assignee)) {
          ObjString* string = AS_STRING(assignee);
          if (i < 0 || i >= string->length) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          ObjString* char_str = copy_string(AS_CSTRING(assignee) + i, 1);
          push(OBJ_VAL(char_str));
          break;
        } else if (IS_SEQ(assignee)) {
          ObjSeq* seq = AS_SEQ(assignee);
          if (i < 0 || i >= seq->items.count) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          push(seq->items.values[i]);
        } else {
          runtime_error("%s cannot be get-indexed.", type_name(assignee));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SET_INDEX: {
        Value value = pop();  // We should peek, because assignment is an expression! But,
                              // the order is wrong so we push it back on the stack later.
                              // We are being careful not to trigger a GC in this block,
                              // because value might get collected if we do.
        Value index    = pop();
        Value assignee = pop();

        if (!IS_NUMBER(index)) {
          push(NIL_VAL);
          break;
        }

        double i_raw = AS_NUMBER(index);
        int i;
        if (!is_int(i_raw, &i)) {
          push(NIL_VAL);
          break;
        }

        if (IS_SEQ(assignee)) {
          ObjSeq* seq = AS_SEQ(assignee);

          if (i < 0 || i >= seq->items.count) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          seq->items.values[i] = value;
          push(value);
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
              case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(peek(0));
                if (values_equal(OBJ_VAL(name), vm.reserved_method_names[METHOD_NAME])) {
                  pop();  // Pop the class
                  push(OBJ_VAL(klass->name));
                  goto done_getting_property;
                } else if (values_equal(OBJ_VAL(name), vm.reserved_method_names[METHOD_CTOR])) {
                  pop();  // Pop the class, should be fine since we don't trigger the GC in this
                          // block
                  Value ctor;
                  hashtable_get(&klass->methods, OBJ_VAL(name), &ctor);
                  push(ctor);
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
                } else if (bind_method(instance->klass, name)) {
                  goto done_getting_property;
                }
                break;
              }
            }
            break;
          }
        }

        // If we get here, check if it is a method on the origin object class
        Value value;
        if (hashtable_get(&vm.object_class->methods, OBJ_VAL(name), &value)) {
          pop();  // Pop the object
          push(value);
          goto done_getting_property;
        }

        runtime_error("Property '%s' does not exist on type %s.", name->chars, type_name(peek(0)));
        return exit_with_runtime_error();

      done_getting_property:
        break;
      }
      case OP_SET_PROPERTY: {
        if (!IS_INSTANCE(peek(1))) {
          runtime_error("%s cannot have fields.", type_name(peek(1)));
          return exit_with_runtime_error();
        }

        ObjInstance* instance = AS_INSTANCE(peek(1));
        ObjString* name       = READ_STRING();
        hashtable_set(&instance->fields, OBJ_VAL(name), peek(0));  // Create or update
        Value value = pop();
        pop();
        push(value);
        break;
      }
      case OP_IMPORT: {
        ObjString* name = READ_STRING();
        Value module;

        // Check if we have already imported the module
        if (hashtable_get(&vm.modules, OBJ_VAL(name), &module)) {
          push(module);
          break;
        }

        // Try to open the module instead
        char tmp[256];
        if (sprintf(tmp, "C:\\Projects\\slang\\modules\\%s.sl", name->chars) < 0) {
          runtime_error("Could not import module '%s.sl'. Could not format string", name->chars);
          return exit_with_runtime_error();
        }

        int previous_exit_frame = vm.exit_on_frame;
        vm.exit_on_frame        = vm.frame_count;
        module                  = run_file(tmp, 1 /* new scope */);
        vm.exit_on_frame        = previous_exit_frame;

        if (!IS_OBJ(module) && !IS_INSTANCE(module) && !AS_INSTANCE(module)->klass == vm.module_class) {
          runtime_error("Could not import module '%s'. Expected module type", name->chars);
          return exit_with_runtime_error();
        }
        push(module);  // Show ourselves to the GC before we put it in the hashtable
        hashtable_set(&vm.modules, OBJ_VAL(name), module);
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
        ObjString* str = to_str(peek(0));
        printf("%s\n", str->chars);
        pop();
        break;
      }
      case OP_LIST_LITERAL: {
        int count = READ_ONE();
        make_seq(count);
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
          return pop();
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
        push(OBJ_VAL(new_class(READ_STRING(), vm.object_class)));
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

static void start_module() {
  ObjInstance* module = new_instance(vm.module_class);
  vm.module           = (Obj*)module;

  define_obj(&module->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);
}

Value interpret(const char* source, bool is_module) {
  Obj* enclosing_module = vm.module;
  if (is_module) {
    start_module();
  }

  ObjFunction* function = compile(source, is_module);
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

static char* read_file(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot open NULL path \"%s\"", path);
    exit(74);
  }

  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    INTERNAL_ERROR("Could not open file \"%s\"", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    INTERNAL_ERROR("Not enough memory to read \"%s\"\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    INTERNAL_ERROR("Could not read file \"%s\"\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

Value run_file(const char* path, bool is_module) {
#ifdef DEBUG_TRACE_EXECUTION
  printf(ANSI_MAGENTA_STR("===== "));
  printf(ANSI_CYAN_STR("Running file: %s, Is module: %s\n"), path, is_module ? "true" : "false");
#endif

  char* source = read_file(path);

  if (source == NULL) {
    free(source);
    return NIL_VAL;
  }

  Value result = interpret(source, is_module);
  free(source);

#ifdef DEBUG_TRACE_EXECUTION
  printf(ANSI_MAGENTA_STR("===== "));
  printf(ANSI_CYAN_STR("Done running file: %s\n"), path);
#endif

  return result;
}
