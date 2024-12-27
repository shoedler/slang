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
  Scope* global_scope;   // Global scope, shared between all resolvers

  // Loop state
  AstStatement* current_loop;
  Scope* current_loop_scope;

  // Class state
  bool in_class;
  bool has_baseclass;

  bool had_error;
  bool panic_mode;
};

// Resolves a AST. Returns true if the AST is valid, false otherwise.
bool resolve(AstFn* ast);

// Marks the roots of the resolver.
void resolver_mark_roots();

#endif  // RESOLVER_H