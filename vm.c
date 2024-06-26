#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static Value run();
static Value peek(int distance);

static void reset_stack() {
  vm.stack_top     = vm.stack;
  vm.frame_count   = 0;
  vm.open_upvalues = NULL;
  vm.current_error = NIL_VAL;

  vm.flags &= ~VM_FLAG_HAS_ERROR;  // Clear the error flag
  vm.flags &= ~VM_FLAG_PAUSE_GC;   // Clear the pause flag
}

static void dump_location() {
  CallFrame* frame      = &vm.frames[vm.frame_count - 1];
  ObjFunction* function = frame->closure->function;
  size_t instruction    = frame->ip - function->chunk.code - 1;

  SourceView source = function->chunk.source_views[instruction];

  const char* error_end   = source.start + source.error_end_ofs;
  const char* error_start = source.start + source.error_start_ofs;

  fprintf(stderr, "\n %5d | ", source.line);

  // Print the source code line
  for (const char* c = source.start; c < error_end || (c >= error_end && *c != '\n' && *c != '\0'); c++) {
    if (*c == '\r') {
      continue;
    }

    if (*c == '\n') {
      fputs("...", stderr);
      break;
    } else if (*c == '/' && c[1] == '/') {
      break;  // Break if we reach a line comment
    } else {
      fputc(*c, stderr);
    }
  }

  // Newline and padding
  fputs("\n         ", stderr);
  for (const char* c = source.start; c < error_start; c++) {
    fputc(' ', stderr);
  }

  // Print the squiggly line
  fputs(ANSI_COLOR_RED, stderr);
  for (const char* c = error_start; c < error_end; c++) {
    if (*c == '\r') {
      continue;
    }

    if (*c == '\n') {
      break;
    } else {
      fputc('~', stderr);
    }
  }
  fputs(ANSI_COLOR_RESET, stderr);

  // Done!
  fputs("\n\n", stderr);
}

static void dump_stacktrace() {
  for (int i = vm.frame_count - 1; i >= 0; i--) {
    CallFrame* frame      = &vm.frames[i];
    ObjFunction* function = frame->closure->function;
    size_t instruction    = frame->ip - function->chunk.code - 1;

    fprintf(stderr, "  at line %d ", function->chunk.source_views[instruction].line);

    Value module_name;
    if (!hashtable_get_by_string(&function->globals_context->fields, vm.special_prop_names[SPECIAL_PROP_MODULE_NAME],
                                 &module_name)) {
      fprintf(stderr, "in \"%s\"\n", function->name->chars);
      break;
    }

    // If the module_name is the same ref as the current frame's function-name, then we know it's the
    // toplevel, because that's how we initialize it in the compiler.
    if (AS_STRING(module_name) == function->name) {
      fprintf(stderr, "at the toplevel of module \"%s\"\n", AS_CSTRING(module_name));
    } else {
      fprintf(stderr, "in \"%s\" in module \"%s\"\n", function->name->chars, AS_CSTRING(module_name));
    }
  }

  reset_stack();
}

void runtime_error(const char* format, ...) {
  char buffer[1024] = {0};

  va_list args;
  va_start(args, format);
  size_t length = vsnprintf(buffer, 1024, format, args);
  va_end(args);

  vm.flags |= VM_FLAG_HAS_ERROR;
  vm.current_error = OBJ_VAL(copy_string(buffer, (int)length));
}

// This is just a plcaeholder to find where we actually throw compile errors.
// I want another way to do this, but I don't know how yet.
static void exit_with_compile_error() {
  free_vm();
  // TODO (recovery): Find a way to progpagate errors */
  exit(ECOMPILE_ERROR);
}

void define_native(HashTable* table, const char* name, NativeFn function, const char* doc, int arity) {
  Value key           = OBJ_VAL(copy_string(name, (int)strlen(name)));
  ObjString* doc_str  = copy_string(doc, (int)strlen(doc));
  ObjString* name_str = copy_string(name, (int)strlen(name));
  Value value         = OBJ_VAL(new_native(function, name_str, doc_str, arity));

  push(key);
  push(value);
  push(OBJ_VAL(doc_str));
  push(OBJ_VAL(name_str));
  hashtable_set(table, key, value);
  pop();
  pop();
  pop();
  pop();
}

void define_obj(HashTable* table, const char* name, Obj* obj) {
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

void make_seq(int count) {
  // Since we know the count, we can preallocate the value array for the list. This avoids
  // using write_value_array within the loop, which can trigger a GC due to growing the array
  // and free items in the middle of the loop. Also, it lets us pop the list items on the
  // stack, instead of peeking and then having to pop them later (Requiring us to loop over
  // the array twice)
  ValueArray items = prealloc_value_array(count);
  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = pop();
  }
  push(OBJ_VAL(take_seq(&items)));
}

void make_tuple(int count) {
  // Since we know the count, we can preallocate the value array for the tuple. This avoids
  // using write_value_array within the loop, which can trigger a GC due to growing the array
  // and free items in the middle of the loop. Also, it lets us pop the tuple items on the
  // stack, instead of peeking and then having to pop them later (Requiring us to loop over
  // the array twice)
  ValueArray items = prealloc_value_array(count);

  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = pop();
  }
  push(OBJ_VAL(take_tuple(&items)));
}

