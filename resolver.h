#ifndef RESOLVER_H
#define RESOLVER_H

#include "ast.h"
#include "common.h"
#include "object.h"
#include "scope.h"

typedef struct FnResolver FnResolver;
struct FnResolver {
  struct FnResolver* enclosing;
  AstFn* function;
  Scope* current_scope;  // Current scope, can be a child scope of the [function]s scope
  AstStatement* current_loop;
  Scope* current_loop_scope;
  bool in_class;
  bool has_baseclass;
  int function_local_count;  // Number of local variables in the current function, including its nested scopes. Used to set
                             // function_index when resolving variables/upvalues
};

// Resolves a AST. Returns true if the AST is valid, false otherwise.
bool resolve(AstFn* ast);

// Marks the roots of the resolver.
void resolver_mark_roots();

#endif  // RESOLVER_H