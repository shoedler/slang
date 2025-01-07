#include "parser.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "common.h"
#include "memory.h"
#include "scanner.h"
#include "scope.h"
#include "vm.h"

typedef enum {
  PREC_NONE,
  PREC_ASSIGN,      // =
  PREC_TERNARY,     // ?:
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >= is in
  PREC_TERM,        // + -
  PREC_FACTOR,      // * / %
  PREC_UNARY,       // ! -
  PREC_CALL,        // . () [] grouping with parens ++ --
  PREC_PRIMARY
} Precedence;

// Signature for a function that parses a prefix or infix expression.
typedef AstExpression* (*ParsePrefixFn)(Parser* parser, Token expr_start);
typedef AstExpression* (*ParseInfixFn)(Parser* parser, Token expr_start, AstExpression* left);

// Precedence-dependent rule for parsing expressions.
typedef struct {
  ParsePrefixFn prefix;  // Prefix, e.g. unary operators. Example: -1, where '-' is the prefix operator.
  ParseInfixFn infix;    // Infix, e.g. binary operators. Example: 1 + 2, where '+' is the infix operator.
  Precedence precedence;
} ParseRule;

typedef enum {
  DESTRUCTURE_SEQ,
  DESTRUCTURE_OBJ,
  DESTRUCTURE_TUPLE,
} DestructureType;

// Forward declarations
static AstNode* parse_declaration(Parser* parser);
static AstFn* parse_function(Parser* parser, Token decl_start, Token name, FnType type);
static AstStatement* parse_statement(Parser* parser);
static AstBlock* parse_block(Parser* parser);
static AstExpression* parse_precedence(Parser* parser, Precedence precedence);
static AstExpression* parse_expression(Parser* parser);
static ParseRule* get_rule(TokenKind type);

static void parser_init(Parser* parser) {
  parser->current    = (Token){.type = TOKEN_EOF};
  parser->previous   = (Token){.type = TOKEN_EOF};
  parser->had_error  = false;
  parser->panic_mode = false;
}

// Report a parser error.
static void handle_parser_error(Parser* parser, Token* token, const char* format, va_list args) {
  if (parser->panic_mode) {
    return;
  }
  parser->panic_mode = true;
  parser->had_error  = true;

  if (token->type == TOKEN_ERROR) {
    fprintf(stderr, "Scanner error at line %d: " ANSI_RED_STR("%.*s"), token->line, token->length, token->start);
    return;
  }

  fprintf(stderr, "Parser error at line %d", token->line);
  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type != TOKEN_ERROR) {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": " ANSI_COLOR_RED);
  vfprintf(stderr, format, args);
  fprintf(stderr, ANSI_COLOR_RESET "\n");

  SourceView source = chunk_make_source_view(*token, *token);
  report_error_location(source);
}

// Prints the given token as an error message. Sets the parser into panic mode to avoid cascading errors.
static void parser_error(Parser* parser, Token* token, const char* format, ...) {
  va_list args;
  va_start(args, format);
  handle_parser_error(parser, token, format, args);
  va_end(args);
}

// Prints an error message at the previous token. Sets the parser into panic mode to avoid cascading errors.
static void parser_error_at_previous(Parser* parser, const char* format, ...) {
  va_list args;
  va_start(args, format);
  handle_parser_error(parser, &parser->previous, format, args);
  va_end(args);
}

// Prints an error message at the current token. Sets the parser into panic mode to avoid cascading errors.
static void parser_error_at_current(Parser* parser, const char* format, ...) {
  va_list args;
  va_start(args, format);
  handle_parser_error(parser, &parser->current, format, args);
  va_end(args);
}

// Get a synthetic token.
static Token synthetic_token(const char* text) {
  return (Token){
      .is_first_on_line = false,
      .length           = (int)strlen(text),
      .line             = 0,
      .start            = text,
  };
}

// Get the next token. Consumes error tokens, so that on the next call we have a valid token.
static void advance(Parser* parser) {
  parser->previous = parser->current;

  for (;;) {
    parser->current = scanner_scan_token();

    if (parser->current.type != TOKEN_ERROR) {
      break;
    }

    parser_error_at_current(parser, parser->current.start);
  }
}

// Assert the current token according to the provided token type and advance the parser.
static void consume(Parser* parser, TokenKind type, const char* format, ...) {
  if (parser->current.type == type) {
    advance(parser);
    return;
  }

  va_list args;
  va_start(args, format);
  handle_parser_error(parser, &parser->current, format, args);
  va_end(args);
}

// Compares the current token with the provided token type.
static bool check(Parser* parser, TokenKind type) {
  return parser->current.type == type;
}

// Checks if the current token is a statement terminator. Mainly used for return statements, because they can be followed an
// expression
static bool check_statement_return_end(Parser* parser) {
  return parser->current.is_first_on_line ||  // If the current token is the first on the line
         check(parser, TOKEN_CBRACE) ||  // If the current token is a closing brace, indicating the end of a block ({ ret x` })
         check(parser, TOKEN_ELSE) ||    // If the current token is an else keyword (if a ret b` else ret c)
         check(parser, TOKEN_EOF);       // If the current token is the end of the file
}

// Checks if an expression is a valid assignment target.
static bool check_assignment_target(Parser* parser, AstExpression* expr) {
  if (expr == NULL) {
    INTERNAL_ASSERT(parser->panic_mode == true, "expr should be NULL only in panic mode.");
    UNUSED(parser);  // Release mode
    return false;
  }

  switch (expr->type) {
    case EXPR_VARIABLE:
    case EXPR_SUBS:
    case EXPR_DOT: return true;
    default: return false;
  }
}

// Accept the current token if it matches the provided token type, otherwise do nothing.
static bool match(Parser* parser, TokenKind type) {
  if (!check(parser, type)) {
    return false;
  }
  advance(parser);
  return true;
}

// Synchronize the parser after a parsing error.
static void synchronize(Parser* parser) {
  parser->panic_mode = false;

  while (parser->current.type != TOKEN_EOF) {
    switch (parser->current.type) {
      case TOKEN_CLASS:
      case TOKEN_FN:
      case TOKEN_LET:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN: return;

      default:;  // Do nothing.
    }

    advance(parser);
  }
}

