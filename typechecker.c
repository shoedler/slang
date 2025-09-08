#include "typechecker.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "value.h"
#include "type_rules.h"

static TypeChecker* current_checker = NULL;

// Initialize type checker
static void typechecker_init(TypeChecker* checker, HashTable* global_scope, HashTable* native_scope) {
  checker->had_error = false;
  checker->panic_mode = false;
  checker->global_types = global_scope;
  checker->native_types = native_scope;
}

// Error reporting
void typecheck_error(TypeChecker* checker, AstNode* node, const char* format, ...) {
  if (checker->panic_mode) return;
  
  checker->had_error = true;
  checker->panic_mode = true;
  
  fprintf(stderr, "[Type Error at line %d] ", node->token_start.line);
  
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  
  fprintf(stderr, "\n");
}

// Parse type annotation from identifier
SlangType* parse_type_annotation(AstId* type_id) {
  if (type_id == NULL || type_id->name == NULL) return NULL;
  
  const char* type_name = type_id->name->chars;
  int length = type_id->name->length;
  
  if (strncmp(type_name, "int", length) == 0 && length == 3) {
    return type_create_primitive(TYPE_INT);
  } else if (strncmp(type_name, "flt", length) == 0 && length == 3) {
    return type_create_primitive(TYPE_FLOAT);
  } else if (strncmp(type_name, "str", length) == 0 && length == 3) {
    return type_create_primitive(TYPE_STR);
  } else if (strncmp(type_name, "bool", length) == 0 && length == 4) {
    return type_create_primitive(TYPE_BOOL);
  } else if (strncmp(type_name, "nil", length) == 0 && length == 3) {
    return type_create_primitive(TYPE_NIL);
  }
  
  // TODO: Handle complex types like seq<int>, tuple<int,str>, etc.
  return type_create_primitive(TYPE_UNKNOWN);
}

// Infer type from literal value
SlangType* infer_literal_type(AstLiteral* literal) {
  switch (literal->type) {
    case LIT_NUMBER:
      // For now, check if it's an integer or float based on the literal type
      // This is a simplified check - a full implementation would examine the actual value
      // Check if the value has a fractional part
      if (literal->value.as.float_ == (double)(long long)literal->value.as.float_) {
        return type_create_primitive(TYPE_INT);
      } else {
        return type_create_primitive(TYPE_FLOAT);
      }
    case LIT_STRING:
      return type_create_primitive(TYPE_STR);
    case LIT_BOOL:
      return type_create_primitive(TYPE_BOOL);
    case LIT_NIL:
      return type_create_primitive(TYPE_NIL);
    case LIT_SEQ:
      // TODO: Infer element type
      return type_create_seq(type_create_primitive(TYPE_UNKNOWN));
    case LIT_TUPLE:
      // TODO: Infer element types
      return type_create_primitive(TYPE_TUPLE);
    case LIT_OBJ:
      return type_create_primitive(TYPE_OBJ);
  }
  
  return type_create_primitive(TYPE_UNKNOWN);
}

// Type check literal expression
SlangType* typecheck_literal(TypeChecker* checker, AstLiteral* literal) {
  SlangType* type = infer_literal_type(literal);
  literal->base.slang_type = type;
  return type;
}

// Type check variable expression
SlangType* typecheck_variable_expr(TypeChecker* checker, AstExpression* expr) {
  // TODO: Look up variable type in scope
  // For now, return unknown
  expr->base.slang_type = type_create_primitive(TYPE_UNKNOWN);
  return expr->base.slang_type;
}

// Type check binary expression
SlangType* typecheck_binary_expr(TypeChecker* checker, AstExpression* expr) {
  AstExpression* left = (AstExpression*)expr->base.children[0];
  AstExpression* right = (AstExpression*)expr->base.children[1];
  
  SlangType* left_type = typecheck_expression(checker, left);
  SlangType* right_type = typecheck_expression(checker, right);
  
  // Use the systematic type rule lookup
  TypeKind result_kind = lookup_binary_type_rule(left_type->kind, right_type->kind, expr->operator_.type);
  
  if (result_kind == TYPE_ERROR) {
    typecheck_error(checker, (AstNode*)expr, 
                   "Invalid binary operation '%.*s' between %s and %s", 
                   expr->operator_.length,
                   expr->operator_.start,
                   type_to_string(left_type), 
                   type_to_string(right_type));
    expr->base.slang_type = type_create_primitive(TYPE_ERROR);
    return expr->base.slang_type;
  }
  
  SlangType* result_type = type_create_primitive(result_kind);
  expr->base.slang_type = result_type;
  return result_type;
}

