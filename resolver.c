#include "resolver.h"
#include <stdarg.h>
#include <stdbool.h>
#include "ast.h"
#include "chunk.h"
#include "object.h"
#include "scope.h"

typedef struct {
  Scope* current_scope;
  bool in_function;
  bool in_method;
  bool in_constructor;
  bool in_loop;
  bool had_error;

  // Track classes for base/this validation
  struct {
    bool has_baseclass;
    bool is_static;
  } current_class;
} Resolver;

// Forward declarations
void resolve_children(Resolver* resolver, AstNode* node);
static void resolve_node(Resolver* resolver, AstNode* node);

static void resolver_init(Resolver* resolver, Scope* scope) {
  resolver->current_scope               = scope;
  resolver->in_function                 = false;
  resolver->in_method                   = false;
  resolver->in_constructor              = false;
  resolver->in_loop                     = false;
  resolver->had_error                   = false;
  resolver->current_class.has_baseclass = false;
  resolver->current_class.is_static     = false;
}

#ifdef DEBUG_RESOLVER
static inline AstId* get_child_as_id(AstNode* parent, int index, bool allow_null) {
  AstNode* child = parent->children[index];
  if (child == NULL && !allow_null) {
    INTERNAL_ERROR("Expected " STR(AstId) "child node to be non-null.");
  }
  if (child->type != NODE_ID) {
    INTERNAL_ERROR("Expected child node to be of type " STR(NODE_ID) ", got %d.", child->type);
  }
  return (AstId*)parent->children[index];
}
#else
#define get_child_as_id(parent, index, allow_null) ((AstId*)parent->children[index])
#endif

// Report a resolver error.
static void handle_resolver_error(AstNode* offending_node, const char* format, va_list args) {
  fprintf(stderr, "Resolver Error at line %d", offending_node->token_start.line);
  fprintf(stderr, ": " ANSI_COLOR_RED);
  vfprintf(stderr, format, args);
  fprintf(stderr, ANSI_COLOR_RESET "\n");

  SourceView source = chunk_make_source_view(offending_node->token_start, offending_node->token_end);
  report_error_location(source);
}

// Prints an error message at the offending node.
static void resolver_error(AstNode* offending_node, const char* format, ...) {
  va_list args;
  va_start(args, format);
  handle_resolver_error(offending_node, format, args);
  va_end(args);
}

// Declare a new variable in the current scope
static void declare_variable(Resolver* resolver, AstId* local_var, bool is_const) {
  Symbol* declared = scope_get(resolver->current_scope, local_var->name);
  if (declared != NULL) {
    resolver_error((AstNode*)local_var, "Already a variable with this name in this scope.");
    return;
  }

  if (resolver->current_scope->count >= MAX_LOCALS) {
    resolver_error((AstNode*)local_var, "Too many local variables in scope.");
    return;
  }

  // Add the new local
  if (!scope_add_new(resolver->current_scope, local_var->name, is_const, false, false)) {
    resolver_error((AstNode*)local_var, "Variable '%s' is already defined.", local_var->name->chars);
  }
}

// Define a previously declared variable
static void define_variable(Resolver* resolver, AstId* local_var) {
  // Find the variable's declaration
  Symbol* declaration = scope_get(resolver->current_scope, local_var->name);
  if (declaration == NULL) {
    INTERNAL_ERROR("Could not find local variable in current scope.");
    return;
  }
  declaration->is_initialized = true;
}

static void resolve_declare_function(Resolver* resolver, AstDeclaration* decl) {
  UNUSED(resolver);
  UNUSED(decl);
  printf("Resolving decl fn\n");
  resolve_children(resolver, (AstNode*)decl);
}
static void resolve_declare_function_params(Resolver* resolver, AstDeclaration* decl) {
  UNUSED(resolver);
  UNUSED(decl);
  printf("Resolving decl fn params\n");
  resolve_children(resolver, (AstNode*)decl);
}
static void resolve_declare_class(Resolver* resolver, AstDeclaration* decl) {
  UNUSED(resolver);
  UNUSED(decl);
  printf("Resolving decl class\n");
  resolve_children(resolver, (AstNode*)decl);
}
static void resolve_declare_method(Resolver* resolver, AstDeclaration* decl) {
  UNUSED(resolver);
  UNUSED(decl);
  printf("Resolving decl method\n");
  resolve_children(resolver, (AstNode*)decl);
}
static void resolve_declare_constructor(Resolver* resolver, AstDeclaration* decl) {
  UNUSED(resolver);
  UNUSED(decl);
  printf("Resolving decl ctor\n");
  resolve_children(resolver, (AstNode*)decl);
}
static void resolve_declare_variable(Resolver* resolver, AstDeclaration* decl) {
  printf("Resolving decl var\n");

  if (decl->base.children[0]->type == NODE_PATTERN) {
    INTERNAL_ERROR("Only simple variable declarations are supported for now.");
    return;
  }

  // Declare in current scope
  AstId* id = get_child_as_id((AstNode*)decl, 0, false);
  declare_variable(resolver, id, decl->is_const);

  // Resolve initializer if present
  AstNode* initializer = decl->base.children[1];
  if (initializer != NULL) {
    resolve_node(resolver, initializer);
  }

  // Define the variable
  define_variable(resolver, id);
}

