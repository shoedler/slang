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

typedef struct FnResolver FnResolver;
struct FnResolver {
  struct FnResolver* enclosing;
  AstFn* function;
  Scope* current_scope;  // Current scope, can be a child scope of the [function]s scope
  bool in_loop;
  bool in_class;

  // Track classes for base/this validation
  struct {
    bool has_baseclass;
  } current_class;

  // Track static methods for base/this validation
};

// Forward declarations
static Scope* new_scope(FnResolver* resolver);
static void end_scope(FnResolver* resolver);
void resolve_children(FnResolver* resolver, AstNode* node);
static void resolve_node(FnResolver* resolver, AstNode* node);

static void resolver_init(FnResolver* resolver, FnResolver* enclosing, AstFn* fn) {
  resolver->current_scope               = enclosing != NULL ? enclosing->current_scope : NULL;
  resolver->enclosing                   = enclosing;
  resolver->function                    = fn;
  resolver->in_loop                     = enclosing != NULL ? enclosing->in_loop : false;
  resolver->in_class                    = enclosing != NULL ? enclosing->in_class : false;
  resolver->current_class.has_baseclass = enclosing != NULL ? enclosing->current_class.has_baseclass : false;

  fn->base.scope = new_scope(resolver);  // Also sets resolver->current_scope
}

static void end_resolver(FnResolver* resolver) {
  end_scope(resolver);
}

static inline bool in_global_scope(FnResolver* resolver) {
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

// Prints an error message at the offending node.
static void resolver_error(AstNode* offending_node, const char* format, ...) {
  fprintf(stderr, "Resolver error at line %d", offending_node->token_start.line);
  fprintf(stderr, ": " ANSI_COLOR_RED);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, ANSI_COLOR_RESET "\n");
  va_end(args);

  SourceView source = chunk_make_source_view(offending_node->token_start, offending_node->token_end);
  report_error_location(source);
}

// Prints a warning message at the offending node.
static void resolver_warning(AstNode* offending_node, const char* format, ...) {
  fprintf(stderr, "Resolver warning at line %d", offending_node->token_start.line);
  fprintf(stderr, ": " ANSI_COLOR_YELLOW);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, ANSI_COLOR_RESET "\n");

  SourceView source = chunk_make_source_view(offending_node->token_start, offending_node->token_end);
  report_error_location(source);
}

static Scope* new_scope(FnResolver* resolver) {
  Scope* new_scope = malloc(sizeof(Scope));
  if (new_scope == NULL) {
    INTERNAL_ERROR("Could not allocate memory for new scope.");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }
  scope_init(new_scope, resolver->current_scope);
  resolver->current_scope = new_scope;

#ifdef DEBUG_RESOLVER
  for (int i = 0; i < resolver->current_scope->depth; printf(ANSI_GRAY_STR(":  ")), i++)
    ;
  printf("SCOPE_ENTER " ANSI_BLUE_STR("depth=%d") " :\n", resolver->current_scope->depth);
#endif

  return new_scope;
}

