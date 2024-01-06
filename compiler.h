#ifndef compiler_h
#define compiler_h

#include "object.h"
#include "vm.h"

// We can lift this to UINT16_MAX, but it's hard to test for, so we'll keep it
// at UINT8_MAX for now.
#define MAX_FN_ARGS UINT8_MAX

#define MAX_CONSTANTS UINT16_MAX
#define MAX_JUMP UINT16_MAX
#define MAX_LIST_ITEMS UINT16_MAX

// This function is the main entry point of the compiler.
// It compiles the given source code into bytecode and returns
// the toplebel function.
// If local_scope is true, the compiler will create a new local scope
ObjFunction* compile(const char* source, bool local_scope);

// Marks all compiler roots. This is necessary because the compiler
// can run while the GC is running. Marking compiler roots marks all
// compilers functions.
void mark_compiler_roots();

#endif