#ifndef RESOLVER_H
#define RESOLVER_H

#include "ast.h"
#include "common.h"
#include "object.h"
#include "scope.h"

void resolve(AstFn* node);

#endif  // RESOLVER_H