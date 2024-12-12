#ifndef RESOLVER_H
#define RESOLVER_H

#include "ast.h"
#include "common.h"
#include "object.h"
#include "scope.h"

#define MAX_LOCALS (1024 * 3)    // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.
#define MAX_UPVALUES (1024 * 3)  // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.

void resolve(AstRoot* node);

#endif  // RESOLVER_H