Value parse_number(const char* str, size_t length) {
  if (length == 0) {
    return int_value(0);
  }

  if (str[0] == '0' && length >= 2) {
    char kind = str[1];
    // See if it's a hexadecimal, binary, or octal number.
    if ((kind == 'x' || kind == 'X')) {
      long long value = strtoll(str + 2, NULL, 16);
      return int_value(value);
    }

    if ((kind == 'b' || kind == 'B')) {
      long long value = strtoll(str + 2, NULL, 2);
      return int_value(value);
    }

    if ((kind == 'o' || kind == 'O')) {
      long long value = strtoll(str + 2, NULL, 8);
      return int_value(value);
    }
  }

  // Check if the number is a float.
  bool is_float = false;
  for (size_t i = 0; i < length; i++) {
    if (str[i] == '.') {
      is_float = true;
      break;
    }
  }

  if (is_float) {
    double value = strtod(str, NULL);
    return float_value(value);
  } else {
    long long int value = strtoll(str, NULL, 10);
    return int_value(value);
  }
}

static AstExpression* parse_expr_grouping_or_literal_tuple(Parser* parser, Token expr_start) {
  // Empty tuple
  if (match(parser, TOKEN_COMMA)) {
    consume(parser, TOKEN_CPAR, "Expecting ')' after empty " STR(TYPENAME_TUPLE) " literal. ");
    AstLiteral* tuple = ast_lit_tuple_init(expr_start, parser->previous);
    return ast_expr_literal_init(expr_start, parser->previous, tuple);
  }

  AstExpression* inner = parse_expression(parser);
  if (!match(parser, TOKEN_COMMA)) {
    consume(parser, TOKEN_CPAR, "Expecting ')' after grouping expression.");
    return ast_expr_grouping_init(expr_start, parser->previous, (AstExpression*)inner);
  }

  // Tuple
  AstLiteral* tuple = ast_lit_tuple_init(expr_start, parser->previous);
  ast_lit_tuple_add_item(tuple, (AstExpression*)inner);
  int count = 1;

  if (!check(parser, TOKEN_CPAR)) {
    do {
      if (check(parser, TOKEN_CPAR)) {
        break;  // Allow trailing comma.
      }
      ast_lit_tuple_add_item(tuple, parse_expression(parser));
      count++;
    } while (match(parser, TOKEN_COMMA));
  }
  consume(parser, TOKEN_CPAR, "Expecting ')' after " STR(TYPENAME_TUPLE) " literal. Or maybe you are missing a ','?");

  if (count >= MAX_TUPLE_LITERAL_ITEMS) {
    // Still use 'parser_error_at_current' because the message would be very long if we'd start at 'error_start'.
    parser_error_at_current(parser, "Can't have more than " STR(MAX_TUPLE_LITERAL_ITEMS) " items in a " STR(TYPENAME_TUPLE) ".");
  }

  tuple->base.token_end = parser->previous;
  return ast_expr_literal_init(expr_start, parser->previous, tuple);
}

static AstExpression* parse_literal_obj(Parser* parser, Token expr_start) {
  AstLiteral* obj = ast_lit_obj_init(expr_start, parser->previous);
  int count       = 0;

  if (!check(parser, TOKEN_CBRACE)) {
    do {
      if (check(parser, TOKEN_CBRACE)) {
        break;  // Allow trailing comma.
      }
      AstExpression* key = parse_expression(parser);
      consume(parser, TOKEN_COLON, "Expecting ':' after key.");
      AstExpression* value = parse_expression(parser);
      ast_lit_obj_add_kv_pair(obj, key, value);
      count++;
    } while (match(parser, TOKEN_COMMA));
  }
  consume(parser, TOKEN_CBRACE, "Expecting '}' after " STR(TYPENAME_OBJ) " literal. Or maybe you are missing a ','?");

  if (count >= MAX_OBJECT_LITERAL_ITEMS) {
    // Still use 'parser_error_at_current' because the message would be very long if we'd start at 'error_start'.
    parser_error_at_current(parser, "Can't have more than " STR(MAX_OBJECT_LITERAL_ITEMS) " items in a " STR(TYPENAME_OBJ) ".");
  }

  obj->base.token_end = parser->previous;
  return ast_expr_literal_init(expr_start, parser->previous, obj);
}

static AstExpression* parse_literal_seq(Parser* parser, Token expr_start) {
  AstLiteral* seq = ast_lit_seq_init(expr_start, parser->previous);
  int count       = 0;

  if (!check(parser, TOKEN_CBRACK)) {
    do {
      if (check(parser, TOKEN_CBRACK)) {
        break;  // Allow trailing comma.
      }
      ast_lit_seq_add_item(seq, parse_expression(parser));
      count++;
    } while (match(parser, TOKEN_COMMA));
  }
  consume(parser, TOKEN_CBRACK, "Expecting ']' after " STR(TYPENAME_SEQ) " literal. Or maybe you are missing a ','?");

  if (count >= MAX_SEQ_LITERAL_ITEMS) {
    // Still use 'parser_error_at_current' because the message would be very long if we'd start at 'error_start'.
    parser_error_at_current(parser, "Can't have more than " STR(MAX_SEQ_LITERAL_ITEMS) " items in a " STR(TYPENAME_SEQ) ".");
  }

  seq->base.token_end = parser->previous;
  return ast_expr_literal_init(expr_start, parser->previous, seq);
}