static void end_scope(FnResolver* resolver) {
  Scope* enclosing = resolver->current_scope->enclosing;
#ifdef DEBUG_RESOLVER
  for (int i = 0; i < resolver->current_scope->depth; printf(ANSI_GRAY_STR(":  ")), i++)
    ;
  printf("SCOPE_LEAVE " ANSI_BLUE_STR("depth=%d") " \n", resolver->current_scope->depth);

  int count = 0;
  for (int i = 0; i < resolver->current_scope->capacity; i++) {
    SymbolEntry* entry = &resolver->current_scope->entries[i];
    if (entry->key != NULL) {
      for (int j = 0; j < resolver->current_scope->depth; printf(ANSI_GRAY_STR(":  ")), j++)
        ;
      count++;
      const char* tree_link = count == resolver->current_scope->count ? "└" : "├";

      if (entry->key->length == 0) {
        printf("%s " ANSI_MAGENTA_STR("<reserved>") " ", tree_link);
      } else {
        printf("%s " ANSI_MAGENTA_STR("%s") " ", tree_link, entry->key->chars);
      }

      if (entry->value->type == SYMBOL_UPVALUE) {
        printf(ANSI_GREEN_STR("upvalue") " %d", entry->value->index);
      } else if (entry->value->type == SYMBOL_UPVALUE_OUTER) {
        printf(ANSI_GREEN_STR("upvalue_outer") " %d", entry->value->index);
      } else if (entry->value->type == SYMBOL_GLOBAL) {
        printf(ANSI_CYAN_STR("global"));
      } else if (entry->value->type == SYMBOL_LOCAL) {
        printf(ANSI_CYAN_STR("local") " %d", entry->value->index);
      }
      printf("%s\n", entry->value->is_param ? " (param)" : "");
    }
  }
#endif

  // Warn about unused variables
  for (int i = 0; i < resolver->current_scope->capacity; i++) {
    SymbolEntry* entry = &resolver->current_scope->entries[i];
    if (entry->key == NULL || entry->value->state == SYMSTATE_USED || entry->value->is_captured) {
      continue;
    }
    if (entry->value->source == NULL) {
#ifdef DEBUG_RESOLVER
      SymbolType type           = entry->value->type;
      bool ok_to_have_no_source = type == SYMBOL_NATIVE || type == SYMBOL_UPVALUE || type == SYMBOL_UPVALUE_OUTER;
      INTERNAL_ASSERT(ok_to_have_no_source, "Only natives can be unused without source");
#endif
      continue;
    }
    resolver_warning(entry->value->source, "Variable '%s' is declared but never used.", entry->key->chars);
  }

  resolver->current_scope = enclosing;
}

static Symbol* add_upvalue(FnResolver* resolver,
                           ObjString* name,
                           int index,
                           SymbolState state,
                           bool is_const,
                           bool is_param,
                           bool in_immediate_enclosing_fn) {
  // Add the upvalue to the current scope
  SymbolType upvalue_type = in_immediate_enclosing_fn ? SYMBOL_UPVALUE : SYMBOL_UPVALUE_OUTER;
  // Won't add a new upvalue if it already exists
  Symbol* upvalue = NULL;
  if (scope_add_new(resolver->current_scope, name, NULL, upvalue_type, state, is_const, is_param, &upvalue)) {
    INTERNAL_ASSERT(index >= 0, "Captured symbol should have an index.");
    // It's a new one, so we add it to the function's upvalue list.
    upvalue->index = resolver->function->upvalue_count;  // Set the index to the current upvalue count of this function
    resolver->function->upvalues[resolver->function->upvalue_count].is_local = in_immediate_enclosing_fn;
    resolver->function->upvalues[resolver->function->upvalue_count].index    = index;
    resolver->function->upvalue_count++;
    if (resolver->function->upvalue_count >= MAX_UPVALUES) {
      resolver_error((AstNode*)resolver->function,
                     "Can't have more than " STR(MAX_UPVALUES) " closure variables in one function.");
      return upvalue;
    }
  }
  return upvalue;
}

// Declare a new variable in the current scope
static void declare_variable(FnResolver* resolver, AstId* var, bool is_const) {
  if (resolver->current_scope->local_count >= MAX_LOCALS) {
    resolver_error((AstNode*)var, "Too many variables in scope.");
    return;
  }

  // Add the variable
  SymbolType type = in_global_scope(resolver) ? SYMBOL_GLOBAL : SYMBOL_LOCAL;
  Symbol* local   = NULL;
  if (!scope_add_new(resolver->current_scope, var->name, (AstNode*)var, type, SYMSTATE_DECLARED, is_const, false /* is param */,
                     &local)) {
    resolver_error((AstNode*)var, "Variable '%s' is already declared.", var->name->chars);
  }

  if (in_global_scope(resolver)) {
    return;  // No need to add globals to the function's locals
  }
}

