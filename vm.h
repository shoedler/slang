#ifndef vm_h
#define vm_h

#include "chunk.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

#define KEYWORD_CONSTRUCTOR "__ctor"
#define KEYWORD_CONSTRUCTOR_LEN (sizeof(KEYWORD_CONSTRUCTOR) - 1)

#define KEYWORD_NAME "__name"
#define KEYWORD_NAME_LEN (sizeof(KEYWORD_NAME) - 1)

#define KEYWORD_THIS "this"
#define KEYWORD_THIS_LEN (sizeof(KEYWORD_THIS) - 1)

#define KEYWORD_BASE "base"

// Holds the state of a stack frame.
// Represents a single ongoing function call.
typedef struct {
  ObjClosure* closure;
  uint16_t* ip;
  Value* slots;
} CallFrame;

typedef enum {
  METHOD_CTOR,
  METHOD_NAME,

  METHOD_MAX,
} ReservedMethodNames;

// The virtual machine.
// Contains all the state the Vm requires to execute code.
typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frame_count;

  Chunk* chunk;
  uint16_t* ip;  // Instruction pointer, points to the NEXT instruction to execute
  Value stack[STACK_MAX];
  Value* stack_top;   // Stack pointer
  HashTable globals;  // Global variables
  HashTable strings;  // Interned strings
  ObjUpvalue* open_upvalues;
  Obj* objects;

  HashTable modules;  // Modules
  int exit_on_frame;

  ObjClass* object_class;                   // The class of all objects
  ObjClass* module_class;                   // The module class
  ObjInstance* builtin;                     // The builtin (builtin things) object instance
  Value reserved_method_names[METHOD_MAX];  // Reserved method names. They deliberately are not
                                            // using a values array because they are not dynaminc
                                            // and not garbage collected (e.g. always marked)

  int pause_gc;
  size_t bytes_allocated;
  size_t next_gc;
  int gray_count;
  int gray_capacity;
  Obj** gray_stack;  // Worklist for the garbage collector. This field is not
                     // managed by our own memory allocator, but rather by the
                     // system's allocator.
} Vm;

extern Vm vm;

// Initialize the virtual machine.
void init_vm();

// Free the virtual machine.
void free_vm();

// Takes a string of source code, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// If local_scope is true, the code will be executed in a new scope.
Value interpret(const char* source, bool local_scope);

// Reads a file, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// If local_scope is true, the code will be executed in a new scope.
Value run_file(const char* file_name, bool local_scope);

// Push a value onto the stack.
void push(Value value);

// Pop a value off the stack.
Value pop();

#endif