static AstExpression* parse_literal_string(Parser* parser, Token expr_start) {
  // Build the string using a flexible array
  size_t str_capacity = 8;
  size_t str_length   = 0;
  char* str_bytes     = (char*)malloc(str_capacity);

#define PUSH_CHAR(c)                                          \
  do {                                                        \
    if (str_capacity < str_length + 1) {                      \
      size_t old   = str_capacity;                            \
      str_capacity = GROW_CAPACITY(old);                      \
      str_bytes    = (char*)realloc(str_bytes, str_capacity); \
    }                                                         \
    str_bytes[str_length++] = c;                              \
  } while (0)

  do {
    const char* cur = parser->previous.start + 1;                            // Skip the opening quote.
    const char* end = parser->previous.start + parser->previous.length - 1;  // Skip the closing quote.

    while (cur < end) {
      if (*cur == '\\') {
        if (cur + 1 > end) {
          parser_error_at_current(parser, "Unterminated escape sequence.");
          goto FINISH_STR;
        }
        switch (cur[1]) {
          case 'f': PUSH_CHAR('\f'); break;  // Form feed
          case 'r': PUSH_CHAR('\r'); break;  // Carriage return
          case 'n': PUSH_CHAR('\n'); break;  // Newline
          case 't': PUSH_CHAR('\t'); break;  // Tab
          case 'v': PUSH_CHAR('\v'); break;  // Vertical tab
          case 'b': PUSH_CHAR('\b'); break;  // Backspace
          case '\\': PUSH_CHAR('\\'); break;
          case '\"': PUSH_CHAR('\"'); break;
          case '\'': PUSH_CHAR('\''); break;
          case '0': PUSH_CHAR('\0'); break;  // Null character
          default: parser_error_at_current(parser, "Invalid escape sequence."); goto FINISH_STR;
        }
        cur += 2;
      } else {
        PUSH_CHAR(*cur);
        cur++;
      }
    }
  } while (match(parser, TOKEN_STRING));

FINISH_STR:
  // Emit the string constant.
  ObjString* str          = copy_string(str_bytes, (int)str_length);
  AstLiteral* str_literal = ast_lit_str_init(expr_start, parser->previous, str);

  // Cleanup
  free(str_bytes);

  return ast_expr_literal_init(expr_start, parser->previous, str_literal);
#undef PUSH_CHAR
}

static AstExpression* parse_literal_number(Parser* parser, Token expr_start) {
  Value value             = parse_number(parser->previous.start, parser->previous.length);
  AstLiteral* num_literal = ast_lit_number_init(expr_start, parser->previous, value);
  return ast_expr_literal_init(expr_start, parser->previous, num_literal);
}

static AstExpression* parse_literal(Parser* parser, Token expr_start) {
  TokenKind type = parser->previous.type;
  switch (type) {
    case TOKEN_TRUE:
    case TOKEN_FALSE: {
      AstLiteral* bool_lit = ast_lit_bool_init(expr_start, parser->previous, type == TOKEN_TRUE);
      return ast_expr_literal_init(expr_start, parser->previous, bool_lit);
    }
    case TOKEN_NIL: {
      AstLiteral* nil_lit = ast_lit_nil_init(expr_start, parser->previous);
      return ast_expr_literal_init(expr_start, parser->previous, nil_lit);
    }
    default: INTERNAL_ERROR("Unhandled literal type: %d", type); return NULL;
  }
}

static AstExpression* parse_expr_anon_fn(Parser* parser, Token expr_start) {
  Token name = synthetic_token(VALUE_STR_ANON_FN);
  AstFn* fn  = parse_function(parser, expr_start, name, FN_TYPE_ANONYMOUS_FUNCTION);
  return ast_expr_anon_fn_init(expr_start, parser->previous, fn);
}

static AstExpression* parse_expr_base(Parser* parser, Token expr_start) {
  AstId* this_ = ast_id_init(expr_start, copy_string(KEYWORD_THIS, STR_LEN(KEYWORD_THIS)));
  AstId* base_ = ast_id_init(expr_start, copy_string(KEYWORD_BASE, STR_LEN(KEYWORD_BASE)));

  // Since 'base' can only be called or accessed with a dot, we check that this is the case. This makes it easier to emit the
  // bytecode as we can assume where a base-node can occur in the AST.
  if (!check(parser, TOKEN_DOT) && !check(parser, TOKEN_OPAR)) {
    parser_error_at_current(parser, "Expected '.' or '('. after '" KEYWORD_BASE "'.");
  }

  return ast_expr_base_init(expr_start, parser->previous, this_, base_);
}

static AstExpression* parse_expr_this(Parser* parser, Token expr_start) {
  AstId* this_ = ast_id_init(expr_start, copy_string(KEYWORD_THIS, STR_LEN(KEYWORD_THIS)));
  return ast_expr_this_init(expr_start, parser->previous, this_);
}

static AstExpression* parse_expr_try(Parser* parser, Token expr_start) {
  AstId* error         = ast_id_init(expr_start, copy_string(KEYWORD_ERROR, STR_LEN(KEYWORD_ERROR)));
  AstExpression* expr  = parse_expression(parser);
  AstExpression* else_ = NULL;
  if (match(parser, TOKEN_ELSE)) {
    else_ = parse_expression(parser);
  }
  return ast_expr_try_init(expr_start, parser->previous, error, expr, else_);
}

static AstExpression* parse_expr_unary(Parser* parser, Token expr_start) {
  Token operator_      = parser->previous;
  AstExpression* inner = parse_precedence(parser, PREC_UNARY);

  if (token_is_inc_dec(operator_.type) && !check_assignment_target(parser, inner)) {
    parser_error_at_previous(parser, "Invalid increment/decrement target.");
    return NULL;
  }

  return ast_expr_unary_init(expr_start, parser->previous, operator_, inner);
}

static AstExpression* parse_expr_variable(Parser* parser, Token expr_start) {
  ObjString* name = copy_string(parser->previous.start, parser->previous.length);
  AstId* id       = ast_id_init(parser->previous, name);
  return ast_expr_variable_init(expr_start, parser->previous, id);
}

static AstExpression* parse_expr_binary(Parser* parser, Token expr_start, AstExpression* left) {
  Token operator_      = parser->previous;
  ParseRule* rule      = get_rule(operator_.type);
  AstExpression* right = parse_precedence(parser, (Precedence)(rule->precedence + 1));
  return ast_expr_binary_init(expr_start, parser->previous, operator_, left, right);
}

