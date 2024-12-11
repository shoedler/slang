#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "file.h"
#include "gc.h"
#include "hashtable.h"
#include "memory.h"
#include "native.h"
#include "object.h"
#include "sys.h"
#include "value.h"
#include "vm.h"

#if defined(DEBUG_TRACE_EXECUTION)
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

void vm_push(Value value) {
  *vm.stack_top = value;
  vm.stack_top++;
}

Value vm_pop() {
  vm.stack_top--;
  return *vm.stack_top;
}

// Look at the value without popping it
static Value peek(int distance) {
  return vm.stack_top[-1 - distance];
}

void vm_clear_error() {
  vm.current_error = nil_value();
  VM_CLEAR_FLAG(VM_FLAG_HAS_ERROR);  // Clear the error flag
}

// Retrieves the most recent call frame.
static inline CallFrame* current_frame() {
  return &vm.frames[vm.frame_count - 1];
}

static void reset_stack() {
  vm.stack_top     = vm.stack;
  vm.frame_count   = 0;
  vm.open_upvalues = NULL;

  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);  // Clear the pause flag, just to be sure
  vm_clear_error();
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
    if (AS_STR(module_name) == function->name) {
      fprintf(stderr, "at the toplevel of module \"%s\"\n", AS_CSTRING(module_name));
    } else {
      fprintf(stderr, "in \"%s\" in module \"%s\"\n", function->name->chars, AS_CSTRING(module_name));
    }
  }

  reset_stack();
}

void vm_error(const char* format, ...) {
  char buffer[1024] = {0};

  va_list args;
  va_start(args, format);
  size_t length = vsnprintf(buffer, 1024, format, args);
  va_end(args);

  VM_SET_FLAG(VM_FLAG_HAS_ERROR);
  vm.current_error = str_value(copy_string(buffer, (int)length));
}

void define_native(HashTable* table, const char* name, NativeFn function, int arity) {
  VM_SET_FLAG(VM_FLAG_PAUSE_GC);
  ObjString* name_str = copy_string(name, (int)strlen(name));
  Value key           = str_value(name_str);
  Value value         = fn_value((Obj*)new_native(function, name_str, arity));

  hashtable_set(table, key, value);
  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);  // Resume GC
}

void define_value(HashTable* table, const char* name, Value value) {
  // TODO (fix):🐛 Seemingly out of nowhere the pushed key and value get swapped on the stack...?! I don't know why. Funnily
  // enough, it only happens when we start a nested module. E.g. we're in "main" and import "std", when the natives get attached
  // to the module instances field within the vm_start_module function key and value are swapped! Same goes for the name (WTF).
  // I've found this out because in the stack trace we now see the modules name and for nested modules it always printed __name.
  // The current remedy is to just use variables for "key" and "value" and just use these instead of pushing and peeking.
  VM_SET_FLAG(VM_FLAG_PAUSE_GC);
  Value key = str_value(copy_string(name, (int)strlen(name)));
  hashtable_set(table, key, value);
  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);
}

void vm_make_seq(int count) {
  // Since we know the count, we can preallocate the value array for the list. This avoids
  // using value_array_write within the loop, which can trigger a GC due to growing the array
  // and free items in the middle of the loop. Also, it lets us pop the list items on the
  // stack, instead of peeking and then having to pop them later (Requiring us to loop over
  // the array twice)
  ValueArray items = value_array_init_of_size(count);
  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = vm_pop();
  }
  items.count = count;
  vm_push(seq_value(take_seq(&items)));
}

void vm_make_tuple(int count) {
  // Since we know the count, we can preallocate the value array for the tuple. This avoids
  // using value_array_write within the loop, which can trigger a GC due to growing the array
  // and free items in the middle of the loop. Also, it lets us pop the tuple items on the
  // stack, instead of peeking and then having to pop them later (Requiring us to loop over
  // the array twice)
  ValueArray items = value_array_init_of_size(count);
  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = vm_pop();
  }
  items.count = count;
  vm_push(tuple_value(take_tuple(&items)));
}

// Creates an object from the top "count" * 2 values on the stack.
// The resulting object is pushed onto the stack.
static void make_object(int count) {
  // Since we know the count, we can preallocate the hashtable for the object. This allows using hashtable_set within the loop. We
  // don't have to worry about it wanting to resize the hashtable, which can trigger a GC and free items in the middle of the
  // loop, because it already has enough capacity. Also, it lets us pop the items on the stack, instead of peeking and then having
  // to pop them later (Requiring us to loop over the keys and values twice)
  HashTable entries;
  hashtable_init(&entries);
  hashtable_init_of_size(&entries, count);

  ObjObject* obj = take_object(&entries);
  for (int i = 0; i < count; i++) {
    Value value = vm_pop();
    Value key   = vm_pop();

    hashtable_set(&obj->fields, key, value);
  }

  vm_push(obj_value(obj));
}

