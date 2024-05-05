#ifndef compiler_h
#define compiler_h

#include "object.h"
#include "vm.h"

// We can lift this to UINT16_MAX, but it's hard to test for, so we'll keep it
// at UINT8_MAX for now.
#define MAX_FN_ARGS UINT8_MAX

#define MAX_CONSTANTS OPC_T_MAX
#define MAX_JUMP OPC_T_MAX
#define MAX_LIST_ITEMS OPC_T_MAX

// This function is the main entry point of the compiler.
// It compiles the given source code into bytecode and returns
// the toplebel function.
ObjFunction* compile(const char* source);

// Marks all compiler roots. This is necessary because the compiler
// can run while the GC is running. Marking compiler roots marks all
// compilers functions.
void mark_compiler_roots();

#endif