static AstExpression* parse_expr_postfix(Parser* parser, Token expr_start, AstExpression* left) {
  Token operator_ = parser->previous;

  if (token_is_inc_dec(operator_.type) && !check_assignment_target(parser, left)) {
    parser_error_at_previous(parser, "Invalid increment/decrement target.");
    return NULL;
  }

  return ast_expr_postfix_init(expr_start, parser->previous, operator_, left);
}

static AstExpression* parse_expr_call(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* call = ast_expr_call_init(expr_start, parser->previous, left);

  // Same logic as EXPR_INVOKE
  int arg_count = 0;
  if (!check(parser, TOKEN_CPAR)) {
    do {
      if (arg_count >= MAX_FN_ARGS) {
        parser_error_at_current(parser, "Can't have more than " STR(MAX_FN_ARGS) " arguments.");
      }
      ast_expr_call_add_argument(call, parse_expression(parser));
      arg_count++;
    } while (match(parser, TOKEN_COMMA));
  }

  consume(parser, TOKEN_CPAR, "Expecting ')' after arguments.");

  call->base.token_end = parser->previous;
  return call;
}

static AstExpression* parse_expr_subs_or_slice(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* start = NULL;
  AstExpression* end   = NULL;

#define RETURN_SLICE()                                                \
  if (match(parser, TOKEN_ASSIGN)) {                                  \
    parser_error_at_previous(parser, "Slices can't be assigned to."); \
  }                                                                   \
  return ast_expr_slice_init(expr_start, parser->previous, left, start, end);

  // Handles foo[..] or foo[..e]
  if (match(parser, TOKEN_DOTDOT)) {
    if (match(parser, TOKEN_CBRACK)) {
      RETURN_SLICE();
    }
    end = parse_expression(parser);
    consume(parser, TOKEN_CBRACK, "Expecting ']' after slice.");
    RETURN_SLICE();
  }

  // Parse the start index
  start = parse_expression(parser);

  // Handles foo[e..] or foo[e..e]
  if (match(parser, TOKEN_DOTDOT)) {
    if (match(parser, TOKEN_CBRACK)) {
      RETURN_SLICE();
    }
    end = parse_expression(parser);
    consume(parser, TOKEN_CBRACK, "Expecting ']' after slice.");
    RETURN_SLICE();
  }

  // Else, it's a subscripting expression
  consume(parser, TOKEN_CBRACK, "Expecting ']' after index.");
  return ast_expr_subs_init(expr_start, parser->previous, left, start);
}

static AstExpression* parse_expr_dot(Parser* parser, Token expr_start, AstExpression* left) {
  if (!match(parser, TOKEN_ID)) {
    consume(parser, TOKEN_CTOR, "Expecting property or method name after '.'.");
  }

  ObjString* property = copy_string(parser->previous.start, parser->previous.length);
  AstId* id           = ast_id_init(parser->previous, property);

  if (match(parser, TOKEN_OPAR)) {
    AstExpression* invoke = ast_expr_invoke_init(expr_start, parser->previous, left, id);

    // Same logic as EXPR_CALL
    int arg_count = 0;
    if (!check(parser, TOKEN_CPAR)) {
      do {
        if (arg_count >= MAX_FN_ARGS) {
          parser_error_at_current(parser, "Can't have more than " STR(MAX_FN_ARGS) " arguments.");
        }
        ast_expr_invoke_add_argument(invoke, parse_expression(parser));
        arg_count++;
      } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_CPAR, "Expecting ')' after arguments.");

    invoke->base.token_end = parser->previous;
    return invoke;
  } else {
    return ast_expr_dot_init(expr_start, parser->previous, left, id);
  }
}

static AstExpression* parse_expr_ternary(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* true_branch = parse_precedence(parser, PREC_TERNARY);
  consume(parser, TOKEN_COLON, "Expecting ':' after true branch.");
  AstExpression* false_branch = parse_precedence(parser, PREC_TERNARY);
  return ast_expr_ternary_init(expr_start, parser->previous, left, true_branch, false_branch);
}

static AstExpression* parse_expr_and(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* right = parse_precedence(parser, PREC_AND);
  return ast_expr_and_init(expr_start, parser->previous, left, right);
}

static AstExpression* parse_expr_or(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* right = parse_precedence(parser, PREC_OR);
  return ast_expr_or_init(expr_start, parser->previous, left, right);
}

static AstExpression* parse_expr_is(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* right = parse_precedence(parser, PREC_COMPARISON);
  return ast_expr_is_init(expr_start, parser->previous, left, right);
}

static AstExpression* parse_expr_in(Parser* parser, Token expr_start, AstExpression* left) {
  AstExpression* right = parse_precedence(parser, PREC_COMPARISON);
  return ast_expr_in_init(expr_start, parser->previous, left, right);
}

static AstExpression* parse_expr_assign(Parser* parser, Token expr_start, AstExpression* left) {
  Token operator_ = parser->previous;

  if (operator_.type == TOKEN_ASSIGN && !check_assignment_target(parser, left)) {
    parser_error_at_previous(parser, "Invalid assignment target.");
    return NULL;
  } else if (token_is_compound_assignment(operator_.type) && !check_assignment_target(parser, left)) {
    parser_error_at_previous(parser, "Invalid compound assignment target.");
    return NULL;
  }

  AstExpression* right = parse_precedence(parser, PREC_ASSIGN);
  return ast_expr_assign_init(expr_start, parser->previous, operator_, left, right);
}

