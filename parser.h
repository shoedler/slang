#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "scope.h"

#define MAX_FN_ARGS 255                 // UINT8_MAX
#define MAX_DESTRUCTURING_VARS 255      // UINT8_MAX
#define MAX_SEQ_LITERAL_ITEMS 65535     // UINT16_MAX
#define MAX_TUPLE_LITERAL_ITEMS 65535   // UINT16_MAX
#define MAX_OBJECT_LITERAL_ITEMS 65535  // UINT16_MAX

typedef struct Parser Parser;

struct Parser {
  Token current;    // The current token.
  Token previous;   // The token before the current.
  bool had_error;   // True if there was an error during parsing.
  bool panic_mode;  // True if the parser requires synchronization.
};

// Parses a source string into an AST. Returns true if successful, false if there was a syntax error.
bool parse(const char* source, ObjString* name, AstFn** ast);

// Marks the roots of the parser.
void parser_mark_roots();

#endif  // PARSER_H s