// Creates an object from the top "count" * 2 values on the stack.
// The resulting object is pushed onto the stack.
static void make_object(int count) {
  // Since we know the count, we can preallocate the hashtable for the object. This allows
  // using hashtable_set within the loop. We don't have to worry about it wanting to resize the hashtable,
  // which can trigger a GC and free items in the middle of the loop, because it already has enough capacity.
  // Also, it lets us pop the object items on the stack, instead of peeking and then having to pop them later
  // (Requiring us to loop over the keys and values twice)
  HashTable entries;
  init_hashtable(&entries);
  hashtable_preallocate(&entries, count);

  // Take the hashtable while before we start popping the stack, so the entries are still seen by the GC as
  // we allocate the new object.
  ObjObject* obj = take_object(&entries);

  // Use pop
  for (int i = 0; i < count; i++) {
    Value value = pop();
    Value key   = pop();

    hashtable_set(&obj->fields, key, value);
  }

  push(OBJ_VAL(obj));
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
  vm.exit_on_frame   = 0;  // Default to exit on the first frame

  // Pause while we initialize the vm.
  vm.flags |= VM_FLAG_PAUSE_GC;

  init_hashtable(&vm.strings);
  init_hashtable(&vm.modules);

  // Build the reserved words lookup table
  memset(vm.special_method_names, 0, sizeof(vm.special_method_names));
  vm.special_method_names[SPECIAL_METHOD_CTOR]   = copy_string(STR(SP_METHOD_CTOR), STR_LEN(STR(SP_METHOD_CTOR)));
  vm.special_method_names[SPECIAL_METHOD_TO_STR] = copy_string(STR(SP_METHOD_TO_STR), STR_LEN(STR(SP_METHOD_TO_STR)));
  vm.special_method_names[SPECIAL_METHOD_HAS]    = copy_string(STR(SP_METHOD_HAS), STR_LEN(STR(SP_METHOD_HAS)));
  vm.special_method_names[SPECIAL_METHOD_SLICE]  = copy_string(STR(SP_METHOD_SLICE), STR_LEN(STR(SP_METHOD_SLICE)));

  memset(vm.special_prop_names, 0, sizeof(vm.special_prop_names));
  vm.special_prop_names[SPECIAL_PROP_LEN]         = copy_string(STR(SP_PROP_LEN), STR_LEN(STR(SP_PROP_LEN)));
  vm.special_prop_names[SPECIAL_PROP_NAME]        = copy_string(STR(SP_PROP_NAME), STR_LEN(STR(SP_PROP_NAME)));
  vm.special_prop_names[SPECIAL_PROP_DOC]         = copy_string(STR(SP_PROP_DOC), STR_LEN(STR(SP_PROP_DOC)));
  vm.special_prop_names[SPECIAL_PROP_FILE_PATH]   = copy_string(STR(SP_PROP_FILE_PATH), STR_LEN(STR(SP_PROP_FILE_PATH)));
  vm.special_prop_names[SPECIAL_PROP_MODULE_NAME] = copy_string(STR(SP_PROP_MODULE_NAME), STR_LEN(STR(SP_PROP_MODULE_NAME)));

  // Register the built-in classes, starting with the obj class, which is the base class for all objects.
  register_builtin_obj_class();

  // Register the built-in functions
  register_builtin_functions();

  // Register the built-in classes
  register_builtin_nil_class();
  register_builtin_bool_class();
  register_builtin_num_class();
  register_builtin_int_class();
  register_builtin_float_class();
  register_builtin_seq_class();
  register_builtin_tuple_class();
  register_builtin_str_class();
  register_builtin_fn_class();
  register_builtin_class_class();

  // Create the module class
  BUILTIN_REGISTER_CLASS(TYPENAME_MODULE, TYPENAME_OBJ);
  BUILTIN_FINALIZE_CLASS(TYPENAME_MODULE);

  // Register built-in modules
  register_builtin_file_module();
  register_builtin_perf_module();
  register_builtin_debug_module();

  vm.flags &= ~VM_FLAG_PAUSE_GC;  // Unpause

  reset_stack();
}

