#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include <stdbool.h>
#include "ast.h"
#include "type.h"
#include "hashtable.h"

typedef struct TypeChecker {
  bool had_error;
  bool panic_mode;
  HashTable* global_types;   // Global type environment
  HashTable* native_types;   // Built-in type environment
} TypeChecker;

// Initialize and run type checking
bool typecheck(AstFn* ast, HashTable* global_scope, HashTable* native_scope);

// Type checking for different node types
SlangType* typecheck_node(TypeChecker* checker, AstNode* node);
SlangType* typecheck_expression(TypeChecker* checker, AstExpression* expr);
SlangType* typecheck_literal(TypeChecker* checker, AstLiteral* literal);
SlangType* typecheck_binary_expr(TypeChecker* checker, AstExpression* expr);
SlangType* typecheck_variable_expr(TypeChecker* checker, AstExpression* expr);

// Type checking for declarations and statements
void typecheck_declaration(TypeChecker* checker, AstDeclaration* decl);
void typecheck_statement(TypeChecker* checker, AstStatement* stmt);

// Utility functions for type annotation parsing
SlangType* parse_type_annotation(AstId* type_id);
SlangType* infer_literal_type(AstLiteral* literal);

// Error reporting
void typecheck_error(TypeChecker* checker, AstNode* node, const char* format, ...);

// Get type at position (for LSP support)
SlangType* get_type_at_pos(AstNode* ast, int line, int col);

#endif // TYPECHECKER_H