void vm_init() {
  prioritize_main_thread();
  reset_stack();

  vm.objects         = NULL;
  vm.module          = NULL;  // No active module
  vm.bytes_allocated = 0;
  vm.prev_gc_freed   = 0;
  vm.next_gc         = HEAP_DEFAULT_THRESHOLD;
  vm.exit_on_frame   = 0;  // Default to exit on the first frame
  atomic_init(&vm.object_count, 0);

  gc_thread_pool_init(get_cpu_core_count());

  // Pause Gc while we initialize the vm.
  VM_SET_FLAG(VM_FLAG_PAUSE_GC);

  hashtable_init(&vm.strings);
  hashtable_init(&vm.modules);

  // Register the built-in classes
  // Names are null, because we cannot intern them yet. At this point hashtables won't work, bc without the file_base classes the
  // the hashtable cannot compare the keys.
  vm.obj_class     = native_obj_class_partial_init();
  vm.nil_class     = native_nil_class_partial_init();
  vm.str_class     = native_str_class_partial_init();
  vm.class_class   = native_class_class_partial_init();
  vm.fn_class      = native_fn_class_partial_init();
  vm.bool_class    = native_bool_class_partial_init();
  vm.num_class     = native_num_class_partial_init();
  vm.int_class     = native_int_class_partial_init(vm.num_class);
  vm.float_class   = native_float_class_partial_init(vm.num_class);
  vm.seq_class     = native_seq_class_partial_init();
  vm.tuple_class   = native_tuple_class_partial_init();
  vm.upvalue_class = new_class(NULL, NULL);
  vm.handler_class = new_class(NULL, NULL);

  // Now, we can intern the names. Hashtables are now usable.
  ObjString* obj_name     = copy_string(STR(TYPENAME_OBJ), STR_LEN(STR(TYPENAME_OBJ)));
  ObjString* nil_name     = copy_string(STR(TYPENAME_NIL), STR_LEN(STR(TYPENAME_NIL)));
  ObjString* str_name     = copy_string(STR(TYPENAME_STRING), STR_LEN(STR(TYPENAME_STRING)));
  ObjString* class_name   = copy_string(STR(TYPENAME_CLASS), STR_LEN(STR(TYPENAME_CLASS)));
  ObjString* fn_name      = copy_string(STR(TYPENAME_FUNCTION), STR_LEN(STR(TYPENAME_FUNCTION)));
  ObjString* bool_name    = copy_string(STR(TYPENAME_BOOL), STR_LEN(STR(TYPENAME_BOOL)));
  ObjString* num_name     = copy_string(STR(TYPENAME_NUM), STR_LEN(STR(TYPENAME_NUM)));
  ObjString* int_name     = copy_string(STR(TYPENAME_INT), STR_LEN(STR(TYPENAME_INT)));
  ObjString* float_name   = copy_string(STR(TYPENAME_FLOAT), STR_LEN(STR(TYPENAME_FLOAT)));
  ObjString* upvalue_name = copy_string(STR(TYPENAME_UPVALUE), STR_LEN(STR(TYPENAME_UPVALUE)));
  ObjString* handler_name = copy_string(STR(TYPENAME_HANDLER), STR_LEN(STR(TYPENAME_HANDLER)));
  ObjString* seq_name     = copy_string(STR(TYPENAME_SEQ), STR_LEN(STR(TYPENAME_SEQ)));
  ObjString* tuple_name   = copy_string(STR(TYPENAME_TUPLE), STR_LEN(STR(TYPENAME_TUPLE)));

  // ...and assign them to the classes
  vm.obj_class->name     = obj_name;
  vm.nil_class->name     = nil_name;
  vm.str_class->name     = str_name;
  vm.class_class->name   = class_name;
  vm.fn_class->name      = fn_name;
  vm.bool_class->name    = bool_name;
  vm.num_class->name     = num_name;
  vm.int_class->name     = int_name;
  vm.float_class->name   = float_name;
  vm.upvalue_class->name = upvalue_name;
  vm.handler_class->name = handler_name;
  vm.seq_class->name     = seq_name;
  vm.tuple_class->name   = tuple_name;

  // Create the natives lookup table.
  hashtable_init(&vm.natives);

  // Register the builtin classes in the builtin object
  hashtable_set(&vm.natives, str_value(obj_name), class_value(vm.obj_class));
  hashtable_set(&vm.natives, str_value(nil_name), class_value(vm.nil_class));
  hashtable_set(&vm.natives, str_value(str_name), class_value(vm.str_class));
  hashtable_set(&vm.natives, str_value(class_name), class_value(vm.class_class));
  hashtable_set(&vm.natives, str_value(fn_name), class_value(vm.fn_class));
  hashtable_set(&vm.natives, str_value(bool_name), class_value(vm.bool_class));
  hashtable_set(&vm.natives, str_value(num_name), class_value(vm.num_class));
  hashtable_set(&vm.natives, str_value(int_name), class_value(vm.int_class));
  hashtable_set(&vm.natives, str_value(float_name), class_value(vm.float_class));
  hashtable_set(&vm.natives, str_value(upvalue_name), class_value(vm.upvalue_class));
  hashtable_set(&vm.natives, str_value(handler_name), class_value(vm.handler_class));
  hashtable_set(&vm.natives, str_value(seq_name), class_value(vm.seq_class));
  hashtable_set(&vm.natives, str_value(tuple_name), class_value(vm.tuple_class));

  // Build the reserved words lookup table
  memset(vm.special_method_names, 0, sizeof(vm.special_method_names));
  vm.special_method_names[SPECIAL_METHOD_CTOR]   = copy_string(STR(SP_METHOD_CTOR), STR_LEN(STR(SP_METHOD_CTOR)));
  vm.special_method_names[SPECIAL_METHOD_TO_STR] = copy_string(STR(SP_METHOD_TO_STR), STR_LEN(STR(SP_METHOD_TO_STR)));
  vm.special_method_names[SPECIAL_METHOD_HAS]    = copy_string(STR(SP_METHOD_HAS), STR_LEN(STR(SP_METHOD_HAS)));
  vm.special_method_names[SPECIAL_METHOD_SLICE]  = copy_string(STR(SP_METHOD_SLICE), STR_LEN(STR(SP_METHOD_SLICE)));
  vm.special_method_names[SPECIAL_METHOD_ADD]    = copy_string(STR(SP_METHOD_ADD), STR_LEN(STR(SP_METHOD_ADD)));
  vm.special_method_names[SPECIAL_METHOD_SUB]    = copy_string(STR(SP_METHOD_SUB), STR_LEN(STR(SP_METHOD_SUB)));
  vm.special_method_names[SPECIAL_METHOD_MUL]    = copy_string(STR(SP_METHOD_MUL), STR_LEN(STR(SP_METHOD_MUL)));
  vm.special_method_names[SPECIAL_METHOD_DIV]    = copy_string(STR(SP_METHOD_DIV), STR_LEN(STR(SP_METHOD_DIV)));
  vm.special_method_names[SPECIAL_METHOD_MOD]    = copy_string(STR(SP_METHOD_MOD), STR_LEN(STR(SP_METHOD_MOD)));
  vm.special_method_names[SPECIAL_METHOD_LT]     = copy_string(STR(SP_METHOD_LT), STR_LEN(STR(SP_METHOD_LT)));
  vm.special_method_names[SPECIAL_METHOD_GT]     = copy_string(STR(SP_METHOD_GT), STR_LEN(STR(SP_METHOD_GT)));
  vm.special_method_names[SPECIAL_METHOD_LTEQ]   = copy_string(STR(SP_METHOD_LTEQ), STR_LEN(STR(SP_METHOD_LTEQ)));
  vm.special_method_names[SPECIAL_METHOD_GTEQ]   = copy_string(STR(SP_METHOD_GTEQ), STR_LEN(STR(SP_METHOD_GTEQ)));

  memset(vm.special_prop_names, 0, sizeof(vm.special_prop_names));
  vm.special_prop_names[SPECIAL_PROP_LEN]         = copy_string(STR(SP_PROP_LEN), STR_LEN(STR(SP_PROP_LEN)));
  vm.special_prop_names[SPECIAL_PROP_NAME]        = copy_string(STR(SP_PROP_NAME), STR_LEN(STR(SP_PROP_NAME)));
  vm.special_prop_names[SPECIAL_PROP_FILE_PATH]   = copy_string(STR(SP_PROP_FILE_PATH), STR_LEN(STR(SP_PROP_FILE_PATH)));
  vm.special_prop_names[SPECIAL_PROP_MODULE_NAME] = copy_string(STR(SP_PROP_MODULE_NAME), STR_LEN(STR(SP_PROP_MODULE_NAME)));

  native_obj_class_finalize();

  // Create the module class
  ObjString* module_name = copy_string(STR(TYPENAME_MODULE), STR_LEN(STR(TYPENAME_MODULE)));
  vm.module_class        = new_class(module_name, vm.obj_class);
  hashtable_set(&vm.natives, str_value(module_name), class_value(vm.module_class));
  finalize_new_class(vm.module_class);
  hashtable_add_all(&vm.obj_class->methods, &vm.module_class->methods);  // TODO: Unsure why this is required for it to work -
                                                                         // module.entries() is not found otherwise. Investigate

  // Register native functions
  native_register_functions();

  // Finalize native classes - this attaches the methods to the classes. Accessors are already set.
  native_nil_class_finalize();
  native_bool_class_finalize();
  native_num_class_finalize();
  native_int_class_finalize();
  native_float_class_finalize();
  native_seq_class_finalize();
  native_tuple_class_finalize();
  native_str_class_finalize();
  native_fn_class_finalize();
  native_class_class_finalize();

  // Register native modules
  native_register_file_module();
  native_register_perf_module();
  native_register_debug_module();
  native_register_gc_module();
  native_register_math_module();

  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);  // Unpause

  reset_stack();
}

void vm_free() {
  hashtable_free(&vm.strings);
  hashtable_free(&vm.modules);
  memset(vm.special_method_names, 0, sizeof(vm.special_method_names));
  memset(vm.special_prop_names, 0, sizeof(vm.special_prop_names));
  free_heap();
  gc_thread_pool_shutdown();
}

