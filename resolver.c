#include "resolver.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "scope.h"
#include "vm.h"

typedef struct {
  Scope* current_scope;
  bool in_function;
  bool in_method;
  bool in_constructor;
  bool in_loop;
  bool in_class;

  // Track classes for base/this validation
  struct {
    bool has_baseclass;
  } current_class;

  // Track static methods for base/this validation
  struct {
    bool is_static;
  } current_method;
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
  resolver->in_class                    = false;
  resolver->current_class.has_baseclass = false;
}

static inline bool in_global_scope(Resolver* resolver) {
  return resolver->current_scope->depth == 0;
}

#ifdef DEBUG_RESOLVER
static inline AstId* get_child_as_id(AstNode* parent, int index, bool allow_null) {
  AstNode* child = parent->children[index];
  if (child == NULL) {
    if (!allow_null) {
      INTERNAL_ERROR("Expected " STR(AstId) "child node to be non-null.");
    }
    return NULL;
  }
  if (child->type != NODE_ID) {
    INTERNAL_ERROR("Expected child node to be of type " STR(NODE_ID) ", got %d.", child->type);
  }
  AstId* id = (AstId*)child;
  INTERNAL_ASSERT(id->symbol == NULL, "Symbol should be NULL before resolving.");
  return id;
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

static Scope* new_scope(Resolver* resolver, const char* debug_name) {
  Scope* new_scope = malloc(sizeof(Scope));
  if (new_scope == NULL) {
    INTERNAL_ERROR("Could not allocate memory for new scope.");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }
  scope_init(new_scope, resolver->current_scope);
  resolver->current_scope = new_scope;

#ifdef DEBUG_RESOLVER
  for (int i = 0; i < resolver->current_scope->depth; printf("  "), i++)
    ;
  printf(ANSI_YELLOW_STR("Entering scope <") "%s" ANSI_YELLOW_STR(">") " depth=%d\n", debug_name, resolver->current_scope->depth);
#endif

  return new_scope;
}

static void end_scope(Resolver* resolver) {
  Scope* enclosing = resolver->current_scope->enclosing;
#ifdef DEBUG_RESOLVER
  for (int i = 0; i < resolver->current_scope->depth; printf("  "), i++)
    ;
  printf(ANSI_YELLOW_STR("Leaving scope") " depth=%d\n", resolver->current_scope->depth);
  for (int i = 0; i < resolver->current_scope->capacity; i++) {
    SymbolEntry* entry = &resolver->current_scope->entries[i];
    if (entry->key != NULL) {
      for (int j = 0; j < resolver->current_scope->depth; printf("  "), j++)
        ;
      printf(ANSI_YELLOW_STR("-") " %s\n", entry->key->chars);
    }
  }
#endif
  resolver->current_scope = enclosing;
}

// Declare a new variable in the current scope
static void declare_variable(Resolver* resolver, AstId* var, bool is_const) {
  if (resolver->current_scope->count >= MAX_LOCALS) {
    resolver_error((AstNode*)var, "Too many variables in scope.");
    return;
  }

  // Add the variable
  SymbolType type = in_global_scope(resolver) ? SYMBOL_GLOBAL : SYMBOL_LOCAL;
  if (!scope_add_new(resolver->current_scope, var->name, type, is_const, false)) {
    resolver_error((AstNode*)var, "Variable '%s' is already defined.", var->name->chars);
  }
}

// Declare a new parameter in the current scope
static void declare_parameter(Resolver* resolver, AstId* param) {
  if (!scope_add_new(resolver->current_scope, param->name, SYMBOL_PARAM, false, true)) {
    resolver_error((AstNode*)param, "Parameter '%s' is already defined.", param->name->chars);
  }
}

// Define a previously declared variable in the current scope
static void define_variable(Resolver* resolver, AstId* var) {
  // Find the variable's declaration
  Symbol* declaration = scope_get(resolver->current_scope, var->name);
  if (declaration == NULL) {
    INTERNAL_ERROR("Could not find local variable in current scope.");
    return;
  }
  declaration->is_initialized = true;
}

// Inject a synthetic variable into the current scope
static void inject_synthetic_variable(Resolver* resolver, const char* name) {
  ObjString* name_str = copy_string(name, strlen(name));
  SymbolType type     = in_global_scope(resolver) ? SYMBOL_GLOBAL : SYMBOL_LOCAL;
  if (!scope_add_new(resolver->current_scope, name_str, type, false, true)) {
    INTERNAL_ERROR("Could not inject synthetic variable into scope.");
  }
}

// Resolves a variable lookup
static void resolve_variable(Resolver* resolver, AstId* var) {
  // Look up variable in scope chain
  for (Scope* scope = resolver->current_scope; scope != NULL; scope = scope->enclosing) {
    Symbol* sym = scope_get(scope, var->name);
    if (sym != NULL) {
      var->symbol = sym;
      return;
    }
  }

  // Maybe it's a native?
  Value discard;
  if (hashtable_get_by_string(&vm.natives, var->name, &discard)) {
    var->symbol = allocate_symbol(SYMBOL_NATIVE, false, true);
    return;
  }

  // Not found
  resolver_error((AstNode*)var, "Undefined variable '%s'.", var->name->chars);
}

// Declares a pattern
static void declare_pattern(Resolver* resolver, AstPattern* pattern, bool is_const) {
  switch (pattern->type) {
    case PAT_REST:
    case PAT_BINDING: {
      AstId* id = get_child_as_id((AstNode*)pattern, 0, false);
      declare_variable(resolver, id, is_const);
      break;
    }
    case PAT_SEQ:
    case PAT_OBJ:
    case PAT_TUPLE: {
      for (int i = 0; i < pattern->base.count; i++) {
        declare_pattern(resolver, (AstPattern*)pattern->base.children[i], is_const);
      }
      break;
    }
    default: INTERNAL_ERROR("Unhandled pattern type.");
  }
}

// Defines a pattern
static void define_pattern(Resolver* resolver, AstPattern* pattern) {
  switch (pattern->type) {
    case PAT_REST:
    case PAT_BINDING: {
      AstId* id = get_child_as_id((AstNode*)pattern, 0, false);
      define_variable(resolver, id);
      break;
    }
    case PAT_SEQ:
    case PAT_OBJ:
    case PAT_TUPLE: {
      for (int i = 0; i < pattern->base.count; i++) {
        define_pattern(resolver, (AstPattern*)pattern->base.children[i]);
      }
      break;
    }
    default: INTERNAL_ERROR("Unhandled pattern type.");
  }
}

// // Resolves a pattern
// static void resolve_pattern(Resolver* resolver, AstPattern* pattern) {
//   switch (pattern->type) {
//     case PAT_REST:
//     case PAT_BINDING: {
//       AstId* id = get_child_as_id((AstNode*)pattern, 0, false);
//       resolve_variable(resolver, id);
//       break;
//     }
//     case PAT_SEQ:
//     case PAT_OBJ:
//     case PAT_TUPLE: {
//       for (int i = 0; i < pattern->base.count; i++) {
//         resolve_pattern(resolver, pattern->base.children[i]);
//       }
//       break;
//     }
//     default: INTERNAL_ERROR("Unhandled pattern type.");
//   }
// }

static void resolve_assignment_target(Resolver* resolver, AstExpression* target) {
  if (target->type == EXPR_VARIABLE) {
    AstId* id = get_child_as_id((AstNode*)target, 0, false);
    resolve_variable(resolver, id);
    INTERNAL_ASSERT(id->symbol != NULL, "Variable symbol should be resolved.");

    if (id->symbol->type == SYMBOL_NATIVE) {
      resolver_error((AstNode*)target, "Don't reassign natives.", id->name->chars);
      return;
    }

    if (id->symbol->is_const) {
      resolver_error((AstNode*)target, "Cannot assign to constant variable '%s'.", id->name->chars);
    }
  } else {
    // Could be something like a subscript or dot, which we currently can't check for mutability
    resolve_node(resolver, (AstNode*)target);
  }
}

static void resolve_root(Resolver* resolver, AstRoot* root) {
  root->globals = new_scope(resolver, "global");
  resolve_children(resolver, (AstNode*)root);
  end_scope(resolver);
}

//
// Declaration resolution
//

static void resolve_declare_function(Resolver* resolver, AstDeclaration* decl) {
  // Configure resolver state for function type
  bool was_in_function;
  const char* debug_name;
  switch (decl->fn_type) {
    case FN_TYPE_ANONYMOUS_FUNCTION:
    case FN_TYPE_FUNCTION: {
      was_in_function       = resolver->in_function;
      resolver->in_function = true;
      debug_name            = "function";
      break;
    }
    case FN_TYPE_METHOD_STATIC:
    case FN_TYPE_METHOD: {
      if (!resolver->in_class) {
        resolver_error((AstNode*)decl, "Can't declare a method outside of a class.");
      }
      if (resolver->in_method) {
        resolver_error((AstNode*)decl, "Can't have nested methods.");
      }
      resolver->in_method = true;
      debug_name          = "method";
      break;
    }
    case FN_TYPE_CONSTRUCTOR: {
      if (!resolver->in_class) {
        resolver_error((AstNode*)decl, "Can't declare a constructor outside of a class.");
      }
      if (resolver->in_constructor) {
        resolver_error((AstNode*)decl, "Can't have nested constructors.");
      }
      resolver->in_constructor = true;
      debug_name               = "constructor";
      break;
    }
    default: INTERNAL_ERROR("Unhandled function type.");
  }

  // Resolve. Start by creating a new scope for the function
  decl->base.scope = new_scope(resolver, debug_name);

  // Only functions actually declare the function name in the scope
  // - methods and constructors are declared as part of the class scope and accessed via 'this'
  // - anonymous functions are not accessible by name - that's wy they're anonymous. duh.
  if (decl->fn_type == FN_TYPE_FUNCTION) {
    AstId* fn_name = get_child_as_id((AstNode*)decl, 0, false);
    declare_variable(resolver, fn_name, false);
    define_variable(resolver, fn_name);
  }

  // Parameters
  AstDeclaration* params = (AstDeclaration*)decl->base.children[1];
  if (params != NULL) {
    for (int i = 0; i < params->base.count; i++) {
      AstId* id = get_child_as_id((AstNode*)params, i, false);
      declare_parameter(resolver, id);
      define_variable(resolver, id);
    }
  }

  // Body / expression
  resolve_node(resolver, decl->base.children[2]);
  end_scope(resolver);

  // Done! Now reset the resolver state
  switch (decl->fn_type) {
    case FN_TYPE_ANONYMOUS_FUNCTION:
    case FN_TYPE_FUNCTION: {
      resolver->in_function = was_in_function;
      break;
    }
    case FN_TYPE_METHOD_STATIC:
    case FN_TYPE_METHOD: {
      resolver->in_method = false;
      break;
    }
    case FN_TYPE_CONSTRUCTOR: {
      resolver->in_constructor = false;
      break;
    }
    default: INTERNAL_ERROR("Unhandled function type.");
  }
}

static void resolve_declare_class(Resolver* resolver, AstDeclaration* decl) {
  if (resolver->in_class) {
    resolver_error((AstNode*)decl, "Classes can't be nested.");
  }
  resolver->in_class                    = true;
  resolver->current_class.has_baseclass = decl->base.children[1] != NULL;

  // Inject 'this' and 'base' variables
  decl->base.scope = new_scope(resolver, "class");
  inject_synthetic_variable(resolver, KEYWORD_THIS);
  if (resolver->current_class.has_baseclass) {
    inject_synthetic_variable(resolver, KEYWORD_BASE);
  }

  // Resolve
  AstId* class_name     = get_child_as_id((AstNode*)decl, 0, false);
  AstId* baseclass_name = get_child_as_id((AstNode*)decl, 1, true);
  declare_variable(resolver, class_name, false);
  define_variable(resolver, class_name);
  if (baseclass_name != NULL) {
    declare_variable(resolver, baseclass_name, false);
    define_variable(resolver, baseclass_name);
  }
  for (int i = 2; i < decl->base.count; i++) {
    resolve_node(resolver, decl->base.children[i]);
  }

  // End class scope
  end_scope(resolver);
  resolver->in_class                    = false;  // Class can't be nested
  resolver->current_class.has_baseclass = false;
}

static void resolve_declare_method(Resolver* resolver, AstDeclaration* decl) {
  resolve_declare_function(resolver, (AstDeclaration*)decl->base.children[0]);
}

static void resolve_declare_constructor(Resolver* resolver, AstDeclaration* decl) {
  resolve_declare_function(resolver, (AstDeclaration*)decl->base.children[0]);
}

static void resolve_declare_variable(Resolver* resolver, AstDeclaration* decl) {
  NodeType type = decl->base.children[0]->type;

  // Declare the variable in the current scope
  if (type == NODE_PATTERN) {
    AstPattern* pattern = (AstPattern*)decl->base.children[0];
    declare_pattern(resolver, pattern, decl->is_const);
  } else {
    AstId* id = get_child_as_id((AstNode*)decl, 0, false);
    declare_variable(resolver, id, decl->is_const);
  }

  // Resolve initializer if present
  AstNode* initializer = decl->base.children[1];
  if (initializer != NULL) {
    resolve_node(resolver, initializer);
  }

  // Define the variable
  if (type == NODE_PATTERN) {
    AstPattern* pattern = (AstPattern*)decl->base.children[0];
    define_pattern(resolver, pattern);
  } else {
    AstId* id = get_child_as_id((AstNode*)decl, 0, false);
    define_variable(resolver, id);
  }
}

//
// Statement resolution
//

static void resolve_statement_import(Resolver* resolver, AstStatement* stmt) {
  if (stmt->base.children[0]->type == NODE_ID) {
    AstId* id = get_child_as_id((AstNode*)stmt, 0, false);
    declare_variable(resolver, id, false);
    define_variable(resolver, id);
  } else {
    INTERNAL_ERROR("Only simple variable imports are supported for now.");
  }
}

static void resolve_statement_block(Resolver* resolver, AstStatement* stmt) {
  stmt->base.scope = new_scope(resolver, "block");
  resolve_children(resolver, (AstNode*)stmt);
  end_scope(resolver);
}

static void resolve_statement_if(Resolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_while(Resolver* resolver, AstStatement* stmt) {
  bool was_in_loop  = resolver->in_loop;
  resolver->in_loop = true;
  resolve_children(resolver, (AstNode*)stmt);
  resolver->in_loop = was_in_loop;
}

static void resolve_statement_for(Resolver* resolver, AstStatement* stmt) {
  bool was_in_loop  = resolver->in_loop;
  resolver->in_loop = true;
  stmt->base.scope  = new_scope(resolver, "for");
  resolve_children(resolver, (AstNode*)stmt);
  end_scope(resolver);
  resolver->in_loop = was_in_loop;
}

static void resolve_statement_return(Resolver* resolver, AstStatement* stmt) {
  if (resolver->in_constructor) {
    resolver_error((AstNode*)stmt, "Can't return a value from a constructor.");
  }
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_print(Resolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_expr(Resolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_break(Resolver* resolver, AstStatement* stmt) {
  if (!resolver->in_loop) {
    resolver_error((AstNode*)stmt, "Can't break outside of a loop.");
  }
}

static void resolve_statement_skip(Resolver* resolver, AstStatement* stmt) {
  if (!resolver->in_loop) {
    resolver_error((AstNode*)stmt, "Can't skip outside of a loop.");
  }
}

static void resolve_statement_throw(Resolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_try(Resolver* resolver, AstStatement* stmt) {
  stmt->base.scope = new_scope(resolver, "try");
  inject_synthetic_variable(resolver, KEYWORD_ERROR);
  resolve_children(resolver, (AstNode*)stmt);
  end_scope(resolver);
}

//
// Expression resolution
//

static void resolve_expr_binary(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_postfix(Resolver* resolver, AstExpression* expr) {
  if (token_is_inc_dec(expr->operator_.type)) {
    resolve_assignment_target(resolver, (AstExpression*)expr->base.children[0]);
  } else {
    resolve_children(resolver, (AstNode*)expr);
  }
}

static void resolve_expr_unary(Resolver* resolver, AstExpression* expr) {
  if (token_is_inc_dec(expr->operator_.type)) {
    resolve_assignment_target(resolver, (AstExpression*)expr->base.children[0]);
  } else {
    resolve_children(resolver, (AstNode*)expr);
  }
}

static void resolve_expr_grouping(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_literal(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_variable(Resolver* resolver, AstExpression* expr) {
  AstId* id = get_child_as_id((AstNode*)expr, 0, false);
  resolve_variable(resolver, id);
}

static void resolve_expr_assign(Resolver* resolver, AstExpression* expr) {
  AstExpression* left = (AstExpression*)expr->base.children[0];
  AstNode* right      = expr->base.children[1];
  resolve_assignment_target(resolver, left);
  resolve_node(resolver, (AstNode*)right);
}

static void resolve_expr_and(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_or(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_is(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_in(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_call(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_dot(Resolver* resolver, AstExpression* expr) {
  // Only evaluate the target expression, not the property id
  AstExpression* target = (AstExpression*)expr->base.children[0];
  resolve_node(resolver, (AstNode*)target);
}

static void resolve_expr_subs(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_slice(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_this(Resolver* resolver, AstExpression* expr) {
  if (!resolver->in_class) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_THIS "' outside of a class.");
  } else if (resolver->in_method && resolver->current_method.is_static) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_THIS "' in a static method.");
  }
}

static void resolve_expr_base(Resolver* resolver, AstExpression* expr) {
  if (!resolver->in_class) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_BASE "' outside of a class.");
  } else if (!resolver->current_class.has_baseclass) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_BASE "' in a class without a base class.");
  } else if (resolver->in_method && resolver->current_method.is_static) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_BASE "' in a static method.");
  }
}

static void resolve_expr_lambda(Resolver* resolver, AstExpression* expr) {
  resolve_declare_function(resolver, (AstDeclaration*)expr->base.children[0]);
}

static void resolve_expr_ternary(Resolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_try(Resolver* resolver, AstExpression* expr) {
  expr->base.scope = new_scope(resolver, "try");
  inject_synthetic_variable(resolver, KEYWORD_ERROR);
  resolve_children(resolver, (AstNode*)expr);
  end_scope(resolver);
}

static void resolve_lit_number(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_string(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_bool(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_nil(Resolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_tuple(Resolver* resolver, AstLiteral* lit) {
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_lit_seq(Resolver* resolver, AstLiteral* lit) {
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_lit_obj(Resolver* resolver, AstLiteral* lit) {
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_node(Resolver* resolver, AstNode* node) {
  switch (node->type) {
    case NODE_ROOT: resolve_root(resolver, (AstRoot*)node); break;
    case NODE_ID: INTERNAL_ERROR("Should not resolve " STR(NODE_ID) " directly."); break;
    case NODE_DECL: {
      AstDeclaration* decl = (AstDeclaration*)node;
      switch (decl->type) {
        case DECL_FN: resolve_declare_function(resolver, decl); break;
        case DECL_FN_PARAMS: INTERNAL_ERROR("Should not resolve " STR(DECL_FN_PARAMS) " directly."); break;
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
    case NODE_PATTERN: INTERNAL_ERROR("Should not resolve " STR(NODE_PATTERN) " directly."); break;
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