static ParseRule rules2[] = {
    [TOKEN_OPAR]         = {parse_expr_grouping_or_literal_tuple, parse_expr_call, PREC_CALL},
    [TOKEN_CPAR]         = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACE]       = {parse_literal_obj, NULL, PREC_NONE},
    [TOKEN_CBRACE]       = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACK]       = {parse_literal_seq, parse_expr_subs_or_slice, PREC_CALL},
    [TOKEN_CBRACK]       = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]        = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT]          = {NULL, parse_expr_dot, PREC_CALL},
    [TOKEN_MINUS]        = {parse_expr_unary, parse_expr_binary, PREC_TERM},
    [TOKEN_PLUS]         = {NULL, parse_expr_binary, PREC_TERM},
    [TOKEN_PLUS_PLUS]    = {parse_expr_unary, parse_expr_postfix, PREC_CALL},
    [TOKEN_MINUS_MINUS]  = {parse_expr_unary, parse_expr_postfix, PREC_CALL},
    [TOKEN_DIV]          = {NULL, parse_expr_binary, PREC_FACTOR},
    [TOKEN_MULT]         = {NULL, parse_expr_binary, PREC_FACTOR},
    [TOKEN_MOD]          = {NULL, parse_expr_binary, PREC_FACTOR},
    [TOKEN_NOT]          = {parse_expr_unary, NULL, PREC_NONE},
    [TOKEN_TERNARY]      = {NULL, parse_expr_ternary, PREC_TERNARY},
    [TOKEN_NEQ]          = {NULL, parse_expr_binary, PREC_EQUALITY},
    [TOKEN_ASSIGN]       = {NULL, parse_expr_assign, PREC_ASSIGN},
    [TOKEN_PLUS_ASSIGN]  = {NULL, parse_expr_assign, PREC_ASSIGN},
    [TOKEN_MINUS_ASSIGN] = {NULL, parse_expr_assign, PREC_ASSIGN},
    [TOKEN_MULT_ASSIGN]  = {NULL, parse_expr_assign, PREC_ASSIGN},
    [TOKEN_DIV_ASSIGN]   = {NULL, parse_expr_assign, PREC_ASSIGN},
    [TOKEN_MOD_ASSIGN]   = {NULL, parse_expr_assign, PREC_ASSIGN},
    [TOKEN_EQ]           = {NULL, parse_expr_binary, PREC_EQUALITY},
    [TOKEN_GT]           = {NULL, parse_expr_binary, PREC_COMPARISON},
    [TOKEN_GTEQ]         = {NULL, parse_expr_binary, PREC_COMPARISON},
    [TOKEN_LT]           = {NULL, parse_expr_binary, PREC_COMPARISON},
    [TOKEN_LTEQ]         = {NULL, parse_expr_binary, PREC_COMPARISON},
    [TOKEN_ID]           = {parse_expr_variable, NULL, PREC_NONE},
    [TOKEN_STRING]       = {parse_literal_string, NULL, PREC_NONE},
    [TOKEN_NUMBER]       = {parse_literal_number, NULL, PREC_NONE},
    [TOKEN_AND]          = {NULL, parse_expr_and, PREC_AND},
    [TOKEN_CLASS]        = {NULL, NULL, PREC_NONE},
    [TOKEN_STATIC]       = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]         = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]        = {parse_literal, NULL, PREC_NONE},
    [TOKEN_FOR]          = {NULL, NULL, PREC_NONE},
    [TOKEN_FN]           = {parse_expr_anon_fn, NULL, PREC_NONE},
    [TOKEN_LAMBDA]       = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]           = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]          = {parse_literal, NULL, PREC_NONE},
    [TOKEN_OR]           = {NULL, parse_expr_or, PREC_OR},
    [TOKEN_PRINT]        = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]       = {NULL, NULL, PREC_NONE},
    [TOKEN_BASE]         = {parse_expr_base, NULL, PREC_NONE},
    [TOKEN_TRY]          = {parse_expr_try, NULL, PREC_NONE},
    [TOKEN_CATCH]        = {NULL, NULL, PREC_NONE},
    [TOKEN_THROW]        = {NULL, NULL, PREC_NONE},
    [TOKEN_IS]           = {NULL, parse_expr_is, PREC_COMPARISON},
    [TOKEN_IN]           = {NULL, parse_expr_in, PREC_COMPARISON},
    [TOKEN_THIS]         = {parse_expr_this, NULL, PREC_NONE},
    [TOKEN_TRUE]         = {parse_literal, NULL, PREC_NONE},
    [TOKEN_LET]          = {NULL, NULL, PREC_NONE},
    [TOKEN_CONST]        = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]        = {NULL, NULL, PREC_NONE},
    [TOKEN_BREAK]        = {NULL, NULL, PREC_NONE},
    [TOKEN_SKIP]         = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR]        = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]          = {NULL, NULL, PREC_NONE},
};

// Returns the rule for the given token type.
static inline ParseRule* get_rule(TokenKind type) {
  return &rules2[type];
}

static inline bool is_meaningful_newline(Parser* parser) {
  // Except if it's a dot, because chaining method calls on newlines is a common pattern and still allowed.
  return parser->current.is_first_on_line && parser->current.type != TOKEN_DOT;
}

// Parses any expression with a precedence higher or equal to the provided precedence. Handles prefix and infix expressions and
// determines whether the expression can be assigned to.
static AstExpression* parse_precedence(Parser* parser, Precedence precedence) {
  advance(parser);

  // Prefix
  ParsePrefixFn prefix_rule = get_rule(parser->previous.type)->prefix;
  if (prefix_rule == NULL) {
    parser_error_at_previous(parser, "Expecting expression.");
    return NULL;
  }
  AstExpression* left = prefix_rule(parser, parser->previous);

  // Infix
  while (!is_meaningful_newline(parser) && precedence <= get_rule(parser->current.type)->precedence) {
    advance(parser);
    ParseInfixFn infix_rule = get_rule(parser->previous.type)->infix;
    left                    = infix_rule(parser, parser->previous, left);
  }
  return left;
}

static AstExpression* parse_expression(Parser* parser) {
  return parse_precedence(parser, PREC_ASSIGN);
}