// If [expected] is positive, [actual] must match exactly. If [expected] is negative, [actual] must be at least
// the absolute value of [expected].
#define CHECK_ARGS(expected, actual)                                             \
  if (expected >= 0 ? actual != expected : actual < -expected) {                 \
    if (expected == 1) {                                                         \
      vm_error("Expected 1 argument but got %d.", actual);                       \
    } else if (expected == -1) {                                                 \
      vm_error("Expected at least 1 argument but got %d.", actual);              \
    } else if (expected < -1) {                                                  \
      vm_error("Expected at least %d arguments but got %d.", -expected, actual); \
    } else {                                                                     \
      vm_error("Expected %d arguments but got %d.", expected, actual);           \
    }                                                                            \
    return CALL_FAILED;                                                          \
  }

// Executes a call to a managed-code function or method by creating a new call frame and pushing it onto the
// frame stack.
// `Stack: ...[closure][arg0][arg1]...[argN]`
static CallResult call_managed(ObjClosure* closure, int arg_count) {
  CHECK_ARGS(closure->function->arity, arg_count);

  if (vm.frame_count == FRAMES_MAX) {
    vm_error("Stack overflow. Maximum call stack depth of %d reached.", FRAMES_MAX);
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
  vm_push(result);

  return CALL_RETURNED;
}

#undef CHECK_ARGS

// Calls a callable managed or native value (function, method, class (ctor), etc.) with the given number of
// arguments on the stack.
// `Stack: ...[callable][arg0][arg1]...[argN]`.
// Not used for invocation of methods (no receiver). But it can handle bound methods and constructors - in
// which case it will put the appropriate receiver on the stack.
//
// TODO (refactor): Add class.__call() to handle all this stuff.
static CallResult call_value(Value callable, int arg_count) {
  if (is_native(callable)) {
    return call_native(AS_NATIVE(callable), arg_count);
  }

  if (is_closure(callable)) {
    return call_managed(AS_CLOSURE(callable), arg_count);
  }

  if (is_bound_method(callable)) {
    ObjBoundMethod* bound        = AS_BOUND_METHOD(callable);
    vm.stack_top[-arg_count - 1] = bound->receiver;
    switch (bound->method->type) {
      case OBJ_GC_CLOSURE: return call_managed((ObjClosure*)bound->method, arg_count);
      case OBJ_GC_NATIVE: return call_native((ObjNative*)bound->method, arg_count);
      default: {
        INTERNAL_ERROR("Cannot invoke ctor of type %d", bound->method->type);
        return CALL_FAILED;
      }
    }
  }

  if (is_class(callable)) {
    ObjClass* klass = AS_CLASS(callable);
    // Construct a new instance of the class.
    // We just replace the class on the stack (callable) with an instance of it.
    // This also happens for primitive types. In their constructors, we replace this fresh instance with
    // an actual primitive value.
    vm.stack_top[-arg_count - 1] = instance_value(new_instance(klass));

    // Check if the class has a constructor. 'ctor' is actually a stupid name, because it doesn't
    // construct anything. As you can see, the instance already exists. It's actually more like an 'init'
    // method. It's perfectly valid to have no ctor - you'll also end up with a valid instance on the
    // stack.
    Obj* ctor = klass->__ctor;
    if (ctor != NULL) {
      switch (ctor->type) {
        case OBJ_GC_CLOSURE: return call_managed((ObjClosure*)(ctor), arg_count);
        case OBJ_GC_NATIVE: return call_native((ObjNative*)(ctor), arg_count);
        default: {
          INTERNAL_ERROR("Cannot invoke ctor of type %d", ctor->type);
          return CALL_FAILED;
        }
      }
    }

    if (arg_count != 0) {
      vm_error("Expected 0 arguments but got %d.", arg_count);
      return CALL_FAILED;
    }
    return CALL_RETURNED;
  }

  vm_error("Attempted to call non-callable value of type %s.", callable.type->name->chars);
  return CALL_FAILED;
}

// Invokes a managed-code or native method with receiver and arguments on the top of the stack.
// `Stack: ...[receiver][arg0][arg1]...[argN]`
// You can also provide a [source_klass] to invoke a method on a specific class. This is only used for
// for actual method-lookup. If you provide NULL, the receiver's type will be used.
static CallResult invoke(ObjClass* source_klass, ObjString* name, int arg_count) {
  Value receiver  = peek(arg_count);
  ObjClass* klass = source_klass == NULL ? receiver.type : source_klass;
  Value method;

  // Most likely it's a method on the receiver's class
  if (hashtable_get_by_string(&klass->methods, name, &method)) {
    switch (method.as.obj->type) {
      case OBJ_GC_CLOSURE: return call_managed(AS_CLOSURE(method), arg_count);
      case OBJ_GC_NATIVE: return call_native(AS_NATIVE(method), arg_count);
      default: {
        vm_error("Cannot invoke method of type %s on class", method.type->name->chars);
        return CALL_FAILED;
      }
    }
  }

  // It could be a static method if the receiver is a class
  if (is_class(receiver)) {
    ObjClass* klass_ = AS_CLASS(receiver);
    if (hashtable_get_by_string(&klass_->static_methods, name, &method)) {
      switch (method.as.obj->type) {
        case OBJ_GC_CLOSURE: return call_managed(AS_CLOSURE(method), arg_count);
        case OBJ_GC_NATIVE: return call_native(AS_NATIVE(method), arg_count);
        default: {
          vm_error("Cannot invoke method of type %s on class", method.type->name->chars);
          return CALL_FAILED;
        }
      }
    }
  }

  // It could be a field on an object or instance which is a callable value
  if (is_obj(receiver) || is_instance(receiver)) {
    ObjObject* object = AS_OBJECT(receiver);
    if (hashtable_get_by_string(&object->fields, name, &method)) {
      vm.stack_top[-arg_count - 1] = method;
      return call_value(method, arg_count);
    }
  }

  bool is_base = klass->base == NULL;
  vm_error("Undefined callable '%s' in type %s%s.", name->chars, klass->name->chars,
           is_base ? "" : " or any of its parent classes");
  return CALL_FAILED;
}

// Executes a callframe by running the bytecode until it returns a value or an error occurs.
static Value run_frame() {
  int previous_exit_frame = vm.exit_on_frame;
  vm.exit_on_frame        = vm.frame_count - 1;

  Value result = run();

  vm.exit_on_frame = previous_exit_frame;
  return result;
}

Value vm_exec_callable(Value callable, int arg_count) {
  CallResult result = call_value(callable, arg_count);

  if (result == CALL_RETURNED) {
    return vm_pop();
  }

  if (result == CALL_RUNNING) {
    return run_frame();
  }

  return nil_value();
}

bool bind_method(ObjClass* klass, ObjString* name, Value* bound_method) {
  Value method;
  if (!hashtable_get_by_string(&klass->methods, name, &method)) {
    *bound_method = nil_value();
    return false;
  }

  ObjBoundMethod* bound = new_bound_method(peek(0), method.as.obj);
  *bound_method         = fn_value((Obj*)bound);
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
    case TYPE_METHOD_STATIC: hashtable_set(&klass->static_methods, str_value(name), method); break;
    case TYPE_CONSTRUCTOR:
    case TYPE_METHOD: {
      hashtable_set(&klass->methods, str_value(name), method);
      break;
    }
    default: {
      INTERNAL_ERROR("Unknown method FunctionType %d", type);
      exit(SLANG_EXIT_SW_ERROR);
    }
  }

  vm_pop();
}

bool vm_is_falsey(Value value) {
  return is_nil(value) || (is_bool(value) && !value.as.boolean);
}

bool vm_inherits(ObjClass* klass, ObjClass* base) {
  while (klass != NULL) {
    if (klass == base) {
      return true;
    }
    klass = klass->base;
  }
  return false;
}

