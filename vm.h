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
#define SP_PROP_DOC __doc
#define SP_PROP_FILE_PATH __file_path
#define SP_PROP_MODULE_NAME __module_name

// Holds the state of a stack frame.
// Represents a single ongoing function call.
typedef struct {
  ObjClosure* closure;
  uint16_t* ip;
  Value* slots;
  HashTable* globals;         // Global variables
  ObjNative* current_native;  // The current native function being executed. This is solely used for better
                              // stack traces. It's not perfect, since natives can call other natives, but
                              // it's better than nothing.
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

  ObjClass* BUILTIN_CLASS(TYPENAME_OBJ);       // Base class: The obj class
  ObjClass* BUILTIN_CLASS(TYPENAME_NUMBER);    // Base class: The number class
  ObjClass* BUILTIN_CLASS(TYPENAME_BOOL);      // Base class: The bool class
  ObjClass* BUILTIN_CLASS(TYPENAME_NIL);       // Base class: The nil class
  ObjClass* BUILTIN_CLASS(TYPENAME_SEQ);       // Special class: The sequence class
  ObjClass* BUILTIN_CLASS(TYPENAME_STRING);    // Special class: The string class
  ObjClass* BUILTIN_CLASS(TYPENAME_FUNCTION);  // Special class: The function class
  ObjClass* BUILTIN_CLASS(TYPENAME_CLASS);     // Special class: The class class

  ObjClass* BUILTIN_CLASS(TYPENAME_MODULE);  // The module class

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
Value exec_callable(Obj* callable, int arg_count);

// Defines a native function in the given table.
void define_native(HashTable* table, const char* name, NativeFn function, const char* doc, int arity);

// Defines an object in the given table.
void define_obj(HashTable* table, const char* name, Obj* obj);

// Retrieves the class of a value. Everything in slang is an 'object', so this function will always return
// a class.
ObjClass* typeof(Value value);

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

// Tries to resolve and push a property of a value onto the stack. If the property is not found, it returns
// false and does not touch the stack.
// `Stack: ...[receiver]` ('name' is not on the stack)
// After the call:
// `Stack: ...[result]` or `Stack: ...[receiver]` if the property was not found.
bool value_get_property(ObjString* name);

// Tries to set a property of a value and leaves just the value on the stack. If the property is not found, it is created.
// Returns true for objects, because it'll always work. Returns false in case of an error, because the value does not
// support setting properties.
// `Stack: ...[receiver][value]`
// After the call:
// `Stack: ...[result]` or `Stack: ...[receiver][value]` if it returned false.
// You should test if the error flag is set after this function, because it might have set it.
bool value_set_property(ObjString* name);

// Tries to resolve and push an index of a value onto the stack. If the index is not found, it returns
// false and does not touch the stack.
// `Stack: ...[receiver][index]`
// After the call:
// `Stack: ...[result]` or `Stack: ...[receiver][index]` if it returned false.
// You should test if the error flag is set after this function, because it might have set it.
bool value_get_index();

// Tries to set an index of a value and leaves just the value on the stack.
// Returns true for objects, because it'll always work. Returns false in case of an error (e.g. bounds-check), or for every other
// type that does not support set-indexing.
// `Stack: ...[receiver][index][value]`
//  After the call:
// `Stack: ...[result]` or `Stack: ...[receiver][index][value]` if it returned false.
// You should test if the error flag is set after this function, because it might have set it.
bool value_set_index();

#endif