static AstPattern* parse_destructuring(Parser* parser, DestructureType type) {
  Token pattern_start = parser->previous;  // Previous is '[', '{' or '('

  TokenKind closing;
  PatternType pattern_type;
  switch (type) {
    case DESTRUCTURE_SEQ: {
      closing      = TOKEN_CBRACK;
      pattern_type = PAT_SEQ;
      break;
    }
    case DESTRUCTURE_OBJ: {
      closing      = TOKEN_CBRACE;
      pattern_type = PAT_OBJ;
      break;
    }
    case DESTRUCTURE_TUPLE: {
      closing      = TOKEN_CPAR;
      pattern_type = PAT_TUPLE;
      break;
    }
    default: INTERNAL_ERROR("Unhandled destructuring type."); break;
  }

  int current_index = 0;
  bool has_rest     = false;

  AstPattern* destructure = ast_pattern_init(pattern_start, parser->previous, pattern_type);

  // Parse the bindings.
  while (!check(parser, closing) && !check(parser, TOKEN_EOF)) {
    if (has_rest) {
      parser_error_at_previous(parser, "Rest binding must be last in destructuring assignment.");
    }
    has_rest = match(parser, TOKEN_DOTDOTDOT);
    if (has_rest && type == DESTRUCTURE_OBJ) {
      parser_error_at_previous(parser, "Rest binding is not allowed in " STR(TYPENAME_OBJ) " destructuring assignment.");
    }

    if (has_rest) {
      consume(parser, TOKEN_ID, "Expecting identifier after ellipsis in destructuring.");
      ObjString* rest_name = copy_string(parser->previous.start, parser->previous.length);
      AstId* rest_id       = ast_id_init(parser->previous, rest_name);
      AstPattern* rest     = ast_pattern_rest_init(parser->previous, parser->previous, rest_id);
      ast_pattern_add_element(destructure, rest);
    } else {
      // Either a nested destructuring pattern, or a simple binding/rest binding.
      AstPattern* element = NULL;
      if (match(parser, TOKEN_OBRACK)) {
        element = parse_destructuring(parser, DESTRUCTURE_SEQ);
      } else if (match(parser, TOKEN_OBRACE)) {
        parser_error_at_previous(parser, "Nested " STR(TYPENAME_OBJ) " destructuring is not allowed.");
      } else if (match(parser, TOKEN_OPAR)) {
        element = parse_destructuring(parser, DESTRUCTURE_TUPLE);
      } else {
        consume(parser, TOKEN_ID, "Expecting identifier in destructuring assignment.");
        ObjString* name = copy_string(parser->previous.start, parser->previous.length);
        AstId* id       = ast_id_init(parser->previous, name);
        element         = ast_pattern_binding_init(parser->previous, parser->previous, id);
      }
      ast_pattern_add_element(destructure, element);
    }

    if (++current_index > MAX_DESTRUCTURING_BINDINGS) {
      parser_error_at_previous(parser,
                               "Can't have more than " STR(MAX_DESTRUCTURING_BINDINGS) " bindings in destructuring assignment.");
    }

    if (has_rest) {
      break;
    }

    if (!match(parser, TOKEN_COMMA)) {
      break;
    }
  }

  if (!match(parser, closing)) {
    has_rest ? parser_error_at_previous(parser, "Rest parameter must be last in destructuring assignment.")
             : parser_error_at_previous(parser, "Unterminated destructuring pattern.");
  }

  destructure->base.token_end = parser->previous;
  return destructure;
}

static AstStatement* parse_statement_print(Parser* parser) {
  Token stmt_start          = parser->previous;  // Previous is PRINT
  AstExpression* expression = parse_expression(parser);
  return ast_stmt_print_init(stmt_start, parser->previous, expression);
}

static AstStatement* parse_statement_expression(Parser* parser) {
  Token stmt_start          = parser->previous;  // Previous is the first token of the expression
  AstExpression* expression = parse_expression(parser);
  return ast_stmt_expr_init(stmt_start, parser->previous, expression);
}

static AstStatement* parse_statement_block(Parser* parser) {
  Token stmt_start = parser->previous;  // Previous is OBRACE
  AstBlock* block  = parse_block(parser);
  return ast_stmt_block_init(stmt_start, parser->previous, block);
}

static AstStatement* parse_statement_if(Parser* parser) {
  Token stmt_start          = parser->previous;  // Previous is FOR
  AstExpression* condition  = parse_expression(parser);
  AstStatement* then_branch = parse_statement(parser);
  AstStatement* else_branch = NULL;

  if (match(parser, TOKEN_ELSE)) {
    else_branch = parse_statement(parser);
  }

  return ast_stmt_if_init(stmt_start, parser->previous, condition, then_branch, else_branch);
}

static AstStatement* parse_statement_throw(Parser* parser) {
  Token stmt_start          = parser->previous;  // Previous is THROW
  AstExpression* expression = parse_expression(parser);
  return ast_stmt_throw_init(stmt_start, parser->previous, expression);
}

static AstStatement* parse_statement_try(Parser* parser) {
  Token stmt_start           = parser->previous;  // Previous is TRY
  AstStatement* try_stmt     = parse_statement(parser);
  AstStatement* catch_branch = NULL;

  if (match(parser, TOKEN_CATCH)) {
    catch_branch = parse_statement(parser);
  }
  return ast_stmt_try_init(stmt_start, parser->previous, try_stmt, catch_branch);
}

static AstStatement* parse_statement_while(Parser* parser) {
  Token stmt_start         = parser->previous;  // Previous is WHILE
  AstExpression* condition = parse_expression(parser);
  AstStatement* body       = parse_statement(parser);
  return ast_stmt_while_init(stmt_start, parser->previous, condition, body);
}

static AstStatement* parse_statement_return(Parser* parser) {
  Token stmt_start          = parser->previous;  // Previous is RETURN
  AstExpression* expression = NULL;

  if (!check_statement_return_end(parser)) {
    expression = parse_expression(parser);
  }
  if (!check_statement_return_end(parser)) {
    parser_error_at_current(parser, "Expecting expression, or newline, '}' or some other statement after return.");
  }

  return ast_stmt_return_init(stmt_start, parser->previous, expression);
}

