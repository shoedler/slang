#ifndef RESOLVER_H
#define RESOLVER_H

#include "ast.h"
#include "common.h"
#include "object.h"
#include "scope.h"

// Resolves a AST. Returns true if the AST is valid, false otherwise.
bool resolve(AstFn* node);

#endif  // RESOLVER_H