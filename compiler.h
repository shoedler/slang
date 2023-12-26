#ifndef compiler_h
#define compiler_h

#include "object.h"
#include "vm.h"

#define MAX_FN_ARGS 255

// Compiles raw source code into a function object
ObjFunction* compile(const char* source);

void mark_compiler_roots();

#endif