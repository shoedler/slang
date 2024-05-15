#ifndef vm_h
#define vm_h

#include "builtin.h"
#include "chunk.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

#define KEYWORD_THIS "this"
#define KEYWORD_THIS_LEN (STR_LEN(KEYWORD_THIS))

#define KEYWORD_BASE "base"
#define KEYWORD_ERROR "error"

#define SP_METHOD_CTOR ctor
#define SP_METHOD_TO_STR to_str
#define SP_METHOD_HAS has
#define SP_METHOD_SLICE slice

#define SP_PROP_LEN len
#define SP_PROP_NAME __name
#define SP_PROP_FILE_PATH __file_path
#define SP_PROP_MODULE_NAME __module_name

// Holds the state of a stack frame.
// Represents a single ongoing function call.
typedef struct {
  ObjClosure* closure;
  uint16_t* ip;
  Value* slots;
  HashTable* globals;  // Global variables
} CallFrame;

typedef enum {
  // Methods that are commonly used in the VM and need to be accessed quickly.
  SPECIAL_METHOD_CTOR,    // ctor
  SPECIAL_METHOD_TO_STR,  // to_str  Implicitly used by the string interpolation and print functions
  SPECIAL_METHOD_HAS,     // has     Impllicitly used by the 'in' operator
  SPECIAL_METHOD_SLICE,   // slice   Implicitly used by the slice operator and destructuring

  SPECIAL_METHOD_MAX,
} SpecialMethodNames;

typedef enum {
  SPECIAL_PROP_LEN,
  SPECIAL_PROP_NAME,
  SPECIAL_PROP_DOC,
  SPECIAL_PROP_FILE_PATH,
  SPECIAL_PROP_MODULE_NAME,

  SPECIAL_PROP_MAX,
} SpecialPropNames;

// The virtual machine.
// Contains all the state the Vm requires to execute code.
typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frame_count;

  Chunk* chunk;
  uint16_t* ip;  // Instruction pointer, points to the NEXT instruction to execute
  Value stack[STACK_MAX];
  Value* stack_top;   // Stack pointer
  HashTable strings;  // Interned strings
  ObjUpvalue* open_upvalues;
  Obj* objects;

  HashTable modules;  // Modules
  ObjObject* module;  // The current module
  int exit_on_frame;  // Index of the frame to exit on

  ObjClass* obj_class;    // Base class: The obj class
  ObjClass* num_class;    // Base class: The num class
  ObjClass* int_class;    // Num-class: The int class
  ObjClass* float_class;  // Num-class: The float class
  ObjClass* bool_class;   // Base class: The bool class
  ObjClass* nil_class;    // Base class: The nil class
  ObjClass* seq_class;    // Base class: The sequence class
  ObjClass* tuple_class;  // Base class: The tuple class
  ObjClass* str_class;    // Base class: The string class
  ObjClass* fn_class;     // Base class: The function class
  ObjClass* class_class;  // Base class: The class class

  ObjClass* upvalue_class;  // Unused, just for pointer comparison
  ObjClass* handler_class;  // Unused, just for pointer comparison

  ObjClass* module_class;  // Obj-class: The module class

  ObjObject* builtin;                                   // The builtin (builtin things) object instance
  ObjString* special_method_names[SPECIAL_METHOD_MAX];  // Special method names for quick access
  ObjString* special_prop_names[SPECIAL_PROP_MAX];      // Special prop names for quick access

  size_t bytes_allocated;
  size_t next_gc;
  int gray_count;
  int gray_capacity;
  Obj** gray_stack;  // Worklist for the garbage collector. This field is not
                     // managed by our own memory allocator, but rather by the
                     // system's allocator.

  Value current_error;
  int flags;
} Vm;

#define VM_FLAG_PAUSE_GC (1 << 0)
#define VM_FLAG_HAS_ERROR (1 << 1)

extern Vm vm;

// Initialize the virtual machine.
void init_vm();

// Free the virtual machine.
void free_vm();

// Clears the error state of the Vm.
void clear_error();

// Creates a new module instance.
ObjObject* make_module(const char* source_path, const char* module_name);

// Creates a new module instance and sets it as the current module.
void start_module(const char* source_path, const char* module_name);

// Takes a string of source code, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// Accepts an optional source path and name for the module which should result from calling this function.
// Calling without the latter two arguments just runs the code as a script.
Value interpret(const char* source, const char* source_path, const char* module_name);

// Reads a file from path, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// Accepts an optional name for the module which should result from calling this function. If NULL is
// provided, path is used as the name.
Value run_file(const char* path, const char* module_name);

// Push a value onto the stack.
void push(Value value);

// Pop a value off the stack.
Value pop();