char* vm_resolve_module_path(ObjString* cwd, ObjString* module_name, ObjString* module_path) {
  if (module_path == NULL && module_name == NULL) {
    INTERNAL_ERROR("Cannot resolve module path. Both module name and path are NULL.");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  char* absolute_file_path;

  // Either we have a module path, or we check the current working directory
  if (module_path == NULL) {
    // Just slap the module name + extension onto the cwd
    char* module_file_name = file_ensure_slang_extension(module_name->chars);
    absolute_file_path     = file_join_path(cwd->chars, module_file_name);
    free(module_file_name);
  } else {
    // It's probably a realtive path, we add the extension to the provided path and prepend the cwd
    char* module_path_ = file_ensure_slang_extension(module_path->chars);
    absolute_file_path = file_join_path(cwd->chars, module_path_);
    free(module_path_);

    if (!file_exists(absolute_file_path)) {
      // Clearly, it's not a relative path.
      free(absolute_file_path);

      // We assume it is an absolute path instead, which also has the extension already
      absolute_file_path = malloc(module_path->length + 1);
      if (absolute_file_path == NULL) {
        INTERNAL_ERROR("Could not import module '%s'. Out of memory.",
                       module_name == NULL ? module_path->chars : module_name->chars);
        exit(SLANG_EXIT_MEMORY_ERROR);
      }

      strcpy(absolute_file_path, module_path->chars);
    }
  }

  if (absolute_file_path == NULL) {
    INTERNAL_ERROR(
        "Could not produce a valid module path for module '%s'. Cwd is '%s', additional path is "
        "'%s'",
        module_name->chars, cwd->chars, module_path == NULL ? "NULL" : module_path->chars);
    exit(SLANG_EXIT_IO_ERROR);
  }

  return absolute_file_path;
}

// Imports a module by [module_name] and pushes it onto the stack. If the module was already imported, it is loaded
// from cache. If the module was not imported yet, it is loaded from the file system and then cached.
// If [module_path] is NULL, the module is expected to be in the same directory as the importing module. Returns true if
// the module was successfully imported, false otherwise. *WARNING*: Even though the import might have succeeded, the Vm's error
// flag might still be set. Always check the error flag after calling this function.
static bool import_module(ObjString* module_name, ObjString* module_path) {
  Value module;

  // Check if there's a module with the same name in the modules cache.
  // Getting a module this way is only possible for modules that were registered by name only, which is currently
  // only the case for native modules.
  if (hashtable_get_by_string(&vm.modules, module_name, &module)) {
    vm_push(module);
    return true;
  }

  // First, we need to get the current working directory
  Value cwd = native_cwd(0, NULL);
  if (is_nil(cwd)) {
    vm_error(
        "Could not import module '%s'. Could not get current working directory, because there is no "
        "active module or the active module is not a file.",
        module_name->chars);
    return false;
  }

  char* abs_module_path = vm_resolve_module_path(AS_STR(cwd), module_name, module_path);

  // Check if we have already imported the module by absolute path.
  if (hashtable_get_by_string(&vm.modules, copy_string(abs_module_path, strlen(abs_module_path)), &module)) {
    vm_push(module);
    free(abs_module_path);
    return true;
  }

  // Nope, so we need to load the module from the file system
  if (!file_exists(abs_module_path)) {
    vm_error("Could not import module '%s'. File '%s' does not exist.", module_name->chars, abs_module_path);
    free(abs_module_path);
    return false;
  }

  // Load the module by running the file
  int previous_exit_frame = vm.exit_on_frame;
  vm.exit_on_frame        = vm.frame_count;
  module                  = vm_run_file(abs_module_path, module_name->chars);
  vm.exit_on_frame        = previous_exit_frame;

  // Check if the module is actually a module
  if (!(module.type == vm.module_class)) {
    vm_error("Could not import module '%s' from file '%s'. Expected module type", module_name->chars, abs_module_path);
    free(abs_module_path);
    return false;
  }

  vm_push(module);  // Show ourselves to the GC before we do anything that might trigger a GC
  ObjString* path = copy_string(abs_module_path, strlen(abs_module_path));
  vm_push(str_value(path));  // Show ourselves to the GC before we do anything that might trigger a GC
  hashtable_set(&vm.modules, str_value(path), module);
  vm_pop();  // path

  free(abs_module_path);
  return true;
}

void vm_concatenate() {
  ObjString* right = AS_STR(peek(0));  // Peek, so it doesn't get freed by the GC
  ObjString* left  = AS_STR(peek(1));  // Peek, so it doesn't get freed by the GC

  int length  = left->length + right->length;
  char* chars = ALLOCATE_ARRAY(char, length + 1);
  memcpy(chars, left->chars, left->length);
  memcpy(chars + left->length, right->chars, right->length);
  chars[length] = '\0';

  ObjString* result = take_string(chars, length);
  vm_pop();  // right
  vm_pop();  // left
  vm_push(str_value(result));
}

// Handles errors in the virtual machine.
// This function is responsible for handling errors that occur during the execution of the virtual machine.
// It searches for the nearest error handler in the call stack and resets the virtual machine state to that
// handler's call frame.
// If no error handler is found, it prints the error message, dumps the stack trace, resets the virtual
// machine's stack state and returns false. Returns true if an error handler is found and the virtual machine
// state is reset, false otherwise.
static bool handle_runtime_error() {
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
       stack_offset >= exit_slot && !is_handler(vm.stack[stack_offset]);  // Stop at the exit slot or handler
       stack_offset--) {
    ;
  }

  // Did we find a handler within the current frame?
  if (stack_offset < exit_slot) {
    // No handler found and we reached the bottom of the stack. So we print the stacktrace and reset the Vm's
    // stack state. Should be fine to execute more code after this, because the stack is reset.
    if (exit_slot == 0) {
      VM_SET_FLAG(VM_FLAG_HAD_UNCAUGHT_RUNTIME_ERROR);  // Forever-set

      // Store the current error, so we can reset the stack to call the errors __to_str method
      Value error = vm.current_error;
      vm_clear_error();

      // Try to execute the __to_str method of the error value.
      // Now, what if the __to_str method is managed code and it throws an error itself? It's not too bad, because we have that
      // new error too, so we can guide the user through what went wrong.
      vm_push(error);
      ObjString* str = (ObjString*)vm_exec_callable(fn_value(error.type->__to_str), 0).as.obj;
      if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
        fprintf(stderr, "Uncaught error within " STR(SP_METHOD_TO_STR) "-method of previous error value. ");
        fprintf(stderr, "The previous error value was of type: " ANSI_COLOR_RED);
        value_print_safe(stderr, class_value(error.type));
        fprintf(stderr, ANSI_COLOR_RESET "\n");
        fprintf(stderr, "Calling its " STR(SP_METHOD_TO_STR) "-method resulted in the following uncaught error: " ANSI_COLOR_RED);
        value_print_safe(stderr, vm.current_error);
        fprintf(stderr, ANSI_COLOR_RESET "\n");
        vm_clear_error();  // Is done too in reset_stack, but that might change in the future.
      } else {
        fprintf(stderr, "Uncaught error: " ANSI_RED_STR("%s") "\n", str->chars);
      }

      // Pop the synthetic handler
      vm_pop();

      CallFrame* frame      = current_frame();
      ObjFunction* function = frame->closure->function;
      size_t instruction    = frame->ip - function->chunk.code - 1;

      SourceView source = function->chunk.source_views[instruction];
      report_error_location(source);
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
       frame_offset--) {
    ;
  }

  // Did we find a frame that owns the handler?
  if (frame_offset == -1) {
    INTERNAL_ERROR("Call stack corrupted. No call frame found that owns the handler.");
    exit(SLANG_EXIT_SW_ERROR);
  }

  // We have the handler's index/offset in the stack and the frame it belongs to. Now let's reset the Vm to
  // that call frame.
  close_upvalues(&vm.stack[stack_offset]);  // Close upvalues that are no longer needed
  vm.stack_top   = vm.stack + stack_offset + 1;
  vm.frame_count = frame_offset + 1;

  VM_CLEAR_FLAG(VM_FLAG_HAS_ERROR);

  return true;
}