// Type check expression
SlangType* typecheck_expression(TypeChecker* checker, AstExpression* expr) {
  switch (expr->type) {
    case EXPR_LITERAL: {
      AstLiteral* literal = (AstLiteral*)expr->base.children[0];
      return typecheck_literal(checker, literal);
    }
    case EXPR_VARIABLE:
      return typecheck_variable_expr(checker, expr);
    case EXPR_BINARY:
      return typecheck_binary_expr(checker, expr);
    case EXPR_UNARY: {
      AstExpression* inner = (AstExpression*)expr->base.children[0];
      SlangType* inner_type = typecheck_expression(checker, inner);
      
      switch (expr->operator_.type) {
        case TOKEN_MINUS:
          if (inner_type->kind == TYPE_INT || inner_type->kind == TYPE_FLOAT) {
            expr->base.slang_type = inner_type;
            return inner_type;
          }
          break;
        case TOKEN_NEGATE:
          expr->base.slang_type = type_create_primitive(TYPE_BOOL);
          return expr->base.slang_type;
      }
      
      typecheck_error(checker, (AstNode*)expr, 
                     "Invalid unary operation on %s", 
                     type_to_string(inner_type));
      break;
    }
    case EXPR_ASSIGN: {
      AstExpression* left = (AstExpression*)expr->base.children[0];
      AstExpression* right = (AstExpression*)expr->base.children[1];
      
      SlangType* left_type = typecheck_expression(checker, left);
      SlangType* right_type = typecheck_expression(checker, right);
      
      if (!type_is_assignable(left_type, right_type)) {
        typecheck_error(checker, (AstNode*)expr,
                       "Cannot assign %s to %s",
                       type_to_string(right_type),
                       type_to_string(left_type));
      }
      
      expr->base.slang_type = left_type;
      return left_type;
    }
    default:
      // TODO: Handle other expression types
      expr->base.slang_type = type_create_primitive(TYPE_UNKNOWN);
      return expr->base.slang_type;
  }
  
  expr->base.slang_type = type_create_primitive(TYPE_ERROR);
  return expr->base.slang_type;
}

// Type check variable declaration
void typecheck_declaration(TypeChecker* checker, AstDeclaration* decl) {
  switch (decl->type) {
    case DECL_VARIABLE: {
      SlangType* declared_type = NULL;
      SlangType* inferred_type = NULL;
      
      // Parse type annotation if present
      if (decl->type_annotation != NULL) {
        declared_type = parse_type_annotation(decl->type_annotation);
      }
      
      // Type check initializer if present
      if (decl->base.count > 1) {
        AstExpression* initializer = (AstExpression*)decl->base.children[1];
        inferred_type = typecheck_expression(checker, initializer);
      }
      
      // Determine final type
      SlangType* final_type = NULL;
      if (declared_type != NULL && inferred_type != NULL) {
        // Both type annotation and initializer present
        if (!type_is_assignable(declared_type, inferred_type)) {
          typecheck_error(checker, (AstNode*)decl,
                         "Cannot assign %s to variable of type %s",
                         type_to_string(inferred_type),
                         type_to_string(declared_type));
        }
        final_type = declared_type;
      } else if (declared_type != NULL) {
        // Only type annotation present
        final_type = declared_type;
      } else if (inferred_type != NULL) {
        // Only initializer present
        final_type = inferred_type;
      } else {
        // Neither present - error
        typecheck_error(checker, (AstNode*)decl,
                       "Variable declaration must have either type annotation or initializer");
        final_type = type_create_primitive(TYPE_ERROR);
      }
      
      decl->base.slang_type = final_type;
      break;
    }
    default:
      // TODO: Handle other declaration types
      break;
  }
}

// Type check statement
void typecheck_statement(TypeChecker* checker, AstStatement* stmt) {
  // TODO: Implement statement type checking
  switch (stmt->type) {
    case STMT_EXPR: {
      if (stmt->base.count > 0) {
        AstExpression* expr = (AstExpression*)stmt->base.children[0];
        typecheck_expression(checker, expr);
      }
      break;
    }
    default:
      break;
  }
}

// Type check AST node
SlangType* typecheck_node(TypeChecker* checker, AstNode* node) {
  switch (node->type) {
    case NODE_EXPR: {
      AstExpression* expr = (AstExpression*)node;
      return typecheck_expression(checker, expr);
    }
    case NODE_DECL: {
      AstDeclaration* decl = (AstDeclaration*)node;
      typecheck_declaration(checker, decl);
      return decl->base.slang_type;
    }
    case NODE_STMT: {
      AstStatement* stmt = (AstStatement*)node;
      typecheck_statement(checker, stmt);
      return NULL;
    }
    case NODE_BLOCK: {
      // Type check all children
      for (int i = 0; i < node->count; i++) {
        typecheck_node(checker, node->children[i]);
      }
      return NULL;
    }
    default:
      // Recursively type check children
      for (int i = 0; i < node->count; i++) {
        typecheck_node(checker, node->children[i]);
      }
      return NULL;
  }
}

// Main type checking function
bool typecheck(AstFn* ast, HashTable* global_scope, HashTable* native_scope) {
  current_checker = NULL;
  
  TypeChecker checker;
  typechecker_init(&checker, global_scope, native_scope);
  current_checker = &checker;
  
  // Type check the function body
  if (ast->base.count >= 3) {
    AstNode* body = ast->base.children[2];
    typecheck_node(&checker, body);
  }
  
  current_checker = NULL;
  return !checker.had_error;
}

// Get type at position (for LSP support)
SlangType* get_type_at_pos(AstNode* ast, int line, int col) {
  // TODO: Implement position-based type lookup
  // This would need to traverse the AST and find the node at the given position
  return type_create_primitive(TYPE_UNKNOWN);
}