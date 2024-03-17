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

#define KEYWORD_MODULE_NAME "__module_name"
#define KEYWORD_MODULE_NAME_LEN (sizeof(KEYWORD_MODULE_NAME) - 1)

#define KEYWORD_THIS "this"
#define KEYWORD_THIS_LEN (sizeof(KEYWORD_THIS) - 1)

#define KEYWORD_BASE "base"

// Holds the state of a stack frame.
// Represents a single ongoing function call.
typedef struct {
  ObjClosure* closure;
  uint16_t* ip;
  Value* slots;
  HashTable* globals;  // Global variables
} CallFrame;

typedef enum {
  WORD_CTOR,
  WORD_NAME,
  WORD_MODULE_NAME,

  WORD_MAX,
} CachedWords;

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
  Obj* module;        // The current module
  int exit_on_frame;

  ObjClass* object_class;  // The class of all objects
  ObjClass* module_class;  // The module class
  ObjClass* string_class;  // The string class
  ObjClass* number_class;  // The number class
  ObjClass* bool_class;    // The bool class
  ObjClass* nil_class;     // The nil class
  ObjClass* seq_class;     // The sequence class

  ObjInstance* builtin;          // The builtin (builtin things) object instance
  Value cached_words[WORD_MAX];  // Cached words for quick access

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
// Accepts an optional name for the module which should result from calling this function.
Value interpret(const char* source, const char* module_name);

// Reads a file from path, compiles it and then runs it.
// Returns the result of the interpretation as a value.
// Accepts an optional name for the module which should result from calling this function. If NULL is
// provided, path is used as the name.
Value run_file(const char* path, const char* module_name);

// Push a value onto the stack.
void push(Value value);

// Pop a value off the stack.
Value pop();

#endif