#ifdef DEBUG_TRACE_EXECUTION
static void debug_trace_execution(CallFrame* frame) {
  // Print the current instruction
  debug_disassemble_instruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));

  // Print the VMs stack
  printf(ANSI_CYAN_STR(" Stack "));
  for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
    printf(ANSI_CYAN_STR("["));
    value_print_safe(stdout, *slot);
    printf(ANSI_CYAN_STR("]"));
  }
  printf("\n");

  // Check that we don't have a leaked error state
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    INTERNAL_ERROR(
        "Leaked error state. VM is in error state just before transitioning to the next instruction. You are missing a check "
        "for the VMs error flag somewhere.");
  }
}
#endif

// This function represents the main loop of the virtual machine. It fetches the next instruction, decodes it,
// and dispatches it
static Value run() {
  CallFrame* frame = current_frame();

  static void* dispatch_table[] = {
#define DISPATCH_TABLE_ENTRY(name) &&DO_OP_##name,
      OPCODES(DISPATCH_TABLE_ENTRY)
#undef DISPATCH_TABLE_ENTRY
  };

// Dispatch to the next instruction
#ifdef DEBUG_TRACE_EXECUTION
#define DISPATCH()              \
  debug_trace_execution(frame); \
  goto* dispatch_table[READ_ONE()]
#else
#define DISPATCH() goto* dispatch_table[READ_ONE()]
#endif

// Read a single piece of data from the current instruction pointer and advance it
#define READ_ONE() (*frame->ip++)

// Read a constant from the constant pool. This consumes one piece of data on the stack, which is the index of
// the constant to read
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_ONE()])

// Read a string from the constant pool.
#define READ_STRING() AS_STR(READ_CONSTANT())

#define MAKE_OP(sp_name, op)                                                   \
  Value left   = peek(1);                                                      \
  Value result = vm_exec_callable(fn_value(left.type->PASTE(__, sp_name)), 1); \
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                        \
    goto FINISH_ERROR;                                                         \
  }                                                                            \
  vm_push(result);                                                             \
  DISPATCH();

#ifdef DEBUG_TRACE_EXECUTION
  debug_disassemble_instruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));

  printf(ANSI_CYAN_STR(" Stack "));
  for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
    printf(ANSI_CYAN_STR("["));
    value_print_safe(stdout, *slot);
    printf(ANSI_CYAN_STR("]"));
  }
  printf("\n");
#endif

  DISPATCH();

/**
 * Pushes a constant from the constant pool onto the stack.
 * @note stack: `[...] -> [...][const]`
 * @note synopsis: `OP_CONSTANT, index`
 * @param index index into the constant pool
 */
DO_OP_CONSTANT: {
  Value constant = READ_CONSTANT();
  vm_push(constant);
  DISPATCH();
}

/**
 * Pushes a `nil` value onto the stack.
 * @note stack: `[...] -> [...][nil]`
 * @note synopsis: `OP_NIL`
 */
DO_OP_NIL: {
  vm_push(nil_value());
  DISPATCH();
}

/**
 * Pushes a `true` value onto the stack.
 * @note stack: `[...] -> [...][true]`
 * @note synopsis: `OP_TRUE`
 */
DO_OP_TRUE: {
  vm_push(bool_value(true));
  DISPATCH();
}

/**
 * Pushes a `false` value onto the stack.
 * @note stack: `[...] -> [...][false]`
 * @note synopsis: `OP_FALSE`
 */
DO_OP_FALSE: {
  vm_push(bool_value(false));
  DISPATCH();
}

/**
 * Pops the top value from the stack.
 * @note stack: `[...][a] -> [...]`
 * @note synopsis: `OP_POP`
 */
DO_OP_POP: {
  vm_pop();
  DISPATCH();
}

/**
 * Duplicates the value at the given index on the stack.
 * @note stack: `[...][a] -> [...][a][a]`
 * @note synopsis: `OP_DUPE, index`
 * @param index index into the vms' stack (0: top, 1: second from top, etc.)
 */
DO_OP_DUPE: {
  vm_push(peek(READ_ONE()));
  DISPATCH();
}

/**
 * Gets a local variable and pushes it onto the stack.
 * @note stack: `[...] -> [...][value]`
 * @note synopsis: `OP_GET_LOCAL, slot`
 * @param slot index into the current frames' stack window (aka. slots) (0: top, 1: second from top, etc.)
 */
DO_OP_GET_LOCAL: {
  uint16_t slot = READ_ONE();
  vm_push(frame->slots[slot]);
  DISPATCH();
}

/**
 * Gets a global variable and pushes it onto the stack.
 * @note stack: `[...] -> [...][value]`
 * @note synopsis: `OP_GET_GLOBAL, str_index`
 * @param str_index index into constant pool to get the name, which is then used to get [value] from the globals hashtable (or
 * the natives lookup table - subject to change though)
 */
DO_OP_GET_GLOBAL: {
  ObjString* name = READ_STRING();
  Value value;
  if (!hashtable_get_by_string(frame->globals, name, &value)) {
    if (!hashtable_get_by_string(&vm.natives, name, &value)) {
      vm_error("Undefined variable '%s'.", name->chars);
      goto FINISH_ERROR;
    }
  }
  vm_push(value);
  DISPATCH();
}

/**
 * Gets an upvalue and pushes it onto the stack.
 * @note stack: `[...] -> [value]`
 * @note synopsis: `OP_GET_UPVALUE, slot`
 * @param slot index into the current frames' closures' upvalues (0: first, 1: second, etc.)
 */
DO_OP_GET_UPVALUE: {
  uint16_t slot = READ_ONE();
  vm_push(*frame->closure->upvalues[slot]->location);
  DISPATCH();
}

/**
 * Defines the top value of the stack as a global variable and pops it.
 * @note stack: `[...][value] -> [...]`
 * @note synopsis: `OP_DEFINE_GLOBAL, str_index`
 * @param str_index index into constant pool to get the name, which is then used to set [value] in the globals hashtable.
 */
DO_OP_DEFINE_GLOBAL: {
  ObjString* name = READ_STRING();
  if (!hashtable_set(frame->globals, str_value(name), peek(0))) {
    vm_error("Variable '%s' is already defined.", name->chars);
    goto FINISH_ERROR;
  }
  vm_pop();
  DISPATCH();
}

/**
 * Sets a local variable to the top value of the stack and leaves it.
 * @note stack: `[...][value] -> [...][value]`
 * @note synopsis: `OP_SET_LOCAL, slot`
 * @param slot index into the current frames' stack window (aka. slots) (0: top, 1: second from top, etc.)
 */
DO_OP_SET_LOCAL: {
  uint16_t slot      = READ_ONE();
  frame->slots[slot] = peek(0);  // peek, because assignment is an expression!
  DISPATCH();
}

/**
 * Sets a global variable to the top value of the stack and leaves it.
 * @note stack: `[...][value] -> [...][value]`
 * @note synopsis: `OP_SET_GLOBAL, str_index`
 * @param str_index index into constant pool to get the name, which is then used to set [value] in the globals hashtable.
 */
DO_OP_SET_GLOBAL: {
  ObjString* name = READ_STRING();

  if (hashtable_set(frame->globals, str_value(name),
                    peek(0))) {  // peek, because assignment is an expression!
    hashtable_delete(frame->globals, str_value(name));
    vm_error("Undefined variable '%s'.", name->chars);
    goto FINISH_ERROR;
  }

  DISPATCH();
}