void free_vm() {
  free_hashtable(&vm.strings);
  free_hashtable(&vm.modules);
  memset(vm.special_method_names, 0, sizeof(vm.special_method_names));
  memset(vm.special_prop_names, 0, sizeof(vm.special_prop_names));
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

ObjClass* typeof_(Value value) {
  // Handle primitive and internal types which have a corresponding class
  switch (value.type) {
    case VAL_NIL: return vm.__builtin_Nil_class;      // This field name was created via macro
    case VAL_BOOL: return vm.__builtin_Bool_class;    // This field name was created via macro
    case VAL_INT: return vm.__builtin_Int_class;      // This field name was created via macro
    case VAL_FLOAT: return vm.__builtin_Float_class;  // This field name was created via macro
    case VAL_OBJ: {
      switch (OBJ_TYPE(value)) {
        case OBJ_STRING: return vm.__builtin_Str_class;   // This field name was created via macro
        case OBJ_SEQ: return vm.__builtin_Seq_class;      // This field name was created via macro
        case OBJ_TUPLE: return vm.__builtin_Tuple_class;  // This field name was created via macro
        case OBJ_NATIVE:
        case OBJ_CLOSURE:
        case OBJ_BOUND_METHOD:
        case OBJ_FUNCTION: return vm.__builtin_Fn_class;  // This field name was created via macro
        case OBJ_CLASS: return vm.__builtin_Class_class;  // This field name was created via macro
        case OBJ_OBJECT:
          return AS_OBJECT(value)->klass;  // Class X, if it's an instance. Or just vm.__builtin_Obj_class, if it's an object.
        default: break;
      }
    }
    default: break;
  }
  return vm.__builtin_Obj_class;  // This field name was created via macro
}

// If [expected] is positive, [actual] must match exactly. If [expected] is negative, [actual] must be at least
// the absolute value of [expected].
#define CHECK_ARGS(expected, actual)                                                  \
  if (expected >= 0 ? actual != expected : actual < -expected) {                      \
    if (expected == 1) {                                                              \
      runtime_error("Expected 1 argument but got %d.", actual);                       \
    } else if (expected == -1) {                                                      \
      runtime_error("Expected at least 1 argument but got %d.", actual);              \
    } else if (expected < -1) {                                                       \
      runtime_error("Expected at least %d arguments but got %d.", -expected, actual); \
    } else {                                                                          \
      runtime_error("Expected %d arguments but got %d.", expected, actual);           \
    }                                                                                 \
    return CALL_FAILED;                                                               \
  }

// Executes a call to a managed-code function or method by creating a new call frame and pushing it onto the
// frame stack.
// `Stack: ...[closure][arg0][arg1]...[argN]`
static CallResult call_managed(ObjClosure* closure, int arg_count) {
  CHECK_ARGS(closure->function->arity, arg_count);

  if (vm.frame_count == FRAMES_MAX) {
    runtime_error("Stack overflow.");
    return CALL_FAILED;
  }

  CallFrame* frame = &vm.frames[vm.frame_count++];
  frame->closure   = closure;
  frame->ip        = closure->function->chunk.code;
  frame->slots = vm.stack_top - arg_count - 1;  // -1 to account for either the function or the receiver preceeding the arguments.
  frame->globals = &closure->function->globals_context->fields;

  return CALL_RUNNING;
}

// Calls a native function with the given number of arguments (on the stack).
// `Stack: ...[native|receiver][arg0][arg1]...[argN]`
static CallResult call_native(ObjNative* native, int arg_count) {
  CHECK_ARGS(native->arity, arg_count);

  Value* args  = vm.stack_top - arg_count - 1;
  Value result = native->function(arg_count, args);
  vm.stack_top -= arg_count + 1;  // Remove args + fn or receiver
  push(result);

  return CALL_RETURNED;
}

#undef CHECK_ARGS

// Calls a callable managed or native value (function, method, class (ctor), etc.) with the given number of
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
          case OBJ_NATIVE: return call_native((ObjNative*)bound->method, arg_count);
          default: break;  // Non-callable object type.
        }
        break;
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
        if (hashtable_get_by_string(&klass->methods, vm.special_method_names[SPECIAL_METHOD_CTOR], &ctor)) {
          switch (AS_OBJ(ctor)->type) {
            case OBJ_CLOSURE: return call_managed(AS_CLOSURE(ctor), arg_count);
            case OBJ_NATIVE: return call_native(AS_NATIVE(ctor), arg_count);
            default: {
              runtime_error("Cannot invoke ctor of type %s", typeof_(ctor)->name->chars);
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

  runtime_error("Attempted to call non-callable value of type %s.", typeof_(callable)->name->chars);
  return CALL_FAILED;
}

// Invokes a method on a class by looking up the method in the class' method
// table and calling it. (Combines "get-property" and "call" opcodes)
// `Stack: ...[receiver][arg0][arg1]...[argN]`
static CallResult invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) {
  Value method;
  if (!hashtable_get_by_string(&klass->methods, name, &method)) {
    bool is_base = klass->base == NULL;
    runtime_error("Undefined method '%s' in type %s%s.", name->chars, klass->name->chars,
                  is_base ? "" : " or any of its parent classes");
    return CALL_FAILED;
  }

  switch (AS_OBJ(method)->type) {
    case OBJ_CLOSURE: return call_managed(AS_CLOSURE(method), arg_count);
    case OBJ_NATIVE: return call_native(AS_NATIVE(method), arg_count);
    default: {
      runtime_error("Cannot invoke method of type %s on class", typeof_(method)->name->chars);
      return CALL_FAILED;
    }
  }
}

// Invokes a managed-code or native method with receiver and arguments on the top of the stack.
// `Stack: ...[receiver][arg0][arg1]...[argN]`
static CallResult invoke(ObjString* name, int arg_count) {
  Value receiver = peek(arg_count);

  // Handle static methods on classes
  Value static_method;
  if (IS_CLASS(receiver)) {
    ObjClass* klass = AS_CLASS(receiver);
    if (hashtable_get_by_string(&klass->static_methods, name, &static_method)) {
      vm.stack_top[-arg_count - 1] = static_method;
      return call_value(static_method, arg_count);
    }
  }

  // Handle primitives which have a corresponding class.
  // Everything which is not an object
  if (!IS_OBJECT(receiver)) {
    ObjClass* klass = typeof_(receiver);
    return invoke_from_class(klass, name, arg_count);
  }

  // Handle object (could also be class instances)
  ObjObject* object = AS_OBJECT(receiver);

  // It could be a field which is a function, we need to check that first
  Value function;
  if (hashtable_get_by_string(&object->fields, name, &function)) {
    vm.stack_top[-arg_count - 1] = function;
    return call_value(function, arg_count);
  }

  // Otherwise, it's just a class method
  return invoke_from_class(object->klass, name, arg_count);
}

// Executes a callframe by running the bytecode until it returns a value or an error occurs.
static Value run_frame() {
  int previous_exit_frame = vm.exit_on_frame;
  vm.exit_on_frame        = vm.frame_count - 1;

  Value result = run();

  vm.exit_on_frame = previous_exit_frame;
  return result;
}

Value exec_callable(Obj* callable, int arg_count) {
  CallResult result = CALL_FAILED;
  if (callable->type == OBJ_STRING) {
    Value receiver  = peek(arg_count);
    ObjClass* klass = typeof_(receiver);
    result          = invoke_from_class(klass, (ObjString*)callable, arg_count);
  } else {
    result = call_value(OBJ_VAL(callable), arg_count);
  }

  if (result == CALL_RETURNED) {
    return pop();
  } else if (result == CALL_RUNNING) {
    return run_frame();
  }

  return NIL_VAL;
}

// Binds a method to an instance by creating a new bound method object from the instance and the method name.
// The stack is unchanged.
static bool bind_method(ObjClass* klass, ObjString* name, Value* bound_method) {
  Value method;
  if (!hashtable_get_by_string(&klass->methods, name, &method)) {
    *bound_method = NIL_VAL;
    return false;
  }

  ObjBoundMethod* bound = new_bound_method(peek(0), AS_OBJ(method));
  *bound_method         = OBJ_VAL(bound);
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
static void define_method(ObjString* name, FunctionType type) {
  Value method    = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));  // We trust the compiler that this value
                                        // is actually a class

  switch (type) {
    case TYPE_METHOD_STATIC: hashtable_set(&klass->static_methods, OBJ_VAL(name), method); break;
    case TYPE_CONSTRUCTOR:
    case TYPE_METHOD: {
      hashtable_set(&klass->methods, OBJ_VAL(name), method);
      break;
    }
    default: {
      INTERNAL_ERROR("Unknown method FunctionType %d", type);
      exit(ESW_ERROR);
    }
  }

  pop();
}

bool is_falsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