// Sets the current error value and puts the Vm into error state.
void runtime_error(const char* format, ...);

// Internal function to execute a call to a managed-code or native callable. Also accepts strings - in that case
// there must be a receiver on the stack, from which the method is resolved.
// This function will execute the callable, pop the it and the arguments off the stack and return the result of the function call,
// leaving the stack "untouched":
//
// `Stack before: ...[receiver|function][arg0][arg1]...[argN]`
// `Stack after:  ...`
//
// **Calls should be followed by a check for errors!**
//
// For native functions, the result will be available "immediately", but for managed code we have to execute the new call frame
// (which was provided by call_managed) to get to the result.
Value exec_callable(Value callable, int arg_count);

// Defines a native function in the given table.
void define_native(HashTable* table, const char* name, NativeFn function, int arity);

// Defines a value in the given table.
void define_value(HashTable* table, const char* name, Value value);

// Determines whether a value is falsey. We consider nil and false to be falsey,
// and everything else to be truthy.
bool is_falsey(Value value);

// Creates a sequence of length "count" from the top "count" values on the stack.
// The resulting sequence is pushed onto the stack.
// This is obviously O(n), so use it with caution.
//
// TODO (optimize): This is mainly used for OP_SEQ_LITERAL, where all items are on the stack, maybe we could
// batch-copy them into the value_array?
void make_seq(int count);

// Creates a tuple of length "count" from the top "count" values on the stack.
// The resulting tuple is pushed onto the stack.
// This is obviously O(n), so use it with caution.
//
// TODO (optimize): This is mainly used for OP_TUPLE_LITERAL, where all items are on the stack, maybe we could
// batch-copy them into the value_array?
void make_tuple(int count);

// Binds a method to an instance by creating a new bound method object from the instance and the method name.
// The stack is unchanged.
bool bind_method(ObjClass* klass, ObjString* name, Value* bound_method);

// Integer Value
static inline Value int_value(long long value) {
  return (Value){.type = vm.int_class, {.integer = value}};
}
static inline bool IS_INT(Value value) {
  return value.type == vm.int_class;
}

// Float Value
static inline Value float_value(double value) {
  return (Value){.type = vm.float_class, {.float_ = value}};
}
static inline bool IS_FLOAT(Value value) {
  return value.type == vm.float_class;
}

// Bool Value
static inline Value bool_value(bool value) {
  return (Value){.type = vm.bool_class, {.boolean = value}};
}
static inline bool IS_BOOL(Value value) {
  return value.type == vm.bool_class;
}

// Nil Value
static inline Value nil_value() {
  return (Value){.type = vm.nil_class};
}
static inline bool IS_NIL(Value value) {
  return value.type == vm.nil_class;
}

// Empty Internal Value
static inline Value empty_internal_value() {
  return (Value){.type = NULL};
}
static inline bool IS_EMPTY_INTERNAL(Value value) {
  return value.type == NULL;
}

// Seq Value
static inline Value seq_value(ObjSeq* value) {
  return (Value){.type = vm.seq_class, {.obj = (Obj*)value}};
}
static inline bool IS_SEQ(Value value) {
  return value.type == vm.seq_class;
}

// Tuple Value
static inline Value tuple_value(ObjTuple* value) {
  return (Value){.type = vm.tuple_class, {.obj = (Obj*)value}};
}
static inline bool IS_TUPLE(Value value) {
  return value.type == vm.tuple_class;
}

// String Value
static inline Value str_value(ObjString* value) {
  return (Value){.type = vm.str_class, {.obj = (Obj*)value}};
}
static inline bool IS_STRING(Value value) {
  return value.type == vm.str_class;
}

static inline bool IS_FUNCTION(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_FUNCTION;
}

static inline bool IS_CLOSURE(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_CLOSURE;
}

static inline bool IS_NATIVE(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_NATIVE;
}

static inline bool IS_BOUND_METHOD(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_BOUND_METHOD;
}

// Class Value
static inline Value class_value(ObjClass* value) {
  return (Value){.type = vm.class_class, {.obj = (Obj*)value}};
}
static inline bool IS_CLASS(Value value) {
  return value.type == vm.class_class;
}

// Object Value
static inline Value obj_value(ObjObject* value) {
  return (Value){.type = vm.obj_class, {.obj = (Obj*)value}};
}
static inline bool IS_OBJ(Value value) {
  return value.type == vm.obj_class;
}

// Handler Value
static inline Value handler_value(uint16_t value) {
  return (Value){.type = vm.handler_class, {.handler = value}};
}
static inline bool IS_HANDLER(Value value) {
  return value.type == vm.handler_class;
}

