#include "type_rules.h"

// Lookup function for binary operation type rules
TypeKind lookup_binary_type_rule(TypeKind left, TypeKind right, TokenKind op) {
  for (size_t i = 0; i < BINARY_TYPE_RULES_COUNT; i++) {
    const TypeRule* rule = &binary_type_rules[i];
    if (rule->left_type == left && rule->right_type == right && rule->op == op) {
      return rule->result_type;
    }
  }
  
  // Special case for equality operations - they work on any types
  if (op == TOKEN_EQ || op == TOKEN_NEQ) {
    return TYPE_BOOL;
  }
  
  // Special case for logical operations
  if (op == TOKEN_AND || op == TOKEN_OR) {
    return TYPE_BOOL;
  }
  
  return TYPE_ERROR;  // No rule found
}