static AstStatement* parse_statement_for(Parser* parser) {
  Token stmt_start         = parser->previous;  // Previous is FOR
  AstNode* initializer     = NULL;
  AstExpression* condition = NULL;
  AstExpression* increment = NULL;
  AstStatement* body       = NULL;

  // Initializer
  if (match(parser, TOKEN_SCOLON)) {
    // No initializer
  } else if (check(parser, TOKEN_LET)) {
    // Let declaration
    initializer = (AstNode*)parse_declaration(parser);
    consume(parser, TOKEN_SCOLON, "Expecting ';' after loop initializer.");
  } else {
    // Statement expression
    initializer = (AstNode*)parse_statement_expression(parser);
    consume(parser, TOKEN_SCOLON, "Expecting ';' after loop initializer.");
  }

  // Loop condition
  if (!match(parser, TOKEN_SCOLON)) {
    condition = parse_expression(parser);
    consume(parser, TOKEN_SCOLON, "Expecting ';' after loop condition.");
  }

  // Loop increment
  if (!match(parser, TOKEN_SCOLON)) {
    increment = parse_expression(parser);
    consume(parser, TOKEN_SCOLON, "Expecting ';' after loop increment.");
  }

  // Loop body
  body = parse_statement(parser);

  return ast_stmt_for_init(stmt_start, parser->previous, initializer, condition, increment, body);
}

static AstStatement* parse_statement_import(Parser* parser) {
  Token stmt_start = parser->previous;  // Previous is IMPORT
  ObjString* path  = NULL;

  if (match(parser, TOKEN_OBRACE)) {
    AstPattern* pattern = parse_destructuring(parser, DESTRUCTURE_OBJ);
    consume(parser, TOKEN_FROM, "Expecting 'from' after destructuring assignment.");
    consume(parser, TOKEN_STRING, "Expecting file path after 'from'.");
    path = copy_string(parser->previous.start + 1, parser->previous.length - 2);  // Skip the quotes.
    return ast_stmt_import_init2(stmt_start, parser->previous, path, pattern);
  }

  consume(parser, TOKEN_ID, "Expecting identifier or '{' after 'import'.");
  ObjString* name = copy_string(parser->previous.start, parser->previous.length);
  AstId* id       = ast_id_init(parser->previous, name);
  if (match(parser, TOKEN_FROM)) {
    consume(parser, TOKEN_STRING, "Expecting file path after 'from'.");
    path = copy_string(parser->previous.start + 1, parser->previous.length - 2);  // Skip the quotes.
  }
  return ast_stmt_import_init(stmt_start, parser->previous, path, id);
}

static AstBlock* parse_block(Parser* parser) {
  Token stmt_start = parser->previous;  // Previous is OBRACE
  AstBlock* block  = ast_block_init(stmt_start);

  while (!check(parser, TOKEN_CBRACE) && !check(parser, TOKEN_EOF)) {
    ast_block_add(block, parse_declaration(parser));
  }

  consume(parser, TOKEN_CBRACE, "Expecting '}' after block.");
  block->base.token_end = parser->previous;
  return block;
}

// Parse a statement.
static AstStatement* parse_statement(Parser* parser) {
  if (match(parser, TOKEN_PRINT)) {
    return parse_statement_print(parser);
  }
  if (match(parser, TOKEN_IF)) {
    return parse_statement_if(parser);
  }
  if (match(parser, TOKEN_THROW)) {
    return parse_statement_throw(parser);
  }
  if (match(parser, TOKEN_TRY)) {
    return parse_statement_try(parser);
  }
  if (match(parser, TOKEN_WHILE)) {
    return parse_statement_while(parser);
  }
  if (match(parser, TOKEN_RETURN)) {
    return parse_statement_return(parser);
  }
  if (match(parser, TOKEN_FOR)) {
    return parse_statement_for(parser);
  }
  if (match(parser, TOKEN_IMPORT)) {
    return parse_statement_import(parser);
  }
  if (match(parser, TOKEN_SKIP)) {
    return ast_stmt_skip_init(parser->previous, parser->previous);
  }
  if (match(parser, TOKEN_BREAK)) {
    return ast_stmt_break_init(parser->previous, parser->previous);
  }
  if (match(parser, TOKEN_OBRACE)) {
    return parse_statement_block(parser);
  }
  return parse_statement_expression(parser);
}

static AstDeclaration* parse_fn_params(Parser* parser) {
  Token decl_start = parser->current;  // Nothing has been consumed yet.

  AstDeclaration* params = ast_decl_fn_params_init(decl_start, decl_start);

  int param_count = 0;
  if (match(parser, TOKEN_OPAR)) {
    if (!check(parser, TOKEN_CPAR)) {  // It's allowed to have "()" with no parameters.
      do {
        if (param_count >= MAX_FN_ARGS) {
          parser_error_at_current(parser, "Can't have more than " STR(MAX_FN_ARGS) " parameters.");
        }

        consume(parser, TOKEN_ID, "Expecting parameter name.");
        ObjString* param_name = copy_string(parser->previous.start, parser->previous.length);
        AstId* param_id       = ast_id_init(parser->previous, param_name);
        ast_decl_fn_params_add_param(params, param_id);
        param_count++;
      } while (match(parser, TOKEN_COMMA));
    }

    if (!match(parser, TOKEN_CPAR)) {
      parser_error_at_current(parser, "Expecting ')' after parameters.");
    }
  }

  params->base.token_end = parser->previous;
  return params;
}

// Consolidated function for parsing a function declaration. Named, anonymous, method, constructor.
static AstFn* parse_function(Parser* parser, Token decl_start, Token name, FnType type) {
  AstId* fn_name         = ast_id_init(decl_start, copy_string(name.start, name.length));
  AstDeclaration* params = parse_fn_params(parser);

  if (match(parser, TOKEN_LAMBDA)) {
    if (type == FN_TYPE_CONSTRUCTOR) {
      parser_error_at_previous(parser, "Constructors can't be lambda functions.");
    } else {
      AstExpression* body = parse_expression(parser);
      return ast_fn_init2(decl_start, parser->previous, type, fn_name, params, body);
    }
  } else if (match(parser, TOKEN_OBRACE)) {
    AstBlock* body = parse_block(parser);
    return ast_fn_init(decl_start, parser->previous, type, fn_name, params, body);
  } else {
    const char* emsg = type == FN_TYPE_CONSTRUCTOR                               ? "Expecting '{' before constructor body."
                       : type == FN_TYPE_METHOD || type == FN_TYPE_METHOD_STATIC ? "Expecting '->' or '{' before method body."
                                                                                 : "Expecting '->' or '{' before function body.";
    parser_error_at_current(parser, emsg);
  }

  return NULL;
}

