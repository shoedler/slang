#ifndef scanner_h
#define scanner_h

typedef enum {
  TOKEN_OR,     // 'or'
  TOKEN_AND,    // 'and'
  TOKEN_EQ,     // '=='
  TOKEN_NEQ,    // '!='
  TOKEN_GT,     // '>'
  TOKEN_LT,     // '<'
  TOKEN_GTEQ,   // '>='
  TOKEN_LTEQ,   // '<='
  TOKEN_PLUS,   // '+'
  TOKEN_MINUS,  // '-'
  TOKEN_MULT,   // '*'
  TOKEN_DIV,    // '/'
  TOKEN_MOD,    // '%'
  TOKEN_NOT,    // '!'

  TOKEN_DOT,     // '.'
  TOKEN_COMMA,   // ','
  TOKEN_COLON,   // ':'
  TOKEN_SCOLON,  // ';'
  TOKEN_ASSIGN,  // '='
  TOKEN_OPAR,    // '('
  TOKEN_CPAR,    // ')'
  TOKEN_OBRACE,  // '{'
  TOKEN_CBRACE,  // '}'
  TOKEN_OBRACK,  // '['
  TOKEN_CBRACK,  // ']'

  TOKEN_LAMBDA,  // '->'

  TOKEN_TRUE,      // 'true'
  TOKEN_FALSE,     // 'false'
  TOKEN_NIL,       // 'nil'
  TOKEN_IF,        // 'if'
  TOKEN_ELSE,      // 'else'
  TOKEN_WHILE,     // 'while'
  TOKEN_FOR,       // 'for'
  TOKEN_BREAK,     // 'break'
  TOKEN_CONTINUE,  // 'continue'
  TOKEN_CLASS,     // 'class'
  TOKEN_THIS,      // 'this'
  TOKEN_PRINT,     // 'print'
  TOKEN_FN,        // 'fn'
  TOKEN_RETURN,    // 'ret'
  TOKEN_LET,       // 'let'
  TOKEN_CTOR,      // 'ctor'
  TOKEN_BASE,      // 'base'

  TOKEN_ID,      // [a-zA-Z_] [a-zA-Z_0-9]*
  TOKEN_NUMBER,  // [0-9]+  or [0-9]+ '.' [0-9]* | '.' [0-9]+
  TOKEN_STRING,  // '"' (~["\r\n] | '""')* '"'
  TOKEN_OTHER,   // .
  TOKEN_ERROR,
  TOKEN_EOF
} TokenType;

// Token type
typedef struct {
  TokenType type;
  const char* start;
  int length;
  int line;
} Token;

// Initialize the scanner with the source code
void init_scanner(const char* source);

// Scan and return the next token
Token scan_token();

#endif