/**
 * Sets an upvalue to the top value of the stack and leaves it.
 * @note stack: `[...][value] -> [...][value]`
 * @note synopsis: `OP_SET_UPVALUE, slot`
 * @param slot index into the current frames' closures' upvalues (0: first, 1: second, etc.)
 */
DO_OP_SET_UPVALUE: {
  uint16_t slot                             = READ_ONE();
  *frame->closure->upvalues[slot]->location = peek(0);  // peek, because assignment is an expression!
  DISPATCH();
}

/**
 * Gets a subscript from the top two values on the stack and pushes the result. (Invokes `__get_subs` on the receiver)
 * @note stack: `[...][receiver][index] -> [...][result]`
 * @note synopsis: `OP_GET_SUBSCRIPT`
 */
DO_OP_GET_SUBSCRIPT: {
  Value receiver = peek(1);
  Value index    = peek(0);
  Value result;

  if (receiver.type->__get_subs(receiver, index, &result)) {
    vm_pop();
    vm_pop();
    vm_push(result);
    DISPATCH();
  }

  goto FINISH_ERROR;  // False return value means it encountered an error
}

/**
 * Sets a subscript on the top three values on the stack and leaves the result. (Invokes `__set_subs` on the receiver)
 * @note stack: `[...][receiver][index][value] -> [...][result]`
 * @note synopsis: `OP_SET_SUBSCRIPT`
 */
DO_OP_SET_SUBSCRIPT: {
  Value receiver = peek(2);
  Value index    = peek(1);
  Value result   = peek(0);

  if (receiver.type->__set_subs(receiver, index, result)) {
    vm_pop();
    vm_pop();
    vm_pop();
    vm_push(result);  // Assignments are expressions
    DISPATCH();
  }

  goto FINISH_ERROR;  // False return value means it encountered an error
}

/**
 * Gets a property from the top value on the stack and pushes the result. (Invokes `__get_prop` on the receiver)
 * @note stack: `[...][receiver] -> [...][result]`
 * @note synopsis: `OP_GET_PROPERTY, str_index`
 * @param str_index index into constant pool to get the name, which is then used to get [result] from the receiver.
 */
DO_OP_GET_PROPERTY: {
  ObjString* name = READ_STRING();
  Value receiver  = peek(0);
  Value result;

  if (receiver.type->__get_prop(receiver, name, &result)) {
    vm_pop();
    vm_push(result);
    DISPATCH();
  }

  goto FINISH_ERROR;  // False return value means it encountered an error
}

/**
 * Sets a property on the 2nd-to-top value on the stack and leaves the result. (Invokes `__set_prop` on the receiver)
 * @note stack: `[...][receiver][value] -> [...][result]`
 * @note synopsis: `OP_SET_PROPERTY, str_index`
 * @param str_index index into constant pool to get the name, which is then used to set [value] in the receiver.
 */
DO_OP_SET_PROPERTY: {
  ObjString* name = READ_STRING();
  Value receiver  = peek(1);
  Value result    = peek(0);

  if (receiver.type->__set_prop(receiver, name, result)) {
    vm_pop();
    vm_pop();
    vm_push(result);  // Assignments are expressions
    DISPATCH();
  }

  goto FINISH_ERROR;  // False return value means it encountered an error
}

/**
 * Gets a method from the top value on the stack, binds it to the 2nd-to-top value, and pushes the result.
 * @note stack: `[...][receiver] -> [...][result]`
 * @note synopsis: `OP_GET_METHOD, str_index`
 * @param str_index index into constant pool to get the name, which is then used to get the method from the receiver.
 */
DO_OP_GET_BASE_METHOD: {
  ObjString* name     = READ_STRING();
  ObjClass* baseclass = AS_CLASS(vm_pop());

  Value bound_method;
  if (!bind_method(baseclass, name, &bound_method)) {
    vm_error("Method '%s' does not exist in '%s'.", name->chars, baseclass->name->chars);
    goto FINISH_ERROR;
  }
  vm_pop();
  vm_push(bound_method);
  DISPATCH();
}

/**
 * Gets a slice from the 3rd-to-top value on the stack and pushes the result. (Invokes `__slice` on the receiver)
 * @note stack: `[...][receiver][start][end] -> [...][slice]`
 * @note synopsis: `OP_GET_SLICE`
 */
DO_OP_GET_SLICE: {
  // [receiver][start][end] is on the stack
  ObjClass* type = peek(2).type;
  Value result   = vm_exec_callable(fn_value(type->__slice), 2);
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }

  vm_push(result);

  DISPATCH();
}

/**
 * Compares the top two values on the stack for equality and pushes the result. (Invokes `__equals` on the values)
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_EQ`
 */
DO_OP_EQ: {
  Value right = vm_pop();
  Value left  = vm_pop();
  vm_push(bool_value(left.type->__equals(left, right)));
  DISPATCH();
}

/**
 * Compares the top two values on the stack for inequality and pushes the result. (Invokes `__equals` on the values)
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_NEQ`
 */
DO_OP_NEQ: {
  Value right = vm_pop();
  Value left  = vm_pop();
  vm_push(bool_value(!left.type->__equals(left, right)));
  DISPATCH();
}

/**
 * Compares the top two values on the stack for greater-than and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_GT`
 */
DO_OP_GT: { MAKE_OP(SP_METHOD_GT, >) }

/**
 * Compares the top two values on the stack for less-than and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_LT`
 */
DO_OP_LT: { MAKE_OP(SP_METHOD_LT, <) }

/**
 * Compares the top two values on the stack for greater-than-or-equal and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_GTEQ`
 */
DO_OP_GTEQ: { MAKE_OP(SP_METHOD_GTEQ, >=) }

/**
 * Compares the top two values on the stack for less-than-or-equal and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_LTEQ`
 */
DO_OP_LTEQ: { MAKE_OP(SP_METHOD_LTEQ, <=) }

/**
 * Adds the top two values on the stack and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_ADD`
 */
DO_OP_ADD: { MAKE_OP(SP_METHOD_ADD, +) }

/**
 * Subtracts the top two values on the stack and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_SUBTRACT`
 */
DO_OP_SUBTRACT: { MAKE_OP(SP_METHOD_SUB, -) }

/**
 * Multiplies the top two values on the stack and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_MULTIPLY`
 */
DO_OP_MULTIPLY: { MAKE_OP(SP_METHOD_MUL, *) }

/**
 * Divides the top two values on the stack and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_DIVIDE`
 */
DO_OP_DIVIDE: { MAKE_OP(SP_METHOD_DIV, /) }

/**
 * Modulos the top two values on the stack and pushes the result.
 * @note stack: `[...][a][b] -> [...][result]`
 * @note synopsis: `OP_MODULO`
 */
DO_OP_MODULO: { MAKE_OP(SP_METHOD_MOD, %) }

/**
 * Checks if the top value on the stack is falsy and pushes the result.
 * @note stack: `[...][a] -> [...][result]`
 * @note synopsis: `OP_NOT`
 */
DO_OP_NOT: {
  vm_push(bool_value(vm_is_falsey(vm_pop())));
  DISPATCH();
}

/**
 * Negates the top value on the stack and pushes the result.
 * @note stack: `[...][a] -> [...][result]`
 * @note synopsis: `OP_NEGATE`
 */
DO_OP_NEGATE: {
  if (is_int(peek(0))) {
    vm_push(int_value(-((vm_pop()).as.integer)));
  } else if (is_float(peek(0))) {
    vm_push(float_value(-((vm_pop()).as.float_)));
  } else {
    vm_error("Type for unary - must be a " STR(TYPENAME_NUM) ". Was %s.", peek(0).type->name->chars);
    goto FINISH_ERROR;
  }
  DISPATCH();
}