// Imports a module by name and pushes it onto the stack. If the module was already imported, it is loaded
// from cache. If the module was not imported yet, it is loaded from the file system and then cached.
// If module_path is NULL, the module is expected to be in the same directory as the
// importing module. Returns true if the module was successfully imported, false otherwise.
static bool import_module(ObjString* module_name, ObjString* module_path) {
  Value module;

  // Check if we have already imported the module
  if (hashtable_get_by_string(&vm.modules, module_name, &module)) {
    push(module);
    return true;
  }

  // Not cached, so we need to load it. First, we need to get the current working directory
  Value cwd = BUILTIN_FN(cwd)(0, NULL);
  if (IS_NIL(cwd)) {
    runtime_error(
        "Could not import module '%s'. Could not get current working directory, because there is no "
        "active module or it is not a file.",
        module_name->chars);
    return false;
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
      if (module_to_load_path == NULL) {
        runtime_error("Could not import module '%s'. Out of memory.", module_name->chars);
        return false;
      }

      strcpy(module_to_load_path, module_path->chars);
    }
  }

  if (module_to_load_path == NULL) {
    INTERNAL_ERROR(
        "Could not produce a valid module path for module '%s'. Cwd is '%s', additional path is "
        "'%s'",
        module_name->chars, AS_STRING(cwd)->chars, module_path == NULL ? "NULL" : module_path->chars);
    exit(EIO_ERROR);
  }

  if (!file_exists(module_to_load_path)) {
    runtime_error("Could not import module '%s'. File '%s' does not exist.", module_name->chars, module_to_load_path);
    return false;
  }

  // Load the module by running the file
  int previous_exit_frame = vm.exit_on_frame;
  vm.exit_on_frame        = vm.frame_count;
  module                  = run_file(module_to_load_path, module_name->chars);
  vm.exit_on_frame        = previous_exit_frame;

  // Check if the module is actually a module
  if (!IS_OBJ(module) && !IS_OBJECT(module) && !(AS_OBJECT(module)->klass == vm.__builtin_Module_class)) {
    free(module_to_load_path);
    runtime_error("Could not import module '%s'. Expected module type", module_name->chars);
    return false;
  }

  push(module);  // Show ourselves to the GC before we put it in the hashtable
  hashtable_set(&vm.modules, OBJ_VAL(module_name), module);

  free(module_to_load_path);
  return true;
}

// Concatenates two strings on the stack (pops them) into a new string and pushes it onto the stack
// `Stack: ...[a][b]` → `Stack: ...[a+b]`
static void concatenate() {
  ObjString* b = AS_STRING(peek(0));  // Peek, so it doesn't get freed by the GC
  ObjString* a = AS_STRING(peek(1));  // Peek, so it doesn't get freed by the GC

  int length  = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = take_string(chars, length);
  pop();  // b
  pop();  // a
  push(OBJ_VAL(result));
}

bool value_get_property(ObjString* name) {
  Value receiver = peek(0);
  Value result;

  // Primitive types have no properties, but you can still access the methods of their class.
  if (IS_OBJ(receiver)) {
    switch (AS_OBJ(receiver)->type) {
      case OBJ_OBJECT: {
        // Can be an instance or a plain object
        ObjObject* object = AS_OBJECT(receiver);
        if (hashtable_get_by_string(&object->fields, name, &result)) {
          goto done_getting_property;
        }
        if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {
          result = INT_VAL(object->fields.count);
          goto done_getting_property;
        }
        break;
      }
      case OBJ_CLASS: {
        ObjClass* klass = AS_CLASS(receiver);
        if (hashtable_get_by_string(&klass->static_methods, name, &result)) {
          goto done_getting_property;
        }
        // You can also get a constructor, unbound
        if (name == vm.special_method_names[SPECIAL_METHOD_CTOR]) {
          if (!hashtable_get_by_string(&klass->methods, name, &result)) {
            result = NIL_VAL;
          }
          goto done_getting_property;
        }
        if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
          result = OBJ_VAL(klass->name);
          goto done_getting_property;
        }
        break;
      }
      case OBJ_FUNCTION:
      case OBJ_CLOSURE:
      case OBJ_NATIVE:
      case OBJ_BOUND_METHOD: {
        if (name == vm.special_prop_names[SPECIAL_PROP_NAME]) {
          result = OBJ_VAL(fn_get_name((Obj*)AS_OBJECT(receiver)));
          goto done_getting_property;
        }
        break;
      }
      case OBJ_TUPLE:
      case OBJ_SEQ: {
        if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {
          ValueArray items = LISTLIKE_GET_VALUEARRAY(receiver);
          result           = INT_VAL(items.count);
          goto done_getting_property;
        }
        break;
      }
      case OBJ_STRING: {
        if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {
          ObjString* string = AS_STRING(receiver);
          result            = INT_VAL(string->length);
          goto done_getting_property;
        }
        break;
      }
      default: break;
    }
  }

  // For every value, you can access it's classes methods
  if (bind_method(typeof_(receiver), name, &result)) {
    goto done_getting_property;
  }

  return false;

done_getting_property:
  pop();  // Pop receiver
  push(result);
  return true;
}

bool value_set_property(ObjString* name) {
  Value receiver = peek(1);
  Value value    = peek(0);

  // TODO (optimize): Maybe remove this check for reserved words. We could just allow the user to override these things. This is
  // tightly bound to the retrieval of properties, because if we check the special props before the fields, they would always
  // return the internal value. Currently tough, we check the fields first, because most of the time you probably want to get
  // fields you've defined as a user. So, getting a special property would yield the value the user assigned. This is bad, because
  // you could override e.g. the length property and cause the vm to segfault.

  // Check if it is a reserved property.
  for (int i = 0; i < SPECIAL_PROP_MAX; i++) {
    if (name == vm.special_prop_names[i]) {  // We can just compare pointers, because strings are interned.
      runtime_error("Cannot set reserved property '%s' on value of type %s.", name->chars, typeof_(receiver)->name->chars);
      return false;
    }
  }

  if (IS_OBJECT(receiver)) {
    ObjObject* object = AS_OBJECT(receiver);
    hashtable_set(&object->fields, OBJ_VAL(name), value);
    goto done_setting_property;
  }

  return false;

done_setting_property:
  pop();        // Pop value
  pop();        // Pop receiver
  push(value);  // Push the value back onto the stack, because assignment is an expression
  return true;
}

