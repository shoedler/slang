#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "scope.h"

#define MAX_FN_ARGS 255                 // UINT8_MAX
#define MAX_DESTRUCTURING_VARS 255      // UINT8_MAX
#define MAX_SEQ_LITERAL_ITEMS 65535     // UINT16_MAX
#define MAX_TUPLE_LITERAL_ITEMS 65535   // UINT16_MAX
#define MAX_OBJECT_LITERAL_ITEMS 65535  // UINT16_MAX

AstFn* parse(const char* source, ObjString* name);

#endif  // PARSER_H s