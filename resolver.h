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

  // Shared state
  bool disable_warnings;    // Disable warnings during compilation
  Scope* root_scope;        // Root scope of the AST, shared between all resolvers
  HashTable* global_scope;  // Global scope of the VM, shared between all resolvers. For stuff that the VM adds, like module_name
  HashTable* native_scope;  // Scope in which all the native functions are declared, shared between all resolvers

  // Loop state
  AstStatement* current_loop;
  int current_loop_fn_locals;

  // Class state
  bool in_class;
  bool has_baseclass;

  bool had_error;
  bool panic_mode;
};

// Resolves a AST. Returns true if the AST is valid, false otherwise.
bool resolve(AstFn* ast, HashTable* global_scope, HashTable* native_scope, bool disable_warnings);

// Marks the roots of the resolver.
void resolver_mark_roots();

#endif  // RESOLVER_H