bool value_get_index() {
  Value receiver = peek(1);
  Value index    = peek(0);
  Value result;

  if (IS_OBJ(receiver)) {
    switch (AS_OBJ(receiver)->type) {
      case OBJ_OBJECT: {
        // Objects return nil for non-existing indices or the value at the index
        ObjObject* object = AS_OBJECT(receiver);
        if (hashtable_get(&object->fields, index, &result)) {
          goto done_getting_index;
        }
        result = NIL_VAL;
        goto done_getting_index;
      }
      case OBJ_TUPLE:
      case OBJ_SEQ: {
        // Hack: Just cast to a sequence, because ObjTuple has the same layout.
        ValueArray items = LISTLIKE_GET_VALUEARRAY(receiver);

        if (!IS_INT(index)) {
          break;  // Maybe it's a string index
        }

        long long i = AS_INT(index);
        if (i >= items.count) {
          result = NIL_VAL;
          goto done_getting_index;
        }

        // Negative index
        if (i < 0) {
          i += items.count;
        }
        if (i < 0) {
          result = NIL_VAL;
          goto done_getting_index;
        }

        result = items.values[i];
        goto done_getting_index;
      }
      case OBJ_STRING: {
        ObjString* string = AS_STRING(receiver);

        if (!IS_INT(index)) {
          break;  // Maybe it's a string index
        }

        long long i = AS_INT(index);
        if (i >= string->length) {
          result = NIL_VAL;
          goto done_getting_index;
        }

        // Negative index
        if (i < 0) {
          i += string->length;
        }
        if (i < 0) {
          result = NIL_VAL;
          goto done_getting_index;
        }

        ObjString* char_str = copy_string(string->chars + i, 1);
        result              = OBJ_VAL(char_str);
        goto done_getting_index;
      }
      default: break;
    }
  }

  return false;

done_getting_index:
  pop();  // Pop receiver (or index, if it got swapped)
  pop();  // Pop index (or receiver, if it got swapped)
  push(result);
  return true;
}

bool value_set_index() {
  Value receiver = peek(2);
  Value index    = peek(1);
  Value value    = peek(0);

  if (IS_OBJ(receiver)) {
    switch (AS_OBJ(receiver)->type) {
      case OBJ_OBJECT: {
        ObjObject* object = AS_OBJECT(receiver);
        hashtable_set(&object->fields, index, value);
        goto done_setting_index;
      }
      case OBJ_SEQ: {
        if (!IS_INT(index)) {
          runtime_error(STR(TYPENAME_SEQ) " indices must be " STR(TYPENAME_INT) "s, but got %s.", typeof_(index)->name->chars);
          return false;
        }

        long long i = AS_INT(index);
        ObjSeq* seq = AS_SEQ(receiver);

        if (i < 0 || i >= seq->items.count) {
          runtime_error("Index out of bounds. Was %d, but this " STR(TYPENAME_SEQ) " has length %d.", i, seq->items.count);
          return false;
        }

        seq->items.values[i] = value;
        goto done_setting_index;
      }
      default: break;
    }
  }

  return false;

done_setting_index:
  pop();        // Pop value
  pop();        // Pop index
  pop();        // Pop receiver
  push(value);  // Push the value back onto the stack, because assignment is an expression
  return true;
}

// Handles errors in the virtual machine.
// This function is responsible for handling errors that occur during the execution of the virtual machine.
// It searches for the nearest error handler in the call stack and resets the virtual machine state to that
// handler's call frame.
// If no error handler is found, it prints the error message, dumps the stack trace, resets the virtual
// machine's stack state and returns false. Returns true if an error handler is found and the virtual machine
// state is reset, false otherwise.
static bool handle_error() {
  int stack_offset;
  int frame_offset;
  int exit_slot = 0;  // Slot in the stack where we should stop looking for a handler

  // If we have an exit frame, we need to use it's stack-start slot as the exit slot
  // This ensures that we stay within the current frame's stack, and don't go beyond it.
  if ((vm.exit_on_frame >= 0)) {
    exit_slot = (int)(vm.frames[vm.exit_on_frame].slots - vm.stack);
  }

  // Rewind the stack to the last handler, or the exit slot
  for (stack_offset = (int)(vm.stack_top - vm.stack - 1);                 // Start at the top of the stack
       stack_offset >= exit_slot && !IS_HANDLER(vm.stack[stack_offset]);  // Stop at the exit slot or handler
       stack_offset--)
    ;

  // Did we find a handler within the current frame?
  if (stack_offset < exit_slot) {
    // No handler found and we reached the bottom of the stack. So we print the stacktrace and reset the Vm's
    // stack state. Should be fine to exectute more code after this, because the stack is reset.
    if (exit_slot == 0) {
      fprintf(stderr, "Uncaught error: ");
      print_value_safe(stderr, vm.current_error);
      fprintf(stderr, "\n");

      dump_location();
      dump_stacktrace();
      reset_stack();
      vm.frame_count = 0;
    }
    return false;
  }

  // We have a handler, now we need to find the frame it belongs to.
  // We do that by going through the frames from the top to the bottom, and stop at the frame where the
  // handler (stack_offset) is.
  for (frame_offset = vm.frame_count - 1;  // Start at the top of the frame stack
       frame_offset >= 0 && (int)(vm.frames[frame_offset].slots - vm.stack) > stack_offset;  // Stop at the frame
       frame_offset--)
    ;

  // Did we find a frame that owns the handler?
  if (frame_offset == -1) {
    INTERNAL_ERROR("Call stack corrupted. No call frame found that owns the handler.");
    exit(ESW_ERROR);
  }

  // We have the handler's index/offset in the stack and the frame it belongs to. Now let's reset the Vm to
  // that call frame.
  close_upvalues(&vm.stack[stack_offset]);  // Close upvalues that are no longer needed
  vm.stack_top   = vm.stack + stack_offset + 1;
  vm.frame_count = frame_offset + 1;
  vm.flags &= ~VM_FLAG_HAS_ERROR;

  return true;
}

