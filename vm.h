#ifndef vm_h
#define vm_h

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include "chunk.h"
#include "common.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

#define KEYWORD_THIS "this"
#define KEYWORD_BASE "base"
#define KEYWORD_ERROR "error"

#define SP_METHOD_CTOR ctor
#define SP_METHOD_TO_STR to_str
#define SP_METHOD_HAS has
#define SP_METHOD_SLICE slice
#define SP_METHOD_ADD add
#define SP_METHOD_SUB sub
#define SP_METHOD_MUL mul
#define SP_METHOD_DIV div
#define SP_METHOD_MOD mod
#define SP_METHOD_LT lt
#define SP_METHOD_GT gt
#define SP_METHOD_LTEQ lteq
#define SP_METHOD_GTEQ gteq

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
  SPECIAL_METHOD_CTOR,
  SPECIAL_METHOD_TO_STR,  // Implicitly used by the string interpolation and print functions
  SPECIAL_METHOD_HAS,     // Impllicitly used by the 'in' operator
  SPECIAL_METHOD_SLICE,   // Implicitly used by the slice operator and destructuring
  SPECIAL_METHOD_ADD,     // Implicitly used by the '+' operator and things like Seq.sum()
  SPECIAL_METHOD_SUB,     // Implicitly used by the '-' operator
  SPECIAL_METHOD_MUL,     // Implicitly used by the '*' operator
  SPECIAL_METHOD_DIV,     // Implicitly used by the '/' operator
  SPECIAL_METHOD_MOD,     // Implicitly used by the '%' operator
  SPECIAL_METHOD_LT,      // Implicitly used by the '<' operator and things like Seq.min(), Seq.max(), Seq.sort()
  SPECIAL_METHOD_GT,      // Implicitly used by the '>' operator
  SPECIAL_METHOD_LTEQ,    // Implicitly used by the '<=' operator
  SPECIAL_METHOD_GTEQ,    // Implicitly used by the '>=' operator

  SPECIAL_METHOD_MAX,
} SpecialMethodNames;

typedef enum {
  SPECIAL_PROP_LEN,
  SPECIAL_PROP_NAME,
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
  Value* stack_top;  // Stack pointer

  HashTable modules;  // Modules
  HashTable strings;  // Interned strings
  HashTable natives;  // The table of native functions and types
  ObjUpvalue* open_upvalues;
  Obj* objects;  // Linked list of all objects in the VM (heap)
  atomic_size_t object_count;

  ObjString* special_method_names[SPECIAL_METHOD_MAX];  // Special method names for quick access
  ObjString* special_prop_names[SPECIAL_PROP_MAX];      // Special prop names for quick access

  ObjObject* module;  // The current module
  int exit_on_frame;  // Index of the frame to exit on
  Value current_error;
  int flags;

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

  size_t bytes_allocated;  // Number of bytes currently allocated.
  size_t prev_gc_freed;    // Number of bytes freed in the last garbage collection.
  size_t next_gc;          // Number of bytes at which the next garbage collection will occur.
} Vm;

#define VM_FLAG_PAUSE_GC (1 << 0)
#define VM_FLAG_HAS_ERROR (1 << 1)
#define VM_FLAG_STRESS_GC (1 << 2)
#define VM_FLAG_HAD_COMPILE_ERROR (1 << 3)
#define VM_FLAG_HAD_UNCAUGHT_RUNTIME_ERROR (1 << 4)

#define VM_SET_FLAG(flag) (vm.flags |= (flag))
#define VM_CLEAR_FLAG(flag) (vm.flags &= ~(flag))
#define VM_HAS_FLAG(flag) (vm.flags & (flag))

extern Vm vm;

// Initialize the virtual machine.
void vm_init();

// Free the virtual machine.
void vm_free();

// Creates a new module instance. [source_path] is optional. The [module_name] however, is required.
ObjObject* vm_make_module(const char* source_path, const char* module_name);

// Creates a new module instance and sets it as the current module.
void vm_start_module(const char* source_path, const char* module_name);

// Takes a string of source code, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// Accepts an optional source path and name for the module which should result from calling this function.
// Calling without the latter two arguments just runs the code as a script.
Value vm_interpret(const char* source, const char* source_path, const char* module_name);

// Reads a file from path, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// Accepts an optional name for the module which should result from calling this function. If NULL is
// provided, path is used as the name.
Value vm_run_file(const char* path, const char* module_name);
Value vm_run_file2(const char* path, const char* module_name);

// Push a value onto the stack.
void vm_push(Value value);

// Pop a value off the stack.
Value vm_pop();

// Sets the current error value and puts the Vm into error state.
void vm_error(const char* format, ...);

// Clears the error state of the Vm.
void vm_clear_error();