static AstFn* parse_method(Parser* parser) {
  Token decl_start = parser->current;  // Nothing has been consumed yet.
  FnType type      = match(parser, TOKEN_STATIC) ? FN_TYPE_METHOD_STATIC : FN_TYPE_METHOD;
  consume(parser, TOKEN_FN, "Expecting method initializer.");
  consume(parser, TOKEN_ID, "Expecting method name.");
  return parse_function(parser, decl_start, parser->previous, type);
}

static AstFn* parse_constructor(Parser* parser) {
  Token decl_start = parser->current;  // Nothing has been consumed yet.
  consume(parser, TOKEN_CTOR, "Expecting constructor.");
  return parse_function(parser, decl_start, parser->previous, FN_TYPE_CONSTRUCTOR);
}

static AstDeclaration* parse_declaration_class(Parser* parser) {
  Token decl_start = parser->previous;  // Previous is CLASS
  consume(parser, TOKEN_ID, "Expecting class name.");
  AstId* class_name = ast_id_init(parser->previous, copy_string(parser->previous.start, parser->previous.length));

  AstId* baseclass_name = NULL;
  if (match(parser, TOKEN_COLON)) {
    consume(parser, TOKEN_ID, "Expecting base class name.");
    baseclass_name = ast_id_init(parser->previous, copy_string(parser->previous.start, parser->previous.length));
    if (class_name->name == baseclass_name->name) {
      parser_error_at_previous(parser, "A class can't inherit from itself.");
    }
  }

  AstDeclaration* class = ast_decl_class_init(decl_start, parser->previous, class_name, baseclass_name);

  bool has_ctor = false;
  consume(parser, TOKEN_OBRACE, "Expecting '{' before class body.");
  while (!check(parser, TOKEN_CBRACE) && !check(parser, TOKEN_EOF) && !parser->panic_mode) {
    if (check(parser, TOKEN_CTOR)) {
      if (has_ctor) {
        parser_error_at_previous(parser, "Can't have more than one constructor.");
      }
      ast_decl_class_add_method_or_ctor(class, parse_constructor(parser));
      has_ctor = true;
    } else {
      ast_decl_class_add_method_or_ctor(class, parse_method(parser));
    }
  }

  consume(parser, TOKEN_CBRACE, "Expecting '}' after class body.");
  class->base.token_end = parser->previous;
  return class;
}

static AstDeclaration* parse_declaration_function(Parser* parser) {
  Token decl_start = parser->previous;  // Previous is FN
  consume(parser, TOKEN_ID, "Expecting function name.");
  AstFn* fn = parse_function(parser, decl_start, parser->previous, FN_TYPE_NAMED_FUNCTION);
  return ast_decl_fn_init(decl_start, parser->previous, fn);
}

static AstDeclaration* parse_declaration_variable(Parser* parser) {
  bool is_const    = parser->previous.type == TOKEN_CONST;
  Token decl_start = parser->previous;

  if (match(parser, TOKEN_OBRACK) || match(parser, TOKEN_OBRACE) || match(parser, TOKEN_OPAR)) {
    // Destructuring
    DestructureType type = parser->previous.type == TOKEN_OBRACK   ? DESTRUCTURE_SEQ
                           : parser->previous.type == TOKEN_OBRACE ? DESTRUCTURE_OBJ
                                                                   : DESTRUCTURE_TUPLE;
    AstPattern* pattern  = parse_destructuring(parser, type);
    consume(parser, TOKEN_ASSIGN, "Expecting '=' after destructuring pattern.");
    AstExpression* initializer = parse_expression(parser);
    return ast_decl_variable_init2(decl_start, parser->previous, is_const, pattern, initializer);
  }

  consume(parser, TOKEN_ID, "Expecting variable name.");
  ObjString* name            = copy_string(parser->previous.start, parser->previous.length);
  AstId* id                  = ast_id_init(parser->previous, name);
  AstExpression* initializer = NULL;
  if (match(parser, TOKEN_ASSIGN)) {
    initializer = parse_expression(parser);
  } else if (is_const) {
    parser_error(parser, &decl_start, "Const variable must be initialized.");
  }
  return ast_decl_variable_init(decl_start, parser->previous, is_const, id, initializer);
}

// Parse a declaration.
static AstNode* parse_declaration(Parser* parser) {
  if (match(parser, TOKEN_CLASS)) {
    return (AstNode*)parse_declaration_class(parser);
  }
  if (match(parser, TOKEN_FN)) {
    return (AstNode*)parse_declaration_function(parser);
  }
  if (match(parser, TOKEN_CONST) || match(parser, TOKEN_LET)) {
    return (AstNode*)parse_declaration_variable(parser);
  }

  return (AstNode*)parse_statement(parser);
}

bool parse(const char* source, ObjString* name, AstFn** ast) {
  Parser parser;

  scanner_init(source);
  parser_init(&parser);

  advance(&parser);
  Token start            = parser.current;
  AstId* id              = ast_id_init(parser.current, name);
  AstDeclaration* params = NULL;  // No parameters for the root function. Maybe make some default params for cli by argv/argc?
  AstBlock* body         = ast_block_init(start);

  while (!match(&parser, TOKEN_EOF)) {
    AstNode* decl_or_stmt = parse_declaration(&parser);
    ast_block_add(body, decl_or_stmt);
    if (parser.panic_mode) {
      synchronize(&parser);
    }
  }
  body->base.token_end = parser.previous;
  *ast                 = ast_fn_init(start, parser.previous, FN_TYPE_MODULE, id, params, body);

#ifdef DEBUG_PRINT_AST
  printf("\n\n\n === PARSE ===\n\n");
  ast_print((AstNode*)*ast, 0);
#endif

  return !parser.had_error;
}

void parser_mark_roots() {
  return;  // Nothing to mark, yet.
}