// This function represents the main loop of the virtual machine. It fetches the next instruction, decodes it,
// and dispatches it
static Value run() {
  CallFrame* frame = &vm.frames[vm.frame_count - 1];

// Read a single piece of data from the current instruction pointer and advance it
#define READ_ONE() (*frame->ip++)

// Read a constant from the constant pool. This consumes one piece of data on the stack, which is the index of
// the constant to read
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_ONE()])

// Read a string from the constant pool.
#define READ_STRING() AS_STRING(READ_CONSTANT())

// Perform a binary operation on the top two values on the stack. This consumes two pieces of data from the
// stack, and pushes the result.
//
// TODO (optimize): These probably have some potential for optimization.
#define MAKE_BINARY_OP(operator, b_check)                                                                                     \
  {                                                                                                                           \
    Value b = pop();                                                                                                          \
    Value a = pop();                                                                                                          \
    if (IS_INT(a) && IS_INT(b)) {                                                                                             \
      b_check;                                                                                                                \
      push(INT_VAL(AS_INT(a) operator AS_INT(b)));                                                                            \
      break;                                                                                                                  \
    }                                                                                                                         \
    if (IS_FLOAT(a)) {                                                                                                        \
      if (IS_INT(b)) {                                                                                                        \
        b_check;                                                                                                              \
        push(FLOAT_VAL(AS_FLOAT(a) operator(double) AS_INT(b)));                                                              \
        break;                                                                                                                \
      } else if (IS_FLOAT(b)) {                                                                                               \
        b_check;                                                                                                              \
        push(FLOAT_VAL(AS_FLOAT(a) operator AS_FLOAT(b)));                                                                    \
        break;                                                                                                                \
      }                                                                                                                       \
    } else if (IS_FLOAT(b)) {                                                                                                 \
      if (IS_INT(a)) {                                                                                                        \
        b_check;                                                                                                              \
        push(FLOAT_VAL((double)AS_INT(a) operator AS_FLOAT(b)));                                                              \
        break;                                                                                                                \
      }                                                                                                                       \
    }                                                                                                                         \
    runtime_error("Incompatible types for binary operand %s. Left was %s, right was %s.", #operator, typeof_(a)->name->chars, \
                  typeof_(b)->name->chars);                                                                                   \
    goto finish_error;                                                                                                        \
  }

#define BIN_ADD MAKE_BINARY_OP(+, (void)0)
#define BIN_SUB MAKE_BINARY_OP(-, (void)0)
#define BIN_MUL MAKE_BINARY_OP(*, (void)0)
#define BIN_DIV                                                                                         \
  MAKE_BINARY_OP(                                                                                       \
      /, if ((b.type == VAL_INT && b.as.integer == 0) || (b.type == VAL_FLOAT && b.as.float_ == 0.0)) { \
          runtime_error("Division by zero.");                                                           \
          goto finish_error;                                                                            \
        })

// Perform a comparison operation on the top two values on the stack. This consumes two pieces of data from the
// stack, and pushes the result.
//
// TODO (optimize): These probably have some potential for optimization.
#define MAKE_COMPARATOR(operator)                                                                        \
  {                                                                                                      \
    Value b = pop();                                                                                     \
    Value a = pop();                                                                                     \
    if (IS_INT(a) && IS_INT(b)) {                                                                        \
      push(BOOL_VAL(AS_INT(a) operator AS_INT(b)));                                                      \
      break;                                                                                             \
    }                                                                                                    \
    if (IS_FLOAT(a)) {                                                                                   \
      if (IS_INT(b)) {                                                                                   \
        push(BOOL_VAL(AS_FLOAT(a) operator AS_INT(b)));                                                  \
        break;                                                                                           \
      } else if (IS_FLOAT(b)) {                                                                          \
        push(BOOL_VAL(AS_FLOAT(a) operator AS_FLOAT(b)));                                                \
        break;                                                                                           \
      }                                                                                                  \
    } else if (IS_FLOAT(b)) {                                                                            \
      if (IS_INT(a)) {                                                                                   \
        push(BOOL_VAL(AS_INT(a) operator AS_INT(b)));                                                    \
        break;                                                                                           \
      }                                                                                                  \
    }                                                                                                    \
    runtime_error("Incompatible types for comparison operand %s. Left was %s, right was %s.", #operator, \
                  typeof_(a)->name->chars, typeof_(b)->name->chars);                                     \
    goto finish_error;                                                                                   \
  }

