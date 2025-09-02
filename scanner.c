#include <string.h>

#include "common.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_TOKENS
#include <stdio.h>
#endif

typedef struct {
  const char* start;
  const char* current;
  int line;
  bool is_first_on_line;
} Scanner;

Scanner scanner;
static const char* first_source_char;

void scanner_init(const char* source) {
  scanner.start   = source;
  scanner.current = source;
  scanner.line    = 1;

  first_source_char = source;
}

const char* scanner_get_line_start(Token token) {
  const char* line_start = token.start;
  while (line_start > first_source_char && line_start[-1] != '\n') {
    line_start--;
  }
  return line_start;
}

static bool is_digit(char chr) {
  return chr >= '0' && chr <= '9';
}

static bool is_alpha(char chr) {
  return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z') || chr == '_';
}

static bool is_at_end() {
  return *scanner.current == '\0';
}

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static char peek() {
  return *scanner.current;
}

static char peek_next() {
  if (is_at_end()) {
    return '\0';
  }
  return scanner.current[1];
}

static bool match(char expected) {
  if (is_at_end()) {
    return false;
  }

  if (*scanner.current != expected) {
    return false;
  }
  scanner.current++;
  return true;
}

static Token make_token(TokenKind type) {
  Token token;
  token.type             = type;
  token.start            = scanner.start;
  token.length           = (int)(scanner.current - scanner.start);
  token.line             = scanner.line;
  token.is_first_on_line = scanner.is_first_on_line;

#ifdef DEBUG_PRINT_TOKENS
  printf("TOKEN: %d %.*s\n", token.type, token.length, token.start);
#endif

  return token;
}

static Token error_token(const char* message) {
  Token token;
  token.type             = TOKEN_ERROR;
  token.start            = message;
  token.length           = (int)strlen(message);
  token.line             = scanner.line;
  token.is_first_on_line = scanner.is_first_on_line;
  return token;
}

static void skip_whitespace() {
  for (;;) {
    char chr = peek();
    switch (chr) {
      case ' ':
      case '\r':
      case '\t': advance(); break;
      case '\n':
        scanner.is_first_on_line = true;
        scanner.line++;
        advance();
        break;
      case '/':
        if (peek_next() == '/') {
          // A comment goes until the end of the line.
          while (peek() != '\n' && !is_at_end()) {
            advance();
          }
        } else {
          return;
        }
        break;
      default: return;
    }
  }
}

static TokenKind check_keyword(int start, int length, const char* rest, TokenKind type) {
  if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_ID;
}

static TokenKind identifier_type() {
  switch (scanner.start[0]) {
    case 'a': return check_keyword(1, 2, "nd", TOKEN_AND);
    case 'b': {
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return check_keyword(2, 2, "se", TOKEN_BASE);
          case 'r': return check_keyword(2, 3, "eak", TOKEN_BREAK);
        }
      }
      break;
    }
    case 'c': {
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return check_keyword(2, 3, "tch", TOKEN_CATCH);
          case 'l': return check_keyword(2, 1, "s", TOKEN_CLASS);
          case 'o': return check_keyword(2, 3, "nst", TOKEN_CONST);
          case 't': return check_keyword(2, 2, "or", TOKEN_CTOR);
        }
      }
      break;
    }
    case 'e':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'l': return check_keyword(2, 2, "se", TOKEN_ELSE);
        }
      }
      break;
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return check_keyword(2, 1, "r", TOKEN_FOR);
          case 'n': return check_keyword(2, 0, "", TOKEN_FN);
          case 'r': return check_keyword(2, 2, "om", TOKEN_FROM);
        }
      }
      break;
    case 'i':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'f': return check_keyword(2, 0, "", TOKEN_IF);
          case 's': return check_keyword(2, 0, "", TOKEN_IS);
          case 'm': return check_keyword(2, 4, "port", TOKEN_IMPORT);
          case 'n': return check_keyword(2, 0, "", TOKEN_IN);
        }
      }
      break;
    case 'n':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'i': return check_keyword(2, 1, "l", TOKEN_NIL);
          case 'o': return check_keyword(2, 1, "t", TOKEN_NOT);
        }
      }
      break;
    case 'o': return check_keyword(1, 1, "r", TOKEN_OR);
    case 'p': return check_keyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return check_keyword(1, 2, "et", TOKEN_RETURN);
    case 's':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'k': return check_keyword(2, 2, "ip", TOKEN_SKIP);
          case 't': return check_keyword(2, 4, "atic", TOKEN_STATIC);
        }
      }
      break;
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h':
            if (scanner.current - scanner.start > 2) {
              switch (scanner.start[2]) {
                case 'i': return check_keyword(3, 1, "s", TOKEN_THIS);
                case 'r': return check_keyword(3, 2, "ow", TOKEN_THROW);
              }
            }
            break;
          case 'r':
            if (scanner.current - scanner.start > 2) {
              switch (scanner.start[2]) {
                case 'u': return check_keyword(3, 1, "e", TOKEN_TRUE);
                case 'y': return check_keyword(3, 0, "", TOKEN_TRY);
              }
            }
            break;
        }
      }
      break;
    case 'l': return check_keyword(1, 2, "et", TOKEN_LET);
    case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
  }

  return TOKEN_ID;
}

static Token identifier() {
  while (is_alpha(peek()) || is_digit(peek())) {
    advance();
  }

  return make_token(identifier_type());
}

