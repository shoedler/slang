#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "object.h"

#define MAX_CONSTANTS 65535  // UINT16_MAX
#define MAX_JUMP 65535       // UINT16_MAX

typedef struct FnCompiler FnCompiler;

struct FnCompiler {
  struct FnCompiler* enclosing;
  AstFn* function;
  ObjFunction* result;

  Scope* current_scope;  // Current scope, only used to check if we're in global scope.

  // Loop state
  Scope* innermost_loop_scope;
  int innermost_loop_start;
  int brakes_count;
  int brakes_capacity;
  int* brake_jumps;

  bool had_error;
};

// Compiles an AST into a function object. Returns true if emission was successful, false otherwise.
bool compile(AstFn* ast, ObjObject* globals_context, ObjFunction** result);

// Marks the roots of the compiler.
void compiler_mark_roots();

#endif  // COMPILER_H