#define BIN_LT MAKE_COMPARATOR(<)
#define BIN_GT MAKE_COMPARATOR(>)
#define BIN_LTEQ MAKE_COMPARATOR(<=)
#define BIN_GTEQ MAKE_COMPARATOR(>=)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    disassemble_instruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));

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
      case OP_DUPE: push(peek(READ_ONE())); break;
      case OP_GET_LOCAL: {
        uint16_t slot = READ_ONE();
        push(frame->slots[slot]);
        break;
      }
      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!hashtable_get_by_string(frame->globals, name, &value)) {
          if (!hashtable_get_by_string(&vm.builtin->fields, name, &value)) {
            runtime_error("Undefined variable '%s'.", name->chars);
            goto finish_error;
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
        if (!hashtable_set(frame->globals, OBJ_VAL(name), peek(0))) {
          runtime_error("Variable '%s' is already defined.", name->chars);
          goto finish_error;
        }
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
          goto finish_error;
        }
        break;
      }
      case OP_SET_UPVALUE: {
        uint16_t slot                             = READ_ONE();
        *frame->closure->upvalues[slot]->location = peek(0);  // peek, because assignment is an expression!
        break;
      }
      case OP_GET_INDEX: {
        if (value_get_index()) {
          break;
        }
        if (vm.flags & VM_FLAG_HAS_ERROR) {  // Maybe value_get_index set an error
          goto finish_error;
        }
        runtime_error("Type %s does not support get-indexing with %s.", typeof_(peek(1))->name->chars,
                      typeof_(peek(0))->name->chars);
        goto finish_error;
      }
      case OP_SET_INDEX: {
        if (value_set_index()) {
          break;
        }
        if (vm.flags & VM_FLAG_HAS_ERROR) {  // Maybe value_set_index set an error
          goto finish_error;
        }
        runtime_error("Type %s does not support set-indexing with %s.", typeof_(peek(2))->name->chars,
                      typeof_(peek(1))->name->chars);
        goto finish_error;
      }
      case OP_GET_PROPERTY: {
        ObjString* name = READ_STRING();
        if (value_get_property(name)) {
          break;
        }
        if (vm.flags & VM_FLAG_HAS_ERROR) {  // Will never happen: value_get_property never sets an error. Just a precaution.
          goto finish_error;
        }
        runtime_error("Property '%s' does not exist on value of type %s.", name->chars, typeof_(peek(0))->name->chars);
        goto finish_error;
      }
      case OP_SET_PROPERTY: {
        ObjString* name = READ_STRING();
        if (value_set_property(name)) {
          break;
        }
        if (vm.flags & VM_FLAG_HAS_ERROR) {  // Maybe value_set_property set an error
          goto finish_error;
        }
        runtime_error("Type %s does not support property-set access.", typeof_(peek(1))->name->chars);
        goto finish_error;
      }
      case OP_IMPORT_FROM: {
        ObjString* name = READ_STRING();
        ObjString* from = READ_STRING();
        if (!import_module(name, from)) {
          goto finish_error;
        }
        break;
      }
      case OP_IMPORT: {
        ObjString* name = READ_STRING();
        if (!import_module(name, NULL)) {
          goto finish_error;
        }
        break;
      }
      case OP_GET_BASE_METHOD: {
        ObjString* name     = READ_STRING();
        ObjClass* baseclass = AS_CLASS(pop());

        Value bound_method;
        if (!bind_method(baseclass, name, &bound_method)) {
          runtime_error("Method '%s' does not exist in '%s'.", name->chars, baseclass->name->chars);
          goto finish_error;
        }
        pop();
        push(bound_method);
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
      case OP_GT: BIN_GT
      case OP_LT: BIN_LT
      case OP_GTEQ: BIN_GTEQ
      case OP_LTEQ: BIN_LTEQ
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
          break;
        } else
          BIN_ADD
      }
      case OP_SUBTRACT: BIN_SUB
      case OP_MULTIPLY: BIN_MUL
      case OP_DIVIDE: BIN_DIV
      case OP_MODULO: {
        // Pretty much the same as MAKE_BINARY_OP, but expanded bc we use fmod(double, double) for floats
        Value b = pop();
        Value a = pop();

        if (IS_INT(a) && IS_INT(b)) {
          if (AS_INT(b) == 0) {
            runtime_error("Modulo by zero.");
            goto finish_error;
          }
          push(INT_VAL(AS_INT(a) % AS_INT(b)));
          break;
        }

        if (IS_FLOAT(a)) {
          if (IS_INT(b)) {
            if (AS_INT(b) == 0) {
              runtime_error("Modulo by zero.");
              goto finish_error;
            }
            push(FLOAT_VAL(fmod(AS_FLOAT(a), (double)AS_INT(b))));
            break;
          } else if (IS_FLOAT(b)) {
            if (AS_FLOAT(b) == 0) {
              runtime_error("Modulo by zero.");
              goto finish_error;
            }
            push(FLOAT_VAL(fmod(AS_FLOAT(a), AS_FLOAT(b))));
            break;
          }
        } else if (IS_FLOAT(b)) {
          if (IS_INT(a)) {
            if (AS_INT(b) == 0) {
              runtime_error("Modulo by zero.");
              goto finish_error;
            }
            push(FLOAT_VAL(fmod((double)AS_INT(a), AS_FLOAT(b))));
            break;
          }
        }

        runtime_error("Incompatible types for binary operand %s. Left was %s, right was %s.", "%", typeof_(a)->name->chars,
                      typeof_(b)->name->chars);
        goto finish_error;
      }
      case OP_NOT: push(BOOL_VAL(is_falsey(pop()))); break;
      case OP_NEGATE: {
        switch (peek(0).type) {
          case VAL_INT: push(INT_VAL(-AS_INT(pop()))); break;
          case VAL_FLOAT: push(FLOAT_VAL(-AS_FLOAT(pop()))); break;
          default:
            runtime_error("Type for unary - must be a " STR(TYPENAME_NUM) ". Was %s.", typeof_(peek(0))->name->chars);
            goto finish_error;
        }
        break;
      }
      case OP_PRINT: {
        ObjString* str = AS_STRING(exec_callable(typeof_(peek(0))->__to_str, 0));
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          goto finish_error;
        }
        printf("%s\n", str->chars);
        break;
      }
      case OP_SEQ_LITERAL: {
        int count = READ_ONE();
        make_seq(count);
        break;
      }
      case OP_TUPLE_LITERAL: {
        int count = READ_ONE();
        make_tuple(count);
        break;
      }
      case OP_OBJECT_LITERAL: {
        int count = READ_ONE();
        make_object(count);
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
      case OP_TRY: {
        uint16_t try_target = READ_ONE();
        uint16_t offset = frame->ip - frame->closure->function->chunk.code;  // Offset from start of callframe to the try block
        Value handler   = HANDLER_VAL(try_target + offset);
        push(handler);
        break;
      }
      case OP_THROW: {
        vm.current_error = pop();
        vm.flags |= VM_FLAG_HAS_ERROR;
        goto finish_error;
      }
      case OP_CALL: {
        int arg_count = READ_ONE();
        if (call_value(peek(arg_count), arg_count) == CALL_FAILED) {
          if (vm.flags & VM_FLAG_HAS_ERROR) {
            goto finish_error;
          }
          return NIL_VAL;
        }
        frame = &vm.frames[vm.frame_count - 1];  // Set the frame to the current frame
        break;
      }
      case OP_INVOKE: {
        ObjString* method = READ_STRING();
        int arg_count     = READ_ONE();
        if (invoke(method, arg_count) == CALL_FAILED) {
          if (vm.flags & VM_FLAG_HAS_ERROR) {
            goto finish_error;
          }
          return NIL_VAL;
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_BASE_INVOKE: {
        ObjString* method   = READ_STRING();
        int arg_count       = READ_ONE();
        ObjClass* baseclass = AS_CLASS(pop());
        if (invoke_from_class(baseclass, method, arg_count) == CALL_FAILED) {
          if (vm.flags & VM_FLAG_HAS_ERROR) {
            goto finish_error;
          }
          return NIL_VAL;
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
      case OP_CLASS: {
        // Initially, a class always inherits from Obj
        ObjClass* klass = new_class(READ_STRING(), vm.__builtin_Obj_class);
        push(OBJ_VAL(klass));
        hashtable_add_all(&klass->base->methods, &klass->methods);
        break;
      }
      case OP_INHERIT: {
        Value baseclass    = peek(1);
        ObjClass* subclass = AS_CLASS(peek(0));
        if (!IS_CLASS(baseclass)) {
          runtime_error("Base class must be a class. Was %s.", typeof_(baseclass)->name->chars);
          goto finish_error;
        }
        hashtable_add_all(&AS_CLASS(baseclass)->methods, &subclass->methods);
        subclass->base = AS_CLASS(baseclass);
        pop();  // Subclass.
        break;
      }
      case OP_FINALIZE: {
        Value klass = peek(0);
        finalize_new_class(AS_CLASS(klass));  // We trust the compiler that this value is actually a class
        pop();
        break;
      }
      case OP_IS: {
        Value type  = pop();
        Value value = pop();

        if (!IS_CLASS(type)) {
          runtime_error("Type must be a class. Was %s.", typeof_(type)->name->chars);
          goto finish_error;
        }

        ObjClass* value_klass = typeof_(value);
        ObjString* type_name  = AS_CLASS(type)->name;

        bool result = false;
        while (value_klass != NULL) {
          if (strcmp(value_klass->name->chars, type_name->chars) == 0) {
            result = true;
            break;
          }
          value_klass = value_klass->base;
        }

        push(BOOL_VAL(result));
        break;
      }
      case OP_IN: {
        Value in_target = peek(0);
        Value value     = peek(1);

        ObjClass* target_type = typeof_(in_target);

        push(in_target);  // Receiver
        push(value);      // Argument
        if (target_type->__has == NULL) {
          runtime_error("Type %s does not support the 'in' operator. It must implement '" STR(SP_METHOD_HAS) "'.",
                        target_type->name->chars);
          goto finish_error;
        }
        Value result = exec_callable(target_type->__has, 1);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          goto finish_error;
        }

        // Since users can override this, we should that we got a bool back.
        // Could also just use is_falses, to be less strict - but for now I like this better.
        if (!IS_BOOL(result)) {
          runtime_error("Method '" STR(SP_METHOD_HAS) "' on type %s must return a " STR(TYPENAME_BOOL) ", but got %s.",
                        target_type->name->chars, typeof_(result)->name->chars);
          goto finish_error;
        }

        pop();
        pop();
        push(result);
        break;
      }
      case OP_GET_SLICE: {
        // [receiver][start][end] is on the stack
        ObjClass* type = typeof_(peek(2));
        if (type->__slice == NULL) {
          runtime_error("Type %s does not support slicing. It must implement '" STR(SP_METHOD_SLICE) "'.", type->name->chars);
          goto finish_error;
        }

        Value result = exec_callable(type->__slice, 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          goto finish_error;
        }

        push(result);

        break;
      }
      case OP_METHOD: {
        ObjString* name   = READ_STRING();
        FunctionType type = (FunctionType)READ_ONE();  // We trust the compiler that this is either a method, or a static method
        define_method(name, type);
        break;
      }
    }

    if (!(vm.flags & VM_FLAG_HAS_ERROR)) {
      continue;
    }

  finish_error:
    if (handle_error()) {
      frame     = &vm.frames[vm.frame_count - 1];                              // Get the current frame
      frame->ip = frame->closure->function->chunk.code + AS_HANDLER(peek(0));  // Jump to the handler

      // Remove the handler from the stack and push the error value
      pop();
      push(vm.current_error);

      // We're done with the error, so we can clear it
      vm.current_error = NIL_VAL;
    } else {
      return NIL_VAL;
    }
  }

#undef READ_ONE
#undef READ_CONSTANT
#undef READ_STRING

#undef MAKE_BINARY_OP
#undef BIN_ADD
#undef BIN_SUB
#undef BIN_MUL
#undef BIN_DIV

#undef MAKE_COMPARATOR
#undef BIN_LT
#undef BIN_GT
#undef BIN_LTEQ
#undef BIN_GTEQ
}

ObjObject* make_module(const char* source_path, const char* module_name) {
  ObjObject* prev_module = vm.module;
  ObjObject* module      = new_instance(vm.__builtin_Module_class);

  // We need to have the new module active for the duration of the module creation.
  // We'll restore it afterwards.
  vm.module = module;

  // Add a reference to the builtin instance, providing access to the builtin functions
  // TODO (refactor): Remove this (including the INSTANCENAME_BUILTIN Macro) - I think it's not needed,
  // because vm.builtin is used everywhere.
  define_obj(&module->fields, INSTANCENAME_BUILTIN, (Obj*)vm.builtin);
  // Add a reference to the module name, mostly used for stack traces
  define_obj(&module->fields, STR(SP_PROP_MODULE_NAME), (Obj*)(copy_string(module_name, (int)strlen(module_name))));

  // Add a reference to the file path of the module, if available
  if (source_path == NULL) {
    hashtable_set(&module->fields, OBJ_VAL(vm.special_method_names[SPECIAL_PROP_FILE_PATH]), NIL_VAL);
  } else {
    char* base_dir_path = base(source_path);
    define_obj(&module->fields, STR(SP_PROP_FILE_PATH), (Obj*)(copy_string(base_dir_path, (int)strlen(base_dir_path))));
    free(base_dir_path);
  }

  vm.module = prev_module;

  return module;
}

void start_module(const char* source_path, const char* module_name) {
  ObjObject* module = make_module(source_path, module_name);
  vm.module         = module;
}

Value interpret(const char* source, const char* source_path, const char* module_name) {
  ObjObject* enclosing_module = vm.module;
  bool is_module              = module_name != NULL && source_path != NULL;

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