// Converts a value into a bound method. Value must be of type bound method.
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)(value.as.obj))

// Converts a value into a class. Value must be of type class.
#define AS_CLASS(value) ((ObjClass*)(value.as.obj))

// Converts a value into a closure. Value must be of type closure.
#define AS_CLOSURE(value) ((ObjClosure*)(value.as.obj))

// Converts a value into a sequence. Value must be of type sequence.
#define AS_SEQ(value) ((ObjSeq*)(value.as.obj))

// Converts a value into a tuple. Value must be of type tuple.
#define AS_TUPLE(value) ((ObjTuple*)(value.as.obj))

// Gets the value array of a listlike object (Seq, Tuple).
// Hack: Just cast to ObjSeq* and access the items field, because the layout is the same.
#define LISTLIKE_GET_VALUEARRAY(value) ((ObjSeq*)(value.as.obj))->items

// Converts a value into a function. Value must be of type function.
#define AS_FUNCTION(value) ((ObjFunction*)(value.as.obj))

// Converts a value into an object. Value must be of type object.
#define AS_OBJECT(value) ((ObjObject*)(value.as.obj))

// Converts a value into a native function. Value must be of type native function.
#define AS_NATIVE(value) (((ObjNative*)(value.as.obj)))

// Converts a value into a string. Value must be of type string.
#define AS_STRING(value) ((ObjString*)(value.as.obj))

// Converts a value into a C string. Value must be of type string.
#define AS_CSTRING(value) (((ObjString*)(value.as.obj))->chars)

//
// Utility functions for values
//

static inline bool IS_NON_OBJECT(Value value) {
  // TODO: Remove this. This cannot be used anywhere, bc it's extremely slow.
  return value.type == vm.nil_class || value.type == vm.bool_class || value.type == vm.int_class ||
         value.type == vm.float_class || value.type == vm.handler_class || value.type == NULL;
}

static inline bool IS_INSTANCE(Value value) {
  // TODO: Remove this. This cannot be used anywhere, bc it's extremely slow.
  return value.type != vm.obj_class && value.type != vm.nil_class && value.type != vm.str_class && value.type != vm.class_class &&
         value.type != vm.fn_class && value.type != vm.bool_class && value.type != vm.num_class && value.type != vm.int_class &&
         value.type != vm.float_class && value.type != vm.upvalue_class && value.type != vm.handler_class &&
         value.type != vm.seq_class && value.type != vm.tuple_class;
}
static inline Value instance_value(ObjObject* instance) {
  return (Value){.type = instance->instance_class, {.obj = (Obj*)instance}};
}

// Float or int is a number.
static inline bool is_num(Value value) {
  return IS_INT(value) || IS_FLOAT(value);
}

// Function, closure, native or bound method is a function.
// [fn] must be of one these types.
static inline Value fn_value(Obj* fn) {
  return (Value){.type = vm.fn_class, {.obj = fn}};
}
// Function, closure, native or bound method is a function.
static inline bool is_fn(Value value) {
  return value.type == vm.fn_class;
}

// Callables are fn's or classes.
static inline bool IS_CALLABLE(Value value) {
  return is_fn(value) || IS_CLASS(value);
}

// Get the arity of a callable
static inline int callable_get_arity(Value callable) {
  if (IS_CLASS(callable)) {
    Obj* ctor = AS_CLASS(callable)->__ctor;
    if (ctor == NULL) {
      return 0;
    }
    return callable_get_arity(fn_value(ctor));
  }
  if (IS_BOUND_METHOD(callable)) {
    return callable_get_arity(fn_value(AS_BOUND_METHOD(callable)->method));
  }
  if (IS_NATIVE(callable)) {
    return AS_NATIVE(callable)->arity;
  }
  if (IS_CLOSURE(callable)) {
    return AS_CLOSURE(callable)->function->arity;
  }
  if (IS_FUNCTION(callable)) {
    return AS_FUNCTION(callable)->arity;
  }

  INTERNAL_ERROR("Unhandled callable type: %s", callable.type->name->chars);
  return 0;
}

static inline ObjString* fn_get_name(Value fn) {
  if (IS_BOUND_METHOD(fn)) {
    fn = fn_value((Obj*)AS_BOUND_METHOD(fn)->method);
  }
  if (IS_CLOSURE(fn)) {
    fn = fn_value((Obj*)AS_CLOSURE(fn)->function);
  }
  if (IS_NATIVE(fn)) {
    return AS_NATIVE(fn)->name;
  }
  if (IS_FUNCTION(fn)) {
    return AS_FUNCTION(fn)->name;
  }
  INTERNAL_ERROR("Unhandled function type: %s", fn.type->name->chars);
  return NULL;
}
#endif
