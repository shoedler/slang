#ifndef COMPILER2_H
#define COMPILER2_H

#include "ast.h"
#include "object.h"

#define MAX_CONSTANTS 65535  // UINT16_MAX
#define MAX_JUMP 65535       // UINT16_MAX

ObjFunction* compile(AstFn* node, ObjObject* globals_context);

#endif  // COMPILER2_H