// Internal function to execute a call to a managed-code or native callable.
// This function will execute the [callable] with the arguments on the stack. [arg_count] of zero means only a receiver (or
// [callable]) is on the stack. The stack should be in the following state:
//
// `Stack before: ...[callable|receiver][arg0][arg1]...[argN]`
// `Stack after:  ...`
//
// Note that [callable] does not have to be on the stack - there can be a receiver instead. This is useful for calling methods.
// **Calls should be followed by a check for errors!**
Value vm_exec_callable(Value callable, int arg_count);

// Determines whether a [value] is falsey. We consider nil and false to be falsey,
// and everything else to be truthy.
bool vm_is_falsey(Value value);

// Determines whether a [klass] vm_inherits from [base]
bool vm_inherits(ObjClass* klass, ObjClass* base);

// Concatenates two strings on the stack (pops them) into a new string and pushes it onto the stack
// `Stack: ...[a][b]` → `Stack: ...[a+b]`
void vm_concatenate();

// Resolves an import-path to an absolute file-path, based on the current working directory.
// - [module_name] can be NULL, if the module is imported solely by [module_path] (relative or absolute).
// - [module_path] can be NULL, if the module is expected to be in the same directory as the importing module and the file is
// named after [module_name].
// Returns the absolute path to the module. The caller is responsible for freeing the memory.
char* vm_resolve_module_path(ObjString* cwd, ObjString* module_name, ObjString* module_path);

// Creates a sequence of length "count" from the top "count" values on the stack.
// The resulting sequence is pushed onto the stack.
// This is obviously O(n), so use it with caution.
//
// TODO (optimize): This is mainly used for OP_SEQ_LITERAL, where all items are on the stack, maybe we could
// batch-copy them into the value_array?
void vm_make_seq(int count);

// Creates a tuple of length "count" from the top "count" values on the stack.
// The resulting tuple is pushed onto the stack.
// This is obviously O(n), so use it with caution.
//
// TODO (optimize): This is mainly used for OP_TUPLE_LITERAL, where all items are on the stack, maybe we could
// batch-copy them into the value_array?
void vm_make_tuple(int count);

// Binds a method to an instance by creating a new bound method object from the instance and the method name.
// The stack is unchanged.
// TODO (refactor): Move to object.h/c
bool bind_method(ObjClass* klass, ObjString* name, Value* bound_method);

// Defines a native [function] in the given [table] with the provided [name] and [arity].
// TODO (refactor): Move to object.h/c
void define_native(HashTable* table, const char* name, NativeFn function, int arity);

// Defines a [value] in the given [table] with the provided [name].
// TODO (refactor): Move to object.h/c
void define_value(HashTable* table, const char* name, Value value);

// TODO (refactor): Move all of the following to value.h/c

// Wraps an integer into a value.
static inline Value int_value(long long value) {
  return (Value){.type = vm.int_class, {.integer = value}};
}
// Checks if a value is of type int.
static inline bool is_int(Value value) {
  return value.type == vm.int_class;
}

// Wraps a float into a value.
static inline Value float_value(double value) {
  return (Value){.type = vm.float_class, {.float_ = value}};
}
// Checks if a value is of type float.
static inline bool is_float(Value value) {
  return value.type == vm.float_class;
}

// Wraps a boolean into a value.
static inline Value bool_value(bool value) {
  return (Value){.type = vm.bool_class, {.boolean = value}};
}
// Checks if a value is of type bool.
static inline bool is_bool(Value value) {
  return value.type == vm.bool_class;
}

// Wraps nil into a value.
static inline Value nil_value() {
  return (Value){.type = vm.nil_class};
}
// Checks if a value is of type nil.
static inline bool is_nil(Value value) {
  return value.type == vm.nil_class;
}

// Wraps a seq-obj into a value.
static inline Value seq_value(ObjSeq* value) {
  return (Value){.type = vm.seq_class, {.obj = (Obj*)value}};
}
// Checks if a value is of type seq.
static inline bool is_seq(Value value) {
  return value.type == vm.seq_class;
}

// Wraps a tuple-obj into a value.
static inline Value tuple_value(ObjTuple* value) {
  return (Value){.type = vm.tuple_class, {.obj = (Obj*)value}};
}
// Checks if a value is of type tuple.
static inline bool is_tuple(Value value) {
  return value.type == vm.tuple_class;
}

// Wraps a string-obj into a value.
static inline Value str_value(ObjString* value) {
  return (Value){.type = vm.str_class, {.obj = (Obj*)value}};
}
// Checks if a value is of type string.
static inline bool is_str(Value value) {
  return value.type == vm.str_class;
}

// Wraps a class-obj into a value.
static inline Value class_value(ObjClass* value) {
  return (Value){.type = vm.class_class, {.obj = (Obj*)value}};
}
// Checks if a value is of type class.
static inline bool is_class(Value value) {
  return value.type == vm.class_class;
}