/**
 * Prints the top value on the stack and pops it. (Invokes `__to_str` on the value)
 * @note stack: `[...][a] -> [...]`
 * @note synopsis: `OP_PRINT`
 */
DO_OP_PRINT: {
  ObjString* str = (ObjString*)vm_exec_callable(fn_value(peek(0).type->__to_str), 0).as.obj;
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }
  printf("%s\n", str->chars);
  DISPATCH();
}

/**
 * Jumps to the instruction at the given offset (current ip + offset).
 * @note stack: `[...] -> [...]`
 * @note synopsis: `OP_JUMP, offset`
 * @param offset offset to jump to (from the current ip)
 */
DO_OP_JUMP: {
  uint16_t offset = READ_ONE();
  frame->ip += offset;
  DISPATCH();
}

/**
 * Jumps to the instruction at the given offset if the top value on the stack is falsy and leaves it.
 * @note stack: `[...][a] -> [...][a]`
 * @note synopsis: `OP_JUMP_IF_FALSE, offset`
 * @param offset offset to jump to (from the current ip)
 */
DO_OP_JUMP_IF_FALSE: {
  uint16_t offset = READ_ONE();
  if (vm_is_falsey(peek(0))) {
    frame->ip += offset;
  }
  DISPATCH();
}

/**
 * Pushes a handler-value onto the stack (Consists of try-target and the offset from the start of the callframe to the try
 * block).
 * @note stack: `[...] -> [...][handler]`
 * @note synopsis: `OP_TRY, try_target`
 * @param try_target try-target
 */
DO_OP_TRY: {
  uint16_t try_target = READ_ONE();
  uint16_t offset     = frame->ip - frame->closure->function->chunk.code;  // Offset from start of callframe to the try block
  Value handler       = handler_value(try_target + offset);
  vm_push(handler);
  DISPATCH();
}

/**
 * Loops back to the instruction at the given offset (current ip - offset).
 * @note stack: `[...] -> [...]`
 * @note synopsis: `OP_LOOP, offset`
 * @param offset offset to loop back to (from the current ip)
 */
DO_OP_LOOP: {
  uint16_t offset = READ_ONE();
  frame->ip -= offset;
  DISPATCH();
}

/**
 * Calls the callable at the top of the stack with the given number of arguments.
 * @note stack: `[...][callable][arg_0]...[arg_n] -> [...][result]` (in case of a native function)
 * @note stack: `[...][callable][arg_0]...[arg_n] -> [...][arg_0][arg_1]...[arg_n]` (in case of a managed function)
 * @note synopsis: `OP_CALL, arg_count`
 * @param arg_count number of arguments to pass to the callable
 */
DO_OP_CALL: {
  int arg_count = READ_ONE();
  bool failed   = call_value(peek(arg_count), arg_count) == CALL_FAILED;
  if (failed || VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }

  frame = current_frame();  // Set the frame to the current frame
  DISPATCH();
}

/**
 * Invokes the callable at the top of the stack with the given number of arguments.
 * @note stack: `[...][receiver][arg_0]...[arg_n] -> [...][result]` (in case of a native function)
 * @note stack: `[...][receiver][arg_0]...[arg_n] -> [...][receiver][arg_0][arg_1]...[arg_n]` (in case of a managed function)
 * @note synopsis: `OP_INVOKE, str_index, arg_count`
 * @param str_index index into constant pool to get the name of the method to invoke
 * @param arg_count number of arguments to pass to the callable
 */
DO_OP_INVOKE: {
  ObjString* method = READ_STRING();
  int arg_count     = READ_ONE();
  bool failed       = invoke(NULL, method, arg_count) == CALL_FAILED;
  if (failed || VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }

  frame = current_frame();
  DISPATCH();
}

/**
 * Invokes the callable at the top of the stack with the given number of arguments and the base class.
 * @note stack: `[...][receiver][arg_0]...[arg_n][base] -> [...][result]` (in case of a native function)
 * @note stack: `[...][receiver][arg_0]...[arg_n][base] -> [...][receiver][arg_0]...[arg_n]` (in case of a managed function)
 * @note synopsis: `OP_BASE_INVOKE, str_index, arg_count`
 * @param str_index index into constant pool to get the name of the method to invoke
 * @param arg_count number of arguments to pass to the callable
 */
DO_OP_BASE_INVOKE: {
  ObjString* method   = READ_STRING();
  int arg_count       = READ_ONE();
  ObjClass* baseclass = AS_CLASS(vm_pop());  // Leaves 'this' on the stack, followed by the arguments (if any)
  bool failed         = invoke(baseclass, method, arg_count) == CALL_FAILED;
  if (failed || VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }

  frame = current_frame();
  DISPATCH();
}

/**
 * Creates a closure from the function at the given index in the constant pool and pushes it.
 * @note stack: `[...] -> [...][closure]`
 * @note synopsis: `OP_CLOSURE, fn_index, (is_local, index) * upvalue_count`
 * @param fn_index index into constant pool to get the function
 * @param is_local whether the upvalue is local or not
 * @param index index into the current frames' upvalues (0: first, 1: second, etc.)
 */
DO_OP_CLOSURE: {
  ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
  ObjClosure* closure   = new_closure(function);
  vm_push(fn_value((Obj*)closure));

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
  DISPATCH();
}

/**
 * Closes the topmost value on the stack and pops it.
 * @note stack: `[...][value] -> [...]`
 * @note synopsis: `OP_CLOSE_UPVALUE`
 */
DO_OP_CLOSE_UPVALUE: {
  close_upvalues(vm.stack_top - 1);
  vm_pop();
  DISPATCH();
}

/**
 * Creates a sequence literal from the top `count` values on the stack and pushes the result.
 * @note stack: `[...][v_0][v_1]...[v_n] -> [...][seq]`
 * @note synopsis: `OP_SEQ_LITERAL, count`
 * @param count number of values to include in the sequence (from the top of the stack)
 */
DO_OP_SEQ_LITERAL: {
  int count = READ_ONE();
  vm_make_seq(count);
  DISPATCH();
}

/**
 * Creates a tuple literal from the top `count` values on the stack and pushes the result.
 * @note stack: `[...][v_0][v_1]...[v_n] -> [...][tuple]`
 * @note synopsis: `OP_TUPLE_LITERAL, count`
 * @param count number of values to include in the tuple (from the top of the stack)
 */
DO_OP_TUPLE_LITERAL: {
  int count = READ_ONE();
  vm_make_tuple(count);
  DISPATCH();
}

/**
 * Creates an object literal from the top `count` * 2 values on the stack (kvp) and pushes the result.
 * @note stack: `[...][k_0][v_0][k_1][v_1]...[k_n][v_n] -> [...][object]`
 * @note synopsis: `OP_OBJECT_LITERAL, count`
 * @param count number of key-value pairs to include in the object (from the top of the stack)
 */
DO_OP_OBJECT_LITERAL: {
  int count = READ_ONE();
  make_object(count);
  DISPATCH();
}

/**
 * Closes all upvalues in the current frames' slots and returns from the current function.
 * @note stack: `[...][fn] -> [...]`
 * @note synopsis: `OP_RETURN`
 */
DO_OP_RETURN: {
  Value result = vm_pop();
  close_upvalues(frame->slots);
  vm.frame_count--;
  if (vm.frame_count == 0) {
    return vm_pop();  // Return the toplevel function - used for modules.
  }

  vm.stack_top = frame->slots;
  if (vm.exit_on_frame == vm.frame_count) {
    return result;
  }
  vm_push(result);
  frame = current_frame();
  DISPATCH();
}

/**
 * Creates a new class and pushes it onto the stack.
 * @note stack: `[...] -> [...][class]`
 * @note synopsis: `OP_CLASS, str_index`
 * @param str_index index into constant pool to get the name of the class
 */
