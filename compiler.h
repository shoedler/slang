#ifndef compiler_h
#define compiler_h

#include "object.h"
#include "vm.h"

#define MAX_FN_ARGS UINT8_MAX

#define MAX_CONSTANTS UINT8_MAX

#define MAX_LIST_ITEMS UINT8_MAX
#define MAX_LIST_ITEMS_LONG ((1 << 24) - 1)

// This function is the main entry point of the compiler.
// It compiles the given source code into bytecode and returns
// the toplebel function.
ObjFunction* compile(const char* source);

// Marks all compiler roots. This is necessary because the compiler
// can run while the GC is running. Marking compiler roots marks all
// compilers functions.
void mark_compiler_roots();

#endif