// Declare a new parameter in the current scope
static void declare_define_parameter(FnResolver* resolver, AstId* param) {
  // Params are always locals and start their life as initialized, because currently there's no mechanism to assign them default
  // values.
  INTERNAL_ASSERT(!in_global_scope(resolver), "Parameters cannot be declared in global scope.");
  Symbol* param_sym = NULL;
  if (!scope_add_new(resolver->current_scope, param->name, (AstNode*)param, SYMBOL_LOCAL, SYMSTATE_INITIALIZED, false,
                     true /* is param */, &param_sym)) {
    resolver_error((AstNode*)param, "Parameter '%s' is already declared.", param->name->chars);
  }
}

// Define a previously declared variable in the current scope
static void define_variable(FnResolver* resolver, AstId* var) {
  // Find the variable's declaration
  Symbol* decl = scope_get(resolver->current_scope, var->name);
  if (decl == NULL) {
    INTERNAL_ERROR("Could not find local variable in current scope.");
    return;
  }
  INTERNAL_ASSERT(decl->state == SYMSTATE_DECLARED, "Variable should be declared before defining");
  decl->state = SYMSTATE_INITIALIZED;
}

// Inject a synthetic local variable into the current scope. Is injected as used, local and mutable.
// [add_to_fn] determines whether the synthetic variable should be added to the function's local list, which is only false for the
// synthetic 'this' variable, because that has to be manually inserted into slot 0 of the function's locals.
static Symbol* inject_local(FnResolver* resolver, const char* name) {
  ObjString* name_str = copy_string(name, strlen(name));

  Symbol* local = NULL;
  if (!scope_add_new(resolver->current_scope, name_str, NULL, SYMBOL_LOCAL, SYMSTATE_USED, false, false /* is param */, &local)) {
    INTERNAL_ERROR("Could not inject synthetic variable into scope.");
  }

  return local;
}

static Symbol* try_get_local(FnResolver* resolver, ObjString* name) {
  AstFn* fn    = resolver->function;
  Scope* scope = resolver->current_scope;  // Not necessarily the function's scope. Could be any nested block scope in the
                                           // functions body.

  // Stop at function scope, that's upvalue territory
  while (scope != NULL) {
    Symbol* sym = scope_get(scope, name);
    if (sym != NULL) {
      return sym;
    }
    if (scope == fn->base.scope) {
      break;
    }
    scope = scope->enclosing;  // Move up the scope chain, not the function chain!
  }

  return NULL;
}

static Symbol* try_get_upvalue(FnResolver* resolver, ObjString* name) {
  if (resolver->enclosing == NULL) {
    return NULL;
  }

  Symbol* sym = try_get_local(resolver->enclosing, name);  // Check if it's a local in the enclosing scope
  if (sym != NULL) {
    sym->is_captured = true;
    return add_upvalue(resolver, name, sym->index, sym->state, sym->is_const, sym->is_param, true);
  }

  sym = try_get_upvalue(resolver->enclosing, name);
  if (sym != NULL) {
    return add_upvalue(resolver, name, sym->index, sym->state, sym->is_const, sym->is_param, false);
  }

  return NULL;
}

// Resolves a variable lookup
static void resolve_variable(FnResolver* resolver, AstId* var) {
  Symbol* sym = NULL;

  sym = try_get_local(resolver, var->name);
  if (sym != NULL) {
    goto FOUND;
  }
  sym = try_get_upvalue(resolver, var->name);
  if (sym != NULL) {
    goto FOUND;
  }

  // Maybe it's a native?
  Value discard;
  if (hashtable_get_by_string(&vm.natives, var->name, &discard)) {
    // Natives are always global and marked here as used as well as mutable.
    // Natives are checked during assigment to prevent reassignment.
    sym = allocate_symbol(NULL, SYMBOL_NATIVE, SYMSTATE_USED, false, false /* is param */);
    return;
  }

  // Not found
  resolver_error((AstNode*)var, "Undefined variable '%s'.", var->name->chars);
  return;

FOUND:
  var->symbol = sym;
  if (sym->state == SYMSTATE_DECLARED) {
    resolver_error((AstNode*)var, "Cannot read local in its own initializer.");
  } else if (sym->state == SYMSTATE_INITIALIZED) {
    sym->state = SYMSTATE_USED;
  }
}