DO_OP_CLASS: {
  // Initially, a class always vm_inherits from Obj
  ObjClass* klass = new_class(READ_STRING(), vm.obj_class);
  vm_push(class_value(klass));
  hashtable_add_all(&klass->base->methods, &klass->methods);
  DISPATCH();
}

/**
 * Inherits the top value on the stack from the 2nd-to-top value and leaves the base class.
 * @note stack: `[...][base][derived] -> [...][base]`
 * @note synopsis: `OP_INHERIT`
 */
DO_OP_INHERIT: {
  Value baseclass    = peek(1);
  ObjClass* subclass = AS_CLASS(peek(0));
  if (!is_class(baseclass)) {
    vm_error("Base class must be a class. Was %s.", baseclass.type->name->chars);
    goto FINISH_ERROR;
  }
  hashtable_add_all(&AS_CLASS(baseclass)->methods, &subclass->methods);
  subclass->base = AS_CLASS(baseclass);
  vm_pop();  // Subclass.
  DISPATCH();
}

/**
 * Finalizes the top value on the stack and leaves it.
 * @note stack: `[...][class] -> [...][class]`
 * @note synopsis: `OP_FINALIZE`
 */
DO_OP_FINALIZE: {
  Value klass = peek(0);
  finalize_new_class(AS_CLASS(klass));  // We trust the compiler that this value is actually a class
  vm_pop();
  DISPATCH();
}

/**
 * Defines a method in the 2nd-to-top value on the stack and leaves it.
 * @note stack: `[...][class][method] -> [...][class]`
 * @note synopsis: `OP_DEFINE_METHOD, str_index, fn_type`
 * @param str_index index into constant pool to get the name of the method
 * @param fn_type function type of the method
 */
DO_OP_METHOD: {
  ObjString* name   = READ_STRING();
  FunctionType type = (FunctionType)READ_ONE();  // We trust the compiler that this is either a method, or a static method
  define_method(name, type);
  DISPATCH();
}

/**
 * Imports a module by name.
 * @note stack: `[...] -> [...][module]`
 * @note synopsis: `OP_IMPORT, str_index`
 * @param str_index index into constant pool to get the name of the module to import.
 */
DO_OP_IMPORT: {
  ObjString* name = READ_STRING();
  bool success    = import_module(name, NULL);
  if (!success || VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }
  DISPATCH();
}

/**
 * Imports a module by name and file.
 * @note stack: `[...] -> [...][module]`
 * @note synopsis: `OP_IMPORT_FROM, name_index, file_path_index`
 * @param name_index index into constant pool to get the name of the module to import.
 * @param file_path_index index into constant pool to get the path of the file to import from.
 */
DO_OP_IMPORT_FROM: {
  ObjString* name = READ_STRING();
  ObjString* from = READ_STRING();
  bool success    = import_module(name, from);
  if (!success || VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    goto FINISH_ERROR;
  }
  DISPATCH();
}

/**
 * Sets the current error to the top value on the stack (pops it) and puts the VM into error state.
 * @note stack: `[...][error] -> [...]`
 * @note synopsis: `OP_THROW`
 */
DO_OP_THROW: {
  vm.current_error = vm_pop();
  VM_SET_FLAG(VM_FLAG_HAS_ERROR);
  goto FINISH_ERROR;
}

/**
 * Checks if the 2nd-to-top value on the stack is an instance of the top value and pushes the result.
 * @note stack: `[...][instance][class] -> [...][result]`
 * @note synopsis: `OP_IS`
 */
DO_OP_IS: {
  Value type  = vm_pop();
  Value value = vm_pop();

  if (!is_class(type)) {
    vm_error("Type must be a class. Was %s.", type.type->name->chars);
    goto FINISH_ERROR;
  }

  ObjClass* value_klass = value.type;
  ObjClass* type_klass  = AS_CLASS(type);

  bool result = vm_inherits(value_klass, type_klass);

  vm_push(bool_value(result));
  DISPATCH();
}

/**
 * Checks if the 2nd-to-top value on the stack is "in" the top value and pushes the result. (Invokes `__has` on the container)
 * @note stack: `[...][value][container] -> [...][result]`
 * @note synopsis: `OP_CALL_METHOD, arg_count`
 * @param arg_count number of arguments to pass to the callable
 */
DO_OP_IN: {
  Value a = vm_pop();
  Value b = vm_pop();
  vm_push(a);
  vm_push(b);
  MAKE_OP(SP_METHOD_HAS, in)
}

FINISH_ERROR: {
  if (handle_runtime_error()) {
    frame     = current_frame();                                            // Get the current frame
    frame->ip = frame->closure->function->chunk.code + peek(0).as.handler;  // Jump to the handler

    // Remove the handler from the stack and push the error value
    vm_pop();
    vm_push(vm.current_error);

    // We're done with the error, so we can clear it
    vm.current_error = nil_value();
    DISPATCH();
  } else {
    return nil_value();
  }
}

  return nil_value();  // This should never be reached, but the compiler doesn't know that

#undef DISPATCH

#undef READ_ONE
#undef READ_CONSTANT
#undef READ_STRING

#undef MAKE_OP
}

ObjObject* vm_make_module(const char* source_path, const char* module_name) {
  ObjObject* prev_module = vm.module;
  ObjObject* module      = new_instance(vm.module_class);

  // We need to have the new module active for the duration of the module creation.
  // We'll restore it afterwards.
  vm.module = module;

  // Add a reference to the module name, mostly used for stack traces
  define_value(&module->fields, STR(SP_PROP_MODULE_NAME), str_value(copy_string(module_name, (int)strlen(module_name))));

  // Add a reference to the file path of the module, if available
  if (source_path == NULL) {
    hashtable_set(&module->fields, str_value(vm.special_prop_names[SPECIAL_PROP_FILE_PATH]), nil_value());
  } else {
    char* base_dir_path = file_base(source_path);
    define_value(&module->fields, STR(SP_PROP_FILE_PATH), str_value(copy_string(base_dir_path, (int)strlen(base_dir_path))));
    free(base_dir_path);
  }

  vm.module = prev_module;

  return module;
}

void vm_start_module(const char* source_path, const char* module_name) {
  ObjObject* module = vm_make_module(source_path, module_name);
  vm.module         = module;
}

Value vm_interpret(const char* source, const char* source_path, const char* module_name) {
  ObjObject* enclosing_module = vm.module;
  bool is_module              = module_name != NULL && source_path != NULL;

  if (is_module) {
    vm_start_module(source_path, module_name);
  }

  ObjFunction* function = compiler_compile_module(source);
  if (function == NULL) {
    if (is_module) {
      vm.module = enclosing_module;
    }
    return nil_value();
  }

  vm_push(fn_value((Obj*)function));  // Gc protection
  ObjClosure* closure = new_closure(function);
  vm_pop();
  vm_push(fn_value((Obj*)closure));
  call_value(fn_value((Obj*)closure), 0);

  Value result = run();

  if (is_module) {
    Value out = instance_value(vm.module);
    vm.module = enclosing_module;
    return out;
  }

  return result;
}

Value vm_run_file(const char* path, const char* module_name) {
#ifdef DEBUG_TRACE_EXECUTION
  printf("\n");
  printf(ANSI_CYAN_STR("Running file: %s\n"), path);
#endif
  const char* name = module_name == NULL ? path : module_name;
  char* source     = file_read(path);

  if (source == NULL) {
    free(source);
    return nil_value();
  }

  Value result = vm_interpret(source, path, name);
  free(source);

#ifdef DEBUG_TRACE_EXECUTION
  printf(ANSI_CYAN_STR("Done running file: %s\n"), path);
  printf("\n");
#endif

  return result;
}
