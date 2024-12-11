#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "scope.h"

#define MAX_FN_ARGS 255                 // UINT8_MAX
#define MAX_DESTRUCTURING_VARS 255      // UINT8_MAX
#define MAX_SEQ_LITERAL_ITEMS 65535     // UINT16_MAX
#define MAX_TUPLE_LITERAL_ITEMS 65535   // UINT16_MAX
#define MAX_OBJECT_LITERAL_ITEMS 65535  // UINT16_MAX

typedef struct {
  Token current;    // The current token.
  Token previous;   // The token before the current.
  bool had_error;   // True if there was an error during parsing.
  bool panic_mode;  // True if the parser requires synchronization.
  AstRoot* root;    // The root of the AST
} Parser2;

AstRoot* parse(const char* source);

#endif  // PARSER_H s