// Declares a pattern
static void declare_pattern(FnResolver* resolver, AstPattern* pattern, bool is_const) {
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
static void define_pattern(FnResolver* resolver, AstPattern* pattern) {
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
// static void resolve_pattern(FnResolver* resolver, AstPattern* pattern) {
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

static void resolve_assignment_target(FnResolver* resolver, AstExpression* target) {
  if (target->type == EXPR_VARIABLE) {
    AstId* id = get_child_as_id((AstNode*)target, 0, false);
    resolve_variable(resolver, id);
    if (id->symbol == NULL) {
      return;  // Undefined variable error already reported
    }

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

static void resolve_function(FnResolver* resolver, AstFn* fn) {
  // Only functions actually declare their name in the scope they're defined in.
  // - methods and constructors are declared as part of the class scope and accessed via 'this'
  // - anonymous functions are not accessible by name - that's wy they're anonymous. duh.
  if (fn->type == FN_TYPE_FUNCTION) {
    AstId* fn_name = get_child_as_id((AstNode*)fn, 0, false);
    declare_variable(resolver, fn_name, false);
    define_variable(resolver, fn_name);
  }

  FnResolver subresolver;
  resolver_init(&subresolver, resolver, fn);
  if (fn->type == FN_TYPE_METHOD || fn->type == FN_TYPE_CONSTRUCTOR) {
    inject_local(&subresolver, KEYWORD_THIS);  // Inject 'this' as the first local variable
  } else if (!in_global_scope(&subresolver)) {
    inject_local(&subresolver, "");  // Not accessible
  }

#ifdef DEBUG_RESOLVER
  switch (fn->type) {
    case FN_TYPE_ANONYMOUS_FUNCTION:
    case FN_TYPE_FUNCTION: {
      break;
    }
    case FN_TYPE_METHOD_STATIC:
    case FN_TYPE_METHOD: {
      INTERNAL_ASSERT(subresolver.in_class, "Method should be in a class.");
      INTERNAL_ASSERT(subresolver.function->type == FN_TYPE_METHOD || subresolver.function->type == FN_TYPE_METHOD_STATIC,
                      "Method should not be nested.");
      INTERNAL_ASSERT(fn->base.children[0] != NULL, "Method should have a name.");
      break;
    }
    case FN_TYPE_CONSTRUCTOR: {
      INTERNAL_ASSERT(subresolver.in_class, "Constructor should be in a class.");
      INTERNAL_ASSERT(subresolver.function->type == FN_TYPE_CONSTRUCTOR, "Constructor should not be nested.");
      break;
    }
    case FN_TYPE_MODULE: {
      INTERNAL_ASSERT(false, "Module functions are handled at the resolver entry point.");
      break;
    }
    default: INTERNAL_ERROR("Unhandled function type.");
  }
#endif

  // Parameters
  AstDeclaration* params = (AstDeclaration*)fn->base.children[1];
  if (params != NULL) {
    for (int i = 0; i < params->base.count; i++) {
      AstId* id = get_child_as_id((AstNode*)params, i, false);
      declare_define_parameter(&subresolver, id);
    }
  }

  // Body / expression
  resolve_node(&subresolver, fn->base.children[2]);
  end_resolver(&subresolver);
}

//
// Declaration resolution
//

static void resolve_declare_function(FnResolver* resolver, AstDeclaration* decl) {
  AstFn* fn = (AstFn*)decl->base.children[0];
  resolve_function(resolver, fn);
}

static void resolve_declare_class(FnResolver* resolver, AstDeclaration* decl) {
  if (resolver->in_class) {
    resolver_error((AstNode*)decl, "Classes can't be nested.");
  }
  resolver->in_class                    = true;
  resolver->current_class.has_baseclass = decl->base.children[1] != NULL;

  // Declare and define the class name
  AstId* class_name     = get_child_as_id((AstNode*)decl, 0, false);
  AstId* baseclass_name = get_child_as_id((AstNode*)decl, 1, true);
  declare_variable(resolver, class_name, false);
  define_variable(resolver, class_name);
  if (baseclass_name != NULL) {
    resolve_variable(resolver, baseclass_name);
  }

  // Create the class scope
  decl->base.scope = new_scope(resolver);
  if (resolver->current_class.has_baseclass) {
    inject_local(resolver, KEYWORD_BASE);
  }

  for (int i = 2; i < decl->base.count; i++) {
    resolve_node(resolver, decl->base.children[i]);
  }

  // End class scope
  end_scope(resolver);
  resolver->in_class                    = false;  // Class can't be nested
  resolver->current_class.has_baseclass = false;
}

static void resolve_declare_variable(FnResolver* resolver, AstDeclaration* decl) {
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

static void resolve_statement_import(FnResolver* resolver, AstStatement* stmt) {
  // We can already define the variables, since there's no expression to initialize them where they
  // could be used.
  if (stmt->base.children[0]->type == NODE_ID) {
    AstId* id = get_child_as_id((AstNode*)stmt, 0, false);
    declare_variable(resolver, id, false);
    define_variable(resolver, id);
  } else {
    AstPattern* pattern = (AstPattern*)stmt->base.children[0];
    declare_pattern(resolver, pattern, false);
    define_pattern(resolver, pattern);
  }
}

static void resolve_statement_block(FnResolver* resolver, AstStatement* stmt) {
  stmt->base.scope = new_scope(resolver);
  resolve_children(resolver, (AstNode*)stmt);
  end_scope(resolver);
}

static void resolve_statement_if(FnResolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_while(FnResolver* resolver, AstStatement* stmt) {
  bool was_in_loop  = resolver->in_loop;
  resolver->in_loop = true;
  resolve_children(resolver, (AstNode*)stmt);
  resolver->in_loop = was_in_loop;
}

static void resolve_statement_for(FnResolver* resolver, AstStatement* stmt) {
  bool was_in_loop  = resolver->in_loop;
  resolver->in_loop = true;
  stmt->base.scope  = new_scope(resolver);
  resolve_children(resolver, (AstNode*)stmt);
  end_scope(resolver);
  resolver->in_loop = was_in_loop;
}

static void resolve_statement_return(FnResolver* resolver, AstStatement* stmt) {
  if (resolver->function->type == FN_TYPE_CONSTRUCTOR) {
    resolver_error((AstNode*)stmt, "Can't return a value from a constructor.");
  }
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_print(FnResolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_expr(FnResolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_break(FnResolver* resolver, AstStatement* stmt) {
  if (!resolver->in_loop) {
    resolver_error((AstNode*)stmt, "Can't break outside of a loop.");
  }
}

static void resolve_statement_skip(FnResolver* resolver, AstStatement* stmt) {
  if (!resolver->in_loop) {
    resolver_error((AstNode*)stmt, "Can't skip outside of a loop.");
  }
}

static void resolve_statement_throw(FnResolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_try(FnResolver* resolver, AstStatement* stmt) {
  stmt->base.scope = new_scope(resolver);
  inject_local(resolver, KEYWORD_ERROR);
  resolve_children(resolver, (AstNode*)stmt);
  end_scope(resolver);
}

//
// Expression resolution
//

static void resolve_expr_binary(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_postfix(FnResolver* resolver, AstExpression* expr) {
  if (token_is_inc_dec(expr->operator_.type)) {
    resolve_assignment_target(resolver, (AstExpression*)expr->base.children[0]);
  } else {
    resolve_children(resolver, (AstNode*)expr);
  }
}

static void resolve_expr_unary(FnResolver* resolver, AstExpression* expr) {
  if (token_is_inc_dec(expr->operator_.type)) {
    resolve_assignment_target(resolver, (AstExpression*)expr->base.children[0]);
  } else {
    resolve_children(resolver, (AstNode*)expr);
  }
}

static void resolve_expr_grouping(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_literal(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_variable(FnResolver* resolver, AstExpression* expr) {
  AstId* id = get_child_as_id((AstNode*)expr, 0, false);
  resolve_variable(resolver, id);
}

static void resolve_expr_assign(FnResolver* resolver, AstExpression* expr) {
  AstExpression* left = (AstExpression*)expr->base.children[0];
  AstNode* right      = expr->base.children[1];
  resolve_assignment_target(resolver, left);
  resolve_node(resolver, (AstNode*)right);
}

static void resolve_expr_and(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_or(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_is(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_in(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_call(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_dot(FnResolver* resolver, AstExpression* expr) {
  // Only evaluate the target expression, not the property id
  AstExpression* target = (AstExpression*)expr->base.children[0];
  resolve_node(resolver, (AstNode*)target);
}

static void resolve_expr_subs(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_slice(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_this(FnResolver* resolver, AstExpression* expr) {
  if (!resolver->in_class) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_THIS "' outside of a class.");
  } else if (resolver->function->type == FN_TYPE_METHOD_STATIC) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_THIS "' in a static method.");
  }
}

static void resolve_expr_base(FnResolver* resolver, AstExpression* expr) {
  if (!resolver->in_class) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_BASE "' outside of a class.");
  } else if (!resolver->current_class.has_baseclass) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_BASE "' in a class without a base class.");
  } else if (resolver->function->type == FN_TYPE_METHOD_STATIC) {
    resolver_error((AstNode*)expr, "Can't use '" KEYWORD_BASE "' in a static method.");
  }
}

static void resolve_expr_lambda(FnResolver* resolver, AstExpression* expr) {
  resolve_function(resolver, (AstFn*)expr->base.children[0]);
}

static void resolve_expr_ternary(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_try(FnResolver* resolver, AstExpression* expr) {
  expr->base.scope = new_scope(resolver);
  inject_local(resolver, KEYWORD_ERROR);
  resolve_children(resolver, (AstNode*)expr);
  end_scope(resolver);
}

static void resolve_lit_number(FnResolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_string(FnResolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_bool(FnResolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_nil(FnResolver* resolver, AstLiteral* lit) {
  UNUSED(resolver);
  UNUSED(lit);
}

static void resolve_lit_tuple(FnResolver* resolver, AstLiteral* lit) {
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_lit_seq(FnResolver* resolver, AstLiteral* lit) {
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_lit_obj(FnResolver* resolver, AstLiteral* lit) {
  resolve_children(resolver, (AstNode*)lit);
}

static void resolve_node(FnResolver* resolver, AstNode* node) {
  switch (node->type) {
    case NODE_BLOCK: resolve_children(resolver, node); break;
    case NODE_FN: resolve_function(resolver, (AstFn*)node); break;
    case NODE_ID: INTERNAL_ERROR("Should not resolve " STR(NODE_ID) " directly."); break;
    case NODE_DECL: {
      AstDeclaration* decl = (AstDeclaration*)node;
      switch (decl->type) {
        case DECL_FN: resolve_declare_function(resolver, decl); break;
        case DECL_FN_PARAMS: INTERNAL_ERROR("Should not resolve " STR(DECL_FN_PARAMS) " directly."); break;
        case DECL_CLASS: resolve_declare_class(resolver, decl); break;
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

void resolve_children(FnResolver* resolver, AstNode* node) {
  for (int i = 0; i < node->count; i++) {
    AstNode* child = node->children[i];
    if (child != NULL) {
      resolve_node(resolver, child);
    }
  }
}

void resolve(AstFn* root) {
  FnResolver resolver;
#ifdef DEBUG_RESOLVER
  printf("\n\n\n === RESOLVE ===\n\n");
#endif
  resolver_init(&resolver, NULL, root);
  inject_local(&resolver, "");  // Not accessible
  // Skip the first and second children, which are the name and parameters
  INTERNAL_ASSERT(root->base.count == 3, "Function should have exactly 3 children.");
  resolve_node(&resolver, root->base.children[2]);
  end_resolver(&resolver);
  return;
}
