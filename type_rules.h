#ifndef TYPE_RULES_H
#define TYPE_RULES_H

#include "type.h"
#include "scanner.h"

// Macro-based type rules extracted from native methods
// This allows us to reuse the same logic for both runtime and compile-time

// Binary operation type rules
#define DEFINE_TYPE_RULE(left_type, right_type, op, result_type) \
  { TYPE_##left_type, TYPE_##right_type, TOKEN_##op, TYPE_##result_type },

typedef struct {
  TypeKind left_type;
  TypeKind right_type;
  TokenKind op;
  TypeKind result_type;
} TypeRule;

// Type rules array (based on native method implementations)
static const TypeRule binary_type_rules[] = {
  // Integer operations (from int_add, int_sub, etc.)
  DEFINE_TYPE_RULE(INT, INT, PLUS, INT)
  DEFINE_TYPE_RULE(INT, FLOAT, PLUS, FLOAT)
  DEFINE_TYPE_RULE(INT, INT, MINUS, INT)
  DEFINE_TYPE_RULE(INT, FLOAT, MINUS, FLOAT)
  DEFINE_TYPE_RULE(INT, INT, MULT, INT)
  DEFINE_TYPE_RULE(INT, FLOAT, MULT, FLOAT)
  DEFINE_TYPE_RULE(INT, INT, DIV, INT)
  DEFINE_TYPE_RULE(INT, FLOAT, DIV, FLOAT)
  DEFINE_TYPE_RULE(INT, INT, MOD, INT)
  DEFINE_TYPE_RULE(INT, FLOAT, MOD, FLOAT)
  
  // Float operations (from float_add, float_sub, etc.)
  DEFINE_TYPE_RULE(FLOAT, INT, PLUS, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, PLUS, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, INT, MINUS, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, MINUS, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, INT, MULT, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, MULT, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, INT, DIV, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, DIV, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, INT, MOD, FLOAT)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, MOD, FLOAT)
  
  // String operations (from str_add, etc.)
  DEFINE_TYPE_RULE(STR, STR, PLUS, STR)
  
  // Comparison operations (return bool)
  DEFINE_TYPE_RULE(INT, INT, LT, BOOL)
  DEFINE_TYPE_RULE(INT, FLOAT, LT, BOOL)
  DEFINE_TYPE_RULE(FLOAT, INT, LT, BOOL)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, LT, BOOL)
  DEFINE_TYPE_RULE(STR, STR, LT, BOOL)
  
  DEFINE_TYPE_RULE(INT, INT, GT, BOOL)
  DEFINE_TYPE_RULE(INT, FLOAT, GT, BOOL)
  DEFINE_TYPE_RULE(FLOAT, INT, GT, BOOL)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, GT, BOOL)
  DEFINE_TYPE_RULE(STR, STR, GT, BOOL)
  
  DEFINE_TYPE_RULE(INT, INT, LTEQ, BOOL)
  DEFINE_TYPE_RULE(INT, FLOAT, LTEQ, BOOL)
  DEFINE_TYPE_RULE(FLOAT, INT, LTEQ, BOOL)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, LTEQ, BOOL)
  DEFINE_TYPE_RULE(STR, STR, LTEQ, BOOL)
  
  DEFINE_TYPE_RULE(INT, INT, GTEQ, BOOL)
  DEFINE_TYPE_RULE(INT, FLOAT, GTEQ, BOOL)
  DEFINE_TYPE_RULE(FLOAT, INT, GTEQ, BOOL)
  DEFINE_TYPE_RULE(FLOAT, FLOAT, GTEQ, BOOL)
  DEFINE_TYPE_RULE(STR, STR, GTEQ, BOOL)
};

#define BINARY_TYPE_RULES_COUNT (sizeof(binary_type_rules) / sizeof(TypeRule))

// Lookup function for binary operation type rules
TypeKind lookup_binary_type_rule(TypeKind left, TypeKind right, TokenKind op);

#endif // TYPE_RULES_H