#ifndef vm_h
#define vm_h

#include "chunk.h"
#include "hashtable.h"
#include "object.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

#define CLASS_CONSTRUCTOR_KEYWORD "ctor"
#define CLASS_CONSTRUCTOR_KEYWORD_LENGTH (sizeof(CLASS_CONSTRUCTOR_KEYWORD) - 1)

#define THIS_KEYWORD "this"
#define THIS_KEYWORD_LENGTH (sizeof(THIS_KEYWORD) - 1)

#define BASE_CLASS_KEYWORD "base"

// Holds the state of a stack frame.
// Represents a single ongoing function call.
typedef struct {
  ObjClosure* closure;
  uint16_t* ip;
  Value* slots;
} CallFrame;

// The virtual machine.
// Contains all the state the Vm requires to execute code.
typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frame_count;

  Chunk* chunk;
  uint16_t*
      ip;  // Instruction pointer, points to the NEXT instruction to execute
  Value stack[STACK_MAX];
  Value* stack_top;   // Points to where the next value to be pushed will go
  HashTable globals;  // Global variables
  HashTable strings;  // Interned strings
  HashTable modules;  // Modules
  ObjString* init_string;
  ObjUpvalue* open_upvalues;
  Obj* objects;

  size_t bytes_allocated;
  size_t next_gc;
  int gray_count;
  int gray_capacity;
  int exit_on_frame;
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