static Token decimal() {
  while (is_digit(peek())) {
    advance();
  }

  // Look for a fractional part.
  if (peek() == '.' && is_digit(peek_next())) {
    advance();  // Consume the ".".

    while (is_digit(peek())) {
      advance();
    }
  }

#ifdef DEBUG_PRINT_TOKENS
  printf("NUMBER: %.*s\n", (int)(scanner.current - scanner.start), scanner.start);
#endif

  return make_token(TOKEN_NUMBER);
}

static Token number(char chr) {
  if (chr != '0') {
    return decimal();
  }

  Token number_token;
  char kind = peek();
  switch (kind) {
    case 'x':
    case 'X': {  // Hexadecimal
      advance();
      int num_digits = 0;
      while (is_digit(peek()) || (peek() >= 'a' && peek() <= 'f') || (peek() >= 'A' && peek() <= 'F')) {
        advance();
        num_digits++;
      }
      // Check literal length - this does not account for the actual value that results when parsing the literal
      if (num_digits <= 0 || num_digits > MAX_HEX_DIGITS) {
        return error_token("Hexadecimal number literal must have at least one digit/letter and at most " STR(MAX_HEX_DIGITS) ".");
      }
      number_token = make_token(TOKEN_NUMBER);
      break;
    }
    case 'b':  // Binary
    case 'B': {
      advance();
      int num_digits = 0;
      while (peek() == '0' || peek() == '1') {
        advance();
        num_digits++;
      }
      // Check literal length - this does not account for the actual value that results when parsing the literal
      if (num_digits <= 0 || num_digits > MAX_BINARY_DIGITS) {
        return error_token("Binary number literal must have at least one digit and at most " STR(MAX_BINARY_DIGITS) ".");
      }
      number_token = make_token(TOKEN_NUMBER);
      break;
    }
    case 'o':  // Octal
    case 'O': {
      advance();
      int num_digits = 0;
      while (peek() >= '0' && peek() <= '7') {
        advance();
        num_digits++;
      }
      // Check literal length - this does not account for the actual value that results when parsing the literal
      if (num_digits <= 0 || num_digits > MAX_OCTAL_DIGITS) {
        return error_token("Octal number literal must have at least one digit and at most " STR(MAX_OCTAL_DIGITS) ".");
      }
      number_token = make_token(TOKEN_NUMBER);
      break;
    }

    default: return decimal();  // Otherwise, it's just a decimal
  }

#ifdef DEBUG_PRINT_TOKENS
  printf("NUMBER: %.*s\n", (int)(scanner.current - scanner.start), scanner.start);
#endif

  return number_token;
}

static Token string() {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n') {
      scanner.line++;
    }
    // Handle escape characters, accept any character after a backslash.
    if (peek() == '\\') {
      advance();
    }

    advance();
  }

  if (is_at_end()) {
    return error_token("Unterminated string.");
  }

  advance();  // Consume the closing ".

#ifdef DEBUG_PRINT_TOKENS
  printf("STRING: %.*s\n", (int)(scanner.current - scanner.start), scanner.start);
#endif

  return make_token(TOKEN_STRING);
}

Token scanner_scan_token() {
  scanner.is_first_on_line = false;

  skip_whitespace();
  scanner.start = scanner.current;

  if (is_at_end()) {
    return make_token(TOKEN_EOF);
  }

  char chr = advance();

  if (is_digit(chr)) {
    return number(chr);
  }

  if (is_alpha(chr)) {
    return identifier();
  }

  switch (chr) {
    case '(': return make_token(TOKEN_OPAR);
    case ')': return make_token(TOKEN_CPAR);
    case '{': return make_token(TOKEN_OBRACE);
    case '}': return make_token(TOKEN_CBRACE);
    case '[': return make_token(TOKEN_OBRACK);
    case ']': return make_token(TOKEN_CBRACK);
    case '.': return make_token(match('.') ? match('.') ? TOKEN_DOTDOTDOT : TOKEN_DOTDOT : TOKEN_DOT);
    case ':': return make_token(TOKEN_COLON);
    case ';': return make_token(TOKEN_SCOLON);
    case ',': return make_token(TOKEN_COMMA);
    case '?': return make_token(TOKEN_TERNARY);

    case '+': return make_token(match('=') ? TOKEN_PLUS_ASSIGN : match('+') ? TOKEN_PLUS_PLUS : TOKEN_PLUS);
    case '-':
      return make_token(match('>')   ? TOKEN_LAMBDA
                        : match('-') ? TOKEN_MINUS_MINUS
                        : match('=') ? TOKEN_MINUS_ASSIGN
                                     : TOKEN_MINUS);
    case '/': return make_token(match('=') ? TOKEN_DIV_ASSIGN : TOKEN_DIV);
    case '*': return make_token(match('=') ? TOKEN_MULT_ASSIGN : TOKEN_MULT);
    case '%': return make_token(match('=') ? TOKEN_MOD_ASSIGN : TOKEN_MOD);

    case '=': return make_token(match('=') ? TOKEN_EQ : TOKEN_ASSIGN);
    case '!': return make_token(match('=') ? TOKEN_NEQ : TOKEN_NEGATE);
    case '<': return make_token(match('=') ? TOKEN_LTEQ : TOKEN_LT);
    case '>': return make_token(match('=') ? TOKEN_GTEQ : TOKEN_GT);

    case '"': return string();
  }

  return error_token("Unexpected character.");
}
