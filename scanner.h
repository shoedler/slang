#ifndef scanner_h
#define scanner_h

// Hexadecimal (base-16) digits can represent 4 bits each (since 16=2^4). Given the 53-bit precision of a
// double, the longest hexadecimal literal that can fit without loss of precision would be 53/4=13.25 digits.
// Since we cannot have a fraction of a digit, the practical maximum length is 13 digits (excluding any prefix
// like 0x), but remember the last digit can only partially contribute to the precision, effectively meaning
// the fully precise range is limited to the lower 12 digits with the 13th digit being constrained.
#define MAX_HEX_DIGITS 12

// Binary literals (base-2) , the maximum length without losing precision in a double is 53 bits. So, a binary
// literal can have up to 53 digits (1s or 0s).
#define MAX_BINARY_DIGITS 53

// Octal (base-8) digits can represent 3 bits each (since 8=2^3). Given the 53-bit precision of a double, the
// longest octal literal without losing precision would be 53/3≈17.67 digits. This means you can have up to 17
// digits in an octal literal to fit into a double without loss of precision, with similar considerations for
// the last digit as in the hexadecimal case.
#define MAX_OCTAL_DIGITS 17
typedef enum {
  TOKEN_OR,       // 'or'
  TOKEN_AND,      // 'and'
  TOKEN_EQ,       // '=='
  TOKEN_NEQ,      // '!='
  TOKEN_GT,       // '>'
  TOKEN_LT,       // '<'
  TOKEN_GTEQ,     // '>='
  TOKEN_LTEQ,     // '<='
  TOKEN_PLUS,     // '+'
  TOKEN_MINUS,    // '-'
  TOKEN_MULT,     // '*'
  TOKEN_DIV,      // '/'
  TOKEN_MOD,      // '%'
  TOKEN_NOT,      // '!'
  TOKEN_TERNARY,  // '?'

  TOKEN_PLUS_PLUS,    // '++'
  TOKEN_MINUS_MINUS,  // '--'

  TOKEN_DOT,        // '.'
  TOKEN_DOTDOT,     // '..'
  TOKEN_DOTDOTDOT,  // '...'
  TOKEN_COMMA,      // ','
  TOKEN_COLON,      // ':'
  TOKEN_SCOLON,     // ';'
  TOKEN_ASSIGN,     // '='
  TOKEN_OPAR,       // '('
  TOKEN_CPAR,       // ')'
  TOKEN_OBRACE,     // '{'
  TOKEN_CBRACE,     // '}'
  TOKEN_OBRACK,     // '['
  TOKEN_CBRACK,     // ']'

  TOKEN_PLUS_ASSIGN,   // '+='
  TOKEN_MINUS_ASSIGN,  // '-='
  TOKEN_MULT_ASSIGN,   // '*='
  TOKEN_DIV_ASSIGN,    // '/='
  TOKEN_MOD_ASSIGN,    // '%='

  TOKEN_LAMBDA,  // '->'

  TOKEN_TRUE,    // 'true'
  TOKEN_FALSE,   // 'false'
  TOKEN_NIL,     // 'nil'
  TOKEN_IF,      // 'if'
  TOKEN_IMPORT,  // 'import'
  TOKEN_FROM,    // 'from'
  TOKEN_ELSE,    // 'else'
  TOKEN_WHILE,   // 'while'
  TOKEN_FOR,     // 'for'
  TOKEN_BREAK,   // 'break'
  TOKEN_SKIP,    // 'skip'
  TOKEN_CLASS,   // 'class'
  TOKEN_STATIC,  // 'static'
  TOKEN_THIS,    // 'this'
  TOKEN_PRINT,   // 'print'
  TOKEN_FN,      // 'fn'
  TOKEN_RETURN,  // 'ret'
  TOKEN_LET,     // 'let'
  TOKEN_CTOR,    // 'ctor'
  TOKEN_BASE,    // 'base'
  TOKEN_TRY,     // 'try'
  TOKEN_THROW,   // 'throw'
  TOKEN_CATCH,   // 'catch'
  TOKEN_IS,      // 'is'
  TOKEN_IN,      // 'in'

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
  bool is_first_on_line;
} Token;

// Initialize the scanner with the source code
void init_scanner(const char* source);

// Scan and return the next token
Token scan_token();

#endif