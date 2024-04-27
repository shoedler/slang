#ifndef compiler_h
#define compiler_h

#include "object.h"
#include "vm.h"

// We can lift this to UINT16_MAX, but it's hard to test for, so we'll keep it
// at UINT8_MAX for now.
#define MAX_FN_ARGS 255  // UINT8_MAX

#define MAX_CONSTANTS 65535             // UINT16_MAX
#define MAX_JUMP 65535                  // UINT16_MAX
#define MAX_SEQ_LITERAL_ITEMS 65535     // UINT16_MAX
#define MAX_TUPLE_LITERAL_ITEMS 65535   // UINT16_MAX
#define MAX_OBJECT_LITERAL_ITEMS 65535  // UINT16_MAX

typedef enum {
  TYPE_FUNCTION,
  TYPE_CONSTRUCTOR,
  TYPE_METHOD,
  TYPE_METHOD_STATIC,
  TYPE_ANONYMOUS_FUNCTION,
  TYPE_MODULE
} FunctionType;

// This function is the main entry point of the compiler. It compiles the given source code into bytecode.
ObjFunction* compile_module(const char* source);

// Marks all compiler roots. This is necessary because the compiler
// can run while the GC is running. Marking compiler roots marks all
// compilers functions.
void mark_compiler_roots();

#endif