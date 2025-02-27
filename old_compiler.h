#ifndef OLD_COMPILER_H
#define OLD_COMPILER_H

#include "object.h"

// We can lift this to UINT16_MAX, but it's hard to test for, so we'll keep it at UINT8_MAX for now.
#define MAX_FN_ARGS 255                 // UINT8_MAX
#define MAX_DESTRUCTURING_BINDINGS 255  // UINT8_MAX

#define MAX_LOCALS (1024 * 3)    // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.
#define MAX_UPVALUES (1024 * 3)  // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.
#define MAX_CONST_GLOBALS \
  (1024 * 3)                 // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.
#define MAX_CONSTANTS 65535  // UINT16_MAX
#define MAX_JUMP 65535       // UINT16_MAX
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
ObjFunction* old_compiler_compile_module(const char* source);

// Parses a number literal and returns the corresponding value (int or float).
Value old_compiler_parse_number(const char* str, size_t length);

// Marks all compiler roots. This is necessary because the compiler can run while the GC is running. Marking compiler roots marks
// all compilers functions.
void old_compiler_mark_roots();

#endif  // OLD_COMPILER_H