// Wraps an obj-object into a value.
static inline Value obj_value(ObjObject* value) {
  return (Value){.type = vm.obj_class, {.obj = (Obj*)value}};
}
// Checks if a value is of type obj.
static inline bool is_obj(Value value) {
  return value.type == vm.obj_class;
}

// Wraps an obj-object into a value. The type of the value is inferred by the instance type of the obj-object.
static inline Value instance_value(ObjObject* instance) {
  return (Value){.type = instance->instance_class, {.obj = (Obj*)instance}};
}
// Checks if a value is an instance. An instance is basically any value whose type is not an internal type.
static inline bool is_instance(Value value) {
  return value.type != vm.obj_class && value.type != vm.nil_class && value.type != vm.str_class && value.type != vm.class_class &&
         value.type != vm.fn_class && value.type != vm.bool_class && value.type != vm.num_class && value.type != vm.int_class &&
         value.type != vm.float_class && value.type != vm.upvalue_class && value.type != vm.handler_class &&
         value.type != vm.seq_class && value.type != vm.tuple_class;
}

// Checks if a value is of type fn AND the object is of type function.
static inline bool is_function(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_FUNCTION;
}

// Checks if a value is of type fn AND the object is of type closure.
static inline bool is_closure(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_CLOSURE;
}

// Checks if a value is of type fn AND the object is of type native.
static inline bool is_native(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_NATIVE;
}

// Checks if a value is of type fn AND the object is of type bound method.
static inline bool is_bound_method(Value value) {
  return value.type == vm.fn_class && value.as.obj->type == OBJ_GC_BOUND_METHOD;
}

// Wraps any function-like-obj into a value. [fn] must be of one: Function, closure, native or bound method
static inline Value fn_value(Obj* fn) {
  return (Value){.type = vm.fn_class, {.obj = fn}};
}
// Checks if a value is of type fn.
static inline bool is_fn(Value value) {
  return value.type == vm.fn_class;
}

// Wraps a handler into a value.
static inline Value handler_value(uint16_t value) {
  return (Value){.type = vm.handler_class, {.handler = value}};
}
// Checks if a value is of type handler.
static inline bool is_handler(Value value) {
  return value.type == vm.handler_class;
}

// Wraps an empty internal into a value.
static inline Value empty_internal_value() {
  return (Value){.type = NULL};
}
// Checks if a value is of type empty internal.
static inline bool is_empty_internal(Value value) {
  return value.type == NULL;
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

// Converts a value into a function. Value must be of type function.
#define AS_FUNCTION(value) ((ObjFunction*)(value.as.obj))

// Converts a value into an object. Value must be of type object.
#define AS_OBJECT(value) ((ObjObject*)(value.as.obj))

// Converts a value into a native function. Value must be of type native function.
#define AS_NATIVE(value) (((ObjNative*)(value.as.obj)))

// Converts a value into a string. Value must be of type string.
#define AS_STR(value) ((ObjString*)(value.as.obj))

// Converts a value into a C string. Value must be of type string.
#define AS_CSTRING(value) (((ObjString*)(value.as.obj))->chars)

//
// Utility functions for values
//

// Checks if a value is a primitive. Primitive implies that the value is not an object and not markable by the GC.
static inline bool is_primitive(Value value) {
  return value.type == vm.nil_class || value.type == vm.bool_class || value.type == vm.int_class ||
         value.type == vm.float_class || value.type == vm.handler_class || value.type == NULL;
}

// Callables are fn's or classes.
static inline bool is_callable(Value value) {
  return is_fn(value) || is_class(value);
}

// Get the arity of a callable
static inline int callable_get_arity(Value callable) {
  if (is_class(callable)) {
    Obj* ctor = AS_CLASS(callable)->__ctor;
    if (ctor == NULL) {
      return 0;
    }
    return callable_get_arity(fn_value(ctor));
  }
  if (is_bound_method(callable)) {
    return callable_get_arity(fn_value(AS_BOUND_METHOD(callable)->method));
  }
  if (is_native(callable)) {
    return AS_NATIVE(callable)->arity;
  }
  if (is_closure(callable)) {
    return AS_CLOSURE(callable)->function->arity;
  }
  if (is_function(callable)) {
    return AS_FUNCTION(callable)->arity;
  }

  INTERNAL_ERROR("Unhandled callable type: %s", callable.type->name->chars);
  return 0;
}

static inline ObjString* fn_get_name(Value fn) {
  if (is_bound_method(fn)) {
    fn = fn_value(AS_BOUND_METHOD(fn)->method);
  }
  if (is_closure(fn)) {
    fn = fn_value((Obj*)AS_CLOSURE(fn)->function);
  }
  if (is_native(fn)) {
    return AS_NATIVE(fn)->name;
  }
  if (is_function(fn)) {
    return AS_FUNCTION(fn)->name;
  }
  INTERNAL_ERROR("Unhandled function type: %s", fn.type->name->chars);
  return NULL;
}
#endif