static void resolve_statement_import(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt import\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_block(Resolver* resolver, AstStatement* stmt) {
  UNUSED(stmt);
  printf("Resolving stmt block\n");

  resolver->current_scope = stmt->scope;
  resolve_children(resolver, (AstNode*)stmt);
  resolver->current_scope = stmt->scope->enclosing;
}
static void resolve_statement_if(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt if\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_while(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt while\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_for(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt for\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_return(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt return\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_print(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt print\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_expr(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt expr\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_break(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt break\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_skip(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt skip\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_throw(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt throw\n");
  resolve_children(resolver, (AstNode*)stmt);
}
static void resolve_statement_try(Resolver* resolver, AstStatement* stmt) {
  UNUSED(resolver);
  UNUSED(stmt);
  printf("Resolving stmt try\n");
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_expr_binary(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr binary\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_postfix(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr postfix\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_unary(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr unary\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_grouping(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr grouping\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_literal(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr literal\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_variable(Resolver* resolver, AstExpression* expr) {
  printf("Resolving expr var\n");

  AstId* id = get_child_as_id((AstNode*)expr, 0, false);

  // Look up variable in scope chain
  Symbol* var = NULL;
  for (Scope* scope = resolver->current_scope; scope != NULL; scope = scope->enclosing) {
    var = scope_get(scope, id->name);
    if (var != NULL) {
      id->base.depth       = scope->depth;
      id->base.slot        = var->index;
      id->base.is_captured = var->is_captured;
      id->base.is_const    = var->is_const;
      id->base.is_resolved = true;
      return;
    }
  }
}
static void resolve_expr_assign(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr assign\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_and(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr and\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_or(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr or\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_is(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr is\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_in(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr in\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_call(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr call\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_dot(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr dot\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_subs(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr subs\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_slice(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr slice\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_this(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr this\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_base(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr base\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_lambda(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr lambda\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_ternary(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr ternary\n");
  resolve_children(resolver, (AstNode*)expr);
}
static void resolve_expr_try(Resolver* resolver, AstExpression* expr) {
  UNUSED(resolver);
  UNUSED(expr);
  printf("Resolving expr try\n");
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_lit_number(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit num\n");
}
static void resolve_lit_string(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit string\n");
}
static void resolve_lit_bool(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit bool\n");
}
static void resolve_lit_nil(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit nil\n");
}
static void resolve_lit_tuple(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit tuple\n");
  resolve_children(resolver, (AstNode*)lit);
}
static void resolve_lit_seq(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit seq\n");
  resolve_children(resolver, (AstNode*)lit);
}
static void resolve_lit_obj(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
  printf("Resolving lit obj\n");
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_pattern_tuple(Resolver* resolver, AstPattern* pattern) {
  UNUSED(resolver);
  UNUSED(pattern);
  printf("Resolving pattern tuple\n");
  resolve_children(resolver, (AstNode*)pattern);
}
static void resolve_pattern_seq(Resolver* resolver, AstPattern* pattern) {
  UNUSED(resolver);
  UNUSED(pattern);
  printf("Resolving pattern seq\n");
  resolve_children(resolver, (AstNode*)pattern);
}
static void resolve_pattern_obj(Resolver* resolver, AstPattern* pattern) {
  UNUSED(resolver);
  UNUSED(pattern);
  printf("Resolving pattern obj\n");
  resolve_children(resolver, (AstNode*)pattern);
}
static void resolve_pattern_rest(Resolver* resolver, AstPattern* pattern) {
  UNUSED(resolver);
  UNUSED(pattern);
  printf("Resolving pattern rest\n");
}

static void resolve_node(Resolver* resolver, AstNode* node) {
  switch (node->type) {
    case NODE_ROOT: resolve_children(resolver, node); break;
    case NODE_ID: INTERNAL_ERROR("Should not resolve " STR(NODE_ID) " directly."); break;
    case NODE_DECL: {
      AstDeclaration* decl = (AstDeclaration*)node;
      switch (decl->type) {
        case DECL_FN: resolve_declare_function(resolver, decl); break;
        case DECL_FN_PARAMS: resolve_declare_function_params(resolver, decl); break;
        case DECL_CLASS: resolve_declare_class(resolver, decl); break;
        case DECL_METHOD: resolve_declare_method(resolver, decl); break;
        case DECL_CTOR: resolve_declare_constructor(resolver, decl); break;
        case DECL_VARIABLE: resolve_declare_variable(resolver, decl); break;
        default: INTERNAL_ERROR("Unhandled declaration type."); break;
      }
      break;
    }
    case NODE_STMT: {
      AstStatement* stmt = (AstStatement*)node;
      switch (stmt->type) {
        case STMT_IMPORT: resolve_statement_import(resolver, stmt); break;
        case STMT_BLOCK: resolve_statement_block(resolver, stmt); break;
        case STMT_IF: resolve_statement_if(resolver, stmt); break;
        case STMT_WHILE: resolve_statement_while(resolver, stmt); break;
        case STMT_FOR: resolve_statement_for(resolver, stmt); break;
        case STMT_RETURN: resolve_statement_return(resolver, stmt); break;
        case STMT_PRINT: resolve_statement_print(resolver, stmt); break;
        case STMT_EXPR: resolve_statement_expr(resolver, stmt); break;
        case STMT_BREAK: resolve_statement_break(resolver, stmt); break;
        case STMT_SKIP: resolve_statement_skip(resolver, stmt); break;
        case STMT_THROW: resolve_statement_throw(resolver, stmt); break;
        case STMT_TRY: resolve_statement_try(resolver, stmt); break;
        default: INTERNAL_ERROR("Unhandled statement type."); break;
      }
      break;
    }
    case NODE_EXPR: {
      AstExpression* expr = (AstExpression*)node;
      switch (expr->type) {
        case EXPR_BINARY: resolve_expr_binary(resolver, expr); break;
        case EXPR_POSTFIX: resolve_expr_postfix(resolver, expr); break;
        case EXPR_UNARY: resolve_expr_unary(resolver, expr); break;
        case EXPR_GROUPING: resolve_expr_grouping(resolver, expr); break;
        case EXPR_LITERAL: resolve_expr_literal(resolver, expr); break;
        case EXPR_VARIABLE: resolve_expr_variable(resolver, expr); break;
        case EXPR_ASSIGN: resolve_expr_assign(resolver, expr); break;
        case EXPR_AND: resolve_expr_and(resolver, expr); break;
        case EXPR_OR: resolve_expr_or(resolver, expr); break;
        case EXPR_IS: resolve_expr_is(resolver, expr); break;
        case EXPR_IN: resolve_expr_in(resolver, expr); break;
        case EXPR_CALL: resolve_expr_call(resolver, expr); break;
        case EXPR_DOT: resolve_expr_dot(resolver, expr); break;
        case EXPR_SUBS: resolve_expr_subs(resolver, expr); break;
        case EXPR_SLICE: resolve_expr_slice(resolver, expr); break;
        case EXPR_THIS: resolve_expr_this(resolver, expr); break;
        case EXPR_BASE: resolve_expr_base(resolver, expr); break;
        case EXPR_LAMBDA: resolve_expr_lambda(resolver, expr); break;
        case EXPR_TERNARY: resolve_expr_ternary(resolver, expr); break;
        case EXPR_TRY: resolve_expr_try(resolver, expr); break;
        default: INTERNAL_ERROR("Unhandled expression type."); break;
      }
      break;
    }
    case NODE_LIT: {
      AstLiteral* lit = (AstLiteral*)node;
      switch (lit->type) {
        case LIT_NUMBER: resolve_lit_number(resolver, lit); break;
        case LIT_STRING: resolve_lit_string(resolver, lit); break;
        case LIT_BOOL: resolve_lit_bool(resolver, lit); break;
        case LIT_NIL: resolve_lit_nil(resolver, lit); break;
        case LIT_TUPLE: resolve_lit_tuple(resolver, lit); break;
        case LIT_SEQ: resolve_lit_seq(resolver, lit); break;
        case LIT_OBJ: resolve_lit_obj(resolver, lit); break;
        default: INTERNAL_ERROR("Unhandled literal type."); break;
      }
      break;
    }
    case NODE_PATTERN: {
      AstPattern* pattern = (AstPattern*)node;
      switch (pattern->type) {
        case PAT_TUPLE: resolve_pattern_tuple(resolver, pattern); break;
        case PAT_SEQ: resolve_pattern_seq(resolver, pattern); break;
        case PAT_OBJ: resolve_pattern_obj(resolver, pattern); break;
        case PAT_REST: resolve_pattern_rest(resolver, pattern); break;
        default: INTERNAL_ERROR("Unhandled pattern type."); break;
      }
      break;
    }
  }
}

void resolve_children(Resolver* resolver, AstNode* node) {
  for (int i = 0; i < node->count; i++) {
    AstNode* child = node->children[i];
    if (child != NULL) {
      resolve_node(resolver, child);
    }
  }
}

void resolve(AstRoot* node) {
  Resolver resolver;
  resolver_init(&resolver, node->globals);
  resolve_node(&resolver, (AstNode*)node);
}