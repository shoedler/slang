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

AstFn* resolver_root = NULL;

// Forward declarations
static Scope* new_scope(FnResolver* resolver);
static void end_scope(FnResolver* resolver);
void resolve_children(FnResolver* resolver, AstNode* node);
static void resolve_node(FnResolver* resolver, AstNode* node);

static void resolver_init(FnResolver* resolver, FnResolver* enclosing, AstFn* fn) {
  resolver->enclosing = enclosing;
  resolver->function  = fn;

  resolver->disable_warnings       = enclosing != NULL ? enclosing->disable_warnings : false;  // Default to false
  resolver->root_scope             = enclosing != NULL ? enclosing->root_scope : NULL;
  resolver->global_scope           = enclosing != NULL ? enclosing->global_scope : NULL;
  resolver->native_scope           = enclosing != NULL ? enclosing->native_scope : NULL;
  resolver->current_scope          = enclosing != NULL ? enclosing->current_scope : NULL;  // Set in new_scope
  resolver->current_loop           = enclosing != NULL ? enclosing->current_loop : NULL;
  resolver->current_loop_fn_locals = enclosing != NULL ? enclosing->current_loop_fn_locals : 0;
  resolver->in_class               = enclosing != NULL ? enclosing->in_class : false;
  resolver->has_baseclass          = enclosing != NULL ? enclosing->has_baseclass : false;
  resolver->had_error              = enclosing != NULL ? enclosing->had_error : false;
  resolver->panic_mode             = enclosing != NULL ? enclosing->panic_mode : false;

  fn->base.scope = new_scope(resolver);  // Also sets resolver->current_scope
}

static void end_resolver(FnResolver* resolver) {
  if (resolver->enclosing != NULL) {
    resolver->enclosing->had_error |= resolver->had_error;
  }
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
  INTERNAL_ASSERT(id->ref == NULL, "Symbol should be NULL before resolving.");
  return id;
}
#else
#define get_child_as_id(parent, index, allow_null) ((AstId*)parent->base.children[index])
#endif

// Prints an error message at the offending node.
static void resolver_error(FnResolver* resolver, AstNode* offending_node, const char* format, ...) {
  if (resolver->panic_mode) {
    return;
  }
  resolver->panic_mode = true;
  resolver->had_error  = true;

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
static void resolver_warning(FnResolver* resolver, AstNode* offending_node, const char* format, ...) {
  if (resolver->disable_warnings) {
    return;
  }

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
  return new_scope;
}

static void end_scope(FnResolver* resolver) {
  Scope* enclosing = resolver->current_scope->enclosing;

  // Reduce the function's local count by the number of locals in this scope
  resolver->function->local_count -= resolver->current_scope->local_count;
  INTERNAL_ASSERT(resolver->function->local_count >= 0, "Function local count should never be negative.");

  // Check for unused variables
  for (int i = 0; i < resolver->current_scope->capacity; i++) {
    SymbolEntry* entry = &resolver->current_scope->entries[i];
    if (entry->key == NULL || entry->value->state == SYMSTATE_USED || entry->value->is_captured) {
      continue;
    }

    if (entry->value->source == NULL) {
#ifdef DEBUG_RESOLVER
      INTERNAL_ASSERT(entry->value->type == SYMBOL_NATIVE, "Only natives can be unused without source");
#endif
      continue;
    }
    resolver_warning(resolver, entry->value->source, "Variable '%s' is declared but never used.", entry->key->chars);
  }

  resolver->current_scope = enclosing;
}

static SymbolRef* make_ref(Symbol* symbol) {
  SymbolRef* ref = malloc(sizeof(SymbolRef));
  if (ref == NULL) {
    INTERNAL_ERROR("Could not allocate memory for symbol reference.");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  ref->symbol     = symbol;
  ref->index      = symbol->function_index;
  ref->is_upvalue = false;
  return ref;
}

static SymbolRef* make_ref_upvalue(Upvalue* upvalue) {
  SymbolRef* ref = malloc(sizeof(SymbolRef));
  if (ref == NULL) {
    INTERNAL_ERROR("Could not allocate memory for symbol reference.");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  ref->symbol     = upvalue->symbol;
  ref->index      = upvalue->function_index;
  ref->is_upvalue = true;
  return ref;
}

// Adds or retrieves an upvalue from the functions upvalue list. Won't add duplicates. An upvalue is identical if it points to the
// same symbol (the local that got captured) and {is_local} is the same. [is_local] is relevant, because it determines how the
// upvalues [target_index] is interpreted: if it's local, it's the index in the functions locals array, if it's not, it's the
// index in the functions upvalues array.
static Upvalue* add_upvalue(FnResolver* resolver, Symbol* captured_symbol, int target_index, bool is_local) {
  int upvalue_count = resolver->function->upvalue_count;

  // Check if the upvalue already exists
  for (int i = 0; i < upvalue_count; i++) {
    Upvalue* upvalue = &resolver->function->upvalues[i];
    if (upvalue->symbol == captured_symbol && upvalue->is_local == is_local) {
      return upvalue;
    }
  }

  // Nope, then add it
  if (upvalue_count == MAX_UPVALUES) {
    resolver_error(resolver, (AstNode*)resolver->function,
                   "Can't have more than " STR(MAX_UPVALUES) " closure variables in one function.");
    return NULL;
  }

  resolver->function->upvalues[upvalue_count].is_local     = is_local;
  resolver->function->upvalues[upvalue_count].symbol       = captured_symbol;
  resolver->function->upvalues[upvalue_count].target_index = target_index;  // Where this upvalue points to, depending on is_local
  resolver->function->upvalues[upvalue_count].function_index = upvalue_count;
  return &resolver->function->upvalues[resolver->function->upvalue_count++];
}

// Adds a new local variable to the current scope. If the variable is already declared in this scope, an error is reported.
static Symbol* add_local(FnResolver* resolver, ObjString* name, AstId* var, SymbolState state, bool is_const, bool is_param) {
  Symbol* local  = NULL;
  ObjString* key = name == NULL ? var->name : name;
  if (!scope_add_new(resolver->current_scope, key, (AstNode*)var, SYMBOL_LOCAL, state, is_const, is_param, &local)) {
    if (is_param) {
      resolver_error(resolver, (AstNode*)var, "Parameter '%s' is already declared.", var->name->chars);
      return NULL;
    } else {
      resolver_error(resolver, (AstNode*)var, "Local variable '%s' is already declared.", var->name->chars);
      return NULL;
    }
  }

  // Set the function index of the local, this is the "running total" of locals in the function, including nested scopes, though
  // nested scopes will decrement this count when they end, see end_scope.
  local->function_index = resolver->function->local_count++;
  if (resolver->function->local_count >= MAX_LOCALS) {
    resolver_error(resolver, (AstNode*)var, "Can't have more than " STR(MAX_LOCALS) " local variables total in one function.");
  }
  return local;
}

// Adds a new global variable to the global scope. If the variable is already declared in the global scope, an error is reported.
static Symbol* add_global(FnResolver* resolver, AstId* var, SymbolState state, bool is_const) {
  Symbol* global = NULL;
  if (!scope_add_new(resolver->root_scope, var->name, (AstNode*)var, SYMBOL_GLOBAL, state, is_const, false /* is param */,
                     &global)) {
    // Could be the case that there's already a native with the same name, so we need to shadow it with a new global
    if (global->type == SYMBOL_NATIVE) {
      global->source   = (AstNode*)var;
      global->type     = SYMBOL_GLOBAL;
      global->state    = state;
      global->is_const = is_const;
      global->is_param = false;
    } else if (global->type == SYMBOL_UNDEFINED) {
      // ..or, a late-bound global, which is fine, we just shadow it
      global->source   = (AstNode*)var;
      global->type     = SYMBOL_GLOBAL;
      global->state    = state;
      global->is_const = is_const;
      global->is_param = false;
    } else {
      resolver_error(resolver, (AstNode*)var, "Global variable '%s' is already declared.", var->name->chars);
      return NULL;
    }
  }
  return global;
}

// Declare a new variable in the current scope.
static void declare_variable(FnResolver* resolver, AstId* var, bool is_const) {
  if (in_global_scope(resolver)) {
    add_global(resolver, var, SYMSTATE_DECLARED, is_const);
  } else {
    add_local(resolver, NULL, var, SYMSTATE_DECLARED, is_const, false /* is param */);
  }
}

// Define a previously declared variable in the current scope.
static void define_variable(FnResolver* resolver, AstId* var) {
  // Find the variable's declaration
  Symbol* decl = scope_get(resolver->current_scope, var->name);
  INTERNAL_ASSERT(decl != NULL, "Could not find local variable in current scope. Forgot to declare it?");
  INTERNAL_ASSERT(decl->state == SYMSTATE_DECLARED, "Variable must be declared before defining. Forgot to declare it?");
  decl->state = SYMSTATE_INITIALIZED;

  var->ref = make_ref(decl);
}

// Declare and defines a new parameter in the current scope.
static void add_parameter(FnResolver* resolver, AstId* param) {
  // Params are locals and start their life as initialized, because currently there's no mechanism to assign them default values.
  Symbol* sym = add_local(resolver, NULL, param, SYMSTATE_INITIALIZED, false, true /* is param */);
  if (sym == NULL) {
    return;  // Error already reported
  }

  param->ref = make_ref(sym);
}

// Inject a synthetic local variable into the current scope. It is injected as used, so we don't get unused variable warnings.
static Symbol* inject_local(FnResolver* resolver, const char* name, bool is_const) {
  ObjString* name_str = copy_string(name, strlen(name));
  Symbol* var         = add_local(resolver, name_str, NULL, SYMSTATE_USED, is_const, false /* is param */);
  INTERNAL_ASSERT(var != NULL, "Could not inject synthetic local variable. Name: %s", name);

  return var;
}

// Try to find a variable in the current functions scope chain.
static Symbol* try_get_local(FnResolver* resolver, ObjString* name) {
  AstFn* fn    = resolver->function;
  Scope* scope = resolver->current_scope;  // Not necessarily the function's scope. Could be any nested block scope in the
                                           // functions body.

  // Stop at function scope, that's upvalue territory
  while (scope != NULL) {
    Symbol* sym = scope_get(scope, name);
    if (sym != NULL && sym->type == SYMBOL_LOCAL) {
      return sym;
    }
    if (scope == fn->base.scope) {
      break;
    }
    scope = scope->enclosing;  // Move up the scope chain, not the function chain!
  }

  return NULL;
}

// Try to find an upvalue by looking locals/upvalues it in enclosing functions. This will also add the intermediate upvalues to
// pipe the upvalue through - from the variable we try to resolve up to the function in which it's captured.
static Upvalue* try_get_upvalue(FnResolver* resolver, ObjString* name) {
  if (resolver->enclosing == NULL) {
    return NULL;
  }

  Symbol* local = try_get_local(resolver->enclosing, name);  // Check if it's a local in the enclosing scope
  if (local != NULL) {
    local->is_captured = true;
    return add_upvalue(resolver, local, local->function_index, true);
  }

  Upvalue* upvalue = try_get_upvalue(resolver->enclosing, name);
  if (upvalue != NULL) {
    return add_upvalue(resolver, upvalue->symbol, upvalue->function_index, false);
  }

  return NULL;
}

// Resolves a variable lookup
static void resolve_variable(FnResolver* resolver, AstId* var) {
  SymbolRef* ref = NULL;

  Symbol* local = try_get_local(resolver, var->name);
  if (local != NULL) {
    ref = make_ref(local);
    goto FOUND;
  }

  Upvalue* upvalue = try_get_upvalue(resolver, var->name);
  if (upvalue != NULL) {
    ref = make_ref_upvalue(upvalue);
    goto FOUND;
  }

  // Globals
  // ...can be in the global scope of the ast we're resolving
  Symbol* global = scope_get(resolver->root_scope, var->name);
  if (global != NULL) {
    ref = make_ref(global);
    goto FOUND;
  }

  // ...or in the globals table
  Value discard;
  if (hashtable_get_by_string(resolver->global_scope, var->name, &discard)) {
    if (!scope_add_new(resolver->root_scope, var->name, (AstNode*)var, SYMBOL_GLOBAL, SYMSTATE_USED, false, false /* is param */,
                       &global)) {
      INTERNAL_ERROR("External global should not be redeclared.");
    }
    ref = make_ref(global);
    goto FOUND;
  }

  // ...or in the natives table
  if (hashtable_get_by_string(resolver->native_scope, var->name, &discard)) {
    // Natives are always global and marked here as used as well as mutable, they are checked during assigment to prevent
    // reassignment. This way we can make better error messages.
    // Native references are added to the global scope for now, just so it looks sensible when printing the scopes.
    if (!scope_add_new(resolver->root_scope, var->name, (AstNode*)var, SYMBOL_NATIVE, SYMSTATE_USED, false, false /* is param */,
                       &global)) {
      INTERNAL_ERROR("Native should not be redeclared.");
    }
    ref = make_ref(global);
    goto FOUND;
  }

  // Not found - could be a late-bound global which we'll resolve later
  scope_add_new(resolver->root_scope, var->name, (AstNode*)var, SYMBOL_UNDEFINED, SYMSTATE_USED, false, false /* is param */,
                &global);
  ref = make_ref(global);

FOUND:
  var->ref = ref;  // Set the reference on the variable
  if (ref->symbol->state == SYMSTATE_DECLARED) {
    resolver_error(resolver, (AstNode*)var, "Cannot read variable in its own initializer.");
  } else if (ref->symbol->state == SYMSTATE_INITIALIZED) {
    ref->symbol->state = SYMSTATE_USED;
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
    if (id->ref == NULL) {
      return;  // Undefined variable error already reported
    }

    if (id->ref->symbol->type == SYMBOL_NATIVE) {
      resolver_error(resolver, (AstNode*)target, "Don't reassign natives.", id->name->chars);
      return;
    }

    if (id->ref->symbol->is_const) {
      resolver_error(resolver, (AstNode*)target, "Cannot assign to constant variable '%s'.", id->name->chars);
    }
  } else {
    // Could be something like a subscript or dot, which we currently can't check for mutability
    resolve_node(resolver, (AstNode*)target);
  }
}

static void resolve_function(FnResolver* resolver, AstFn* fn) {
  FnResolver subresolver;
  resolver_init(&subresolver, resolver, fn);
  if (fn->type == FN_TYPE_METHOD || fn->type == FN_TYPE_CONSTRUCTOR) {
    inject_local(&subresolver, KEYWORD_THIS, true);  // Inject 'this' as the first local variable
  } else if (!in_global_scope(&subresolver)) {
    inject_local(&subresolver, "", true);  // Not accessible
  }

  // Only named functions actually declare their name in the scope they're defined in.
  // - methods and constructors are declared as part of the class scope and accessed via 'this'
  // - anonymous functions are not accessible by name - that's wy they're anonymous. duh.
  if (fn->type == FN_TYPE_NAMED_FUNCTION) {
    AstId* fn_name = get_child_as_id((AstNode*)fn, 0, false);
    declare_variable(resolver, fn_name, false);
    define_variable(resolver, fn_name);
  }

#ifdef DEBUG_RESOLVER
  switch (fn->type) {
    case FN_TYPE_ANONYMOUS_FUNCTION:
    case FN_TYPE_NAMED_FUNCTION: {
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
      add_parameter(&subresolver, id);
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
  resolver->in_class      = true;
  resolver->has_baseclass = decl->base.children[1] != NULL;

  // Declare and define the class name
  AstId* class_name     = get_child_as_id((AstNode*)decl, 0, false);
  AstId* baseclass_name = get_child_as_id((AstNode*)decl, 1, true);
  declare_variable(resolver, class_name, false);
  define_variable(resolver, class_name);
  if (baseclass_name != NULL) {
    resolve_variable(resolver, baseclass_name);
  }

  if (resolver->has_baseclass) {
    decl->base.scope = new_scope(resolver);
    inject_local(resolver, KEYWORD_BASE, true);
  }

  for (int i = 2; i < decl->base.count; i++) {
    resolve_node(resolver, decl->base.children[i]);
  }

  if (resolver->has_baseclass) {
    end_scope(resolver);
  }
  resolver->in_class      = false;  // Class can't be nested
  resolver->has_baseclass = false;
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
  AstStatement* enclosing_loop     = resolver->current_loop;
  resolver->current_loop           = stmt;
  int enclosing_loop_fn_locals     = resolver->current_loop_fn_locals;
  resolver->current_loop_fn_locals = resolver->function->local_count;

  resolve_children(resolver, (AstNode*)stmt);

  resolver->current_loop           = enclosing_loop;
  resolver->current_loop_fn_locals = enclosing_loop_fn_locals;
}

static void resolve_statement_for(FnResolver* resolver, AstStatement* stmt) {
  AstStatement* enclosing_loop = resolver->current_loop;
  resolver->current_loop       = stmt;

  stmt->base.scope = new_scope(resolver);

  AstNode* initializer = stmt->base.children[0];
  AstNode* condition   = stmt->base.children[1];
  AstNode* increment   = stmt->base.children[2];
  AstNode* body        = stmt->base.children[3];

  if (initializer != NULL) {
    resolve_node(resolver, initializer);
  }
  if (condition != NULL) {
    resolve_node(resolver, condition);
  }
  if (increment != NULL) {
    resolve_node(resolver, increment);
  }

  int enclosing_loop_fn_locals     = resolver->current_loop_fn_locals;
  resolver->current_loop_fn_locals = resolver->function->local_count;

  if (body != NULL) {
    resolve_node(resolver, body);
  }

  end_scope(resolver);

  resolver->current_loop           = enclosing_loop;
  resolver->current_loop_fn_locals = enclosing_loop_fn_locals;
}

static void resolve_statement_return(FnResolver* resolver, AstStatement* stmt) {
  if (resolver->function->type == FN_TYPE_CONSTRUCTOR && stmt->base.children[0] != NULL) {
    resolver_error(resolver, (AstNode*)stmt, "Can't return a value from a constructor.");
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
  if (resolver->current_loop == NULL) {
    resolver_error(resolver, (AstNode*)stmt, "Can't break outside of a loop.");
  }
  // Break needs to pop all locals that were declared in the loop (Before the break statement)
  stmt->locals_to_pop = resolver->function->local_count - resolver->current_loop_fn_locals;
}

static void resolve_statement_skip(FnResolver* resolver, AstStatement* stmt) {
  if (resolver->current_loop == NULL) {
    resolver_error(resolver, (AstNode*)stmt, "Can't skip outside of a loop.");
  }
  // Skip needs to pop all locals that were declared in the loop (Before the skip statement)
  stmt->locals_to_pop = resolver->function->local_count - resolver->current_loop_fn_locals;
}

static void resolve_statement_throw(FnResolver* resolver, AstStatement* stmt) {
  resolve_children(resolver, (AstNode*)stmt);
}

static void resolve_statement_try(FnResolver* resolver, AstStatement* stmt) {
  stmt->base.scope = new_scope(resolver);
  inject_local(resolver, KEYWORD_ERROR, false);
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
  // Only evaluate the target expression, not the property id - we don't know if that property exists at compile time. (yet)
  AstExpression* target = (AstExpression*)expr->base.children[0];
  resolve_node(resolver, (AstNode*)target);
}

static void resolve_expr_invoke(FnResolver* resolver, AstExpression* expr) {
  // Only resolve the target and arguments, not the method id - we don't know if that method exists at compile time. (yet)
  AstExpression* target = (AstExpression*)expr->base.children[0];
  resolve_node(resolver, (AstNode*)target);
  // Skip the method id, resolve the arguments
  for (int i = 2; i < expr->base.count; i++) {
    resolve_node(resolver, expr->base.children[i]);
  }
}

static void resolve_expr_subs(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_slice(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_this(FnResolver* resolver, AstExpression* expr) {
  if (!resolver->in_class) {
    resolver_error(resolver, (AstNode*)expr, "Can't use '" KEYWORD_THIS "' outside of a class.");
  } else if (resolver->function->type == FN_TYPE_METHOD_STATIC) {
    resolver_error(resolver, (AstNode*)expr, "Can't use '" KEYWORD_THIS "' in a static method.");
  }
  AstId* this_ = get_child_as_id((AstNode*)expr, 0, false);
  resolve_variable(resolver, this_);
}

static void resolve_expr_base(FnResolver* resolver, AstExpression* expr) {
  if (!resolver->in_class) {
    resolver_error(resolver, (AstNode*)expr, "Can't use '" KEYWORD_BASE "' outside of a class.");
  } else if (!resolver->has_baseclass) {
    resolver_error(resolver, (AstNode*)expr, "Can't use '" KEYWORD_BASE "' in a class without a base class.");
  } else if (resolver->function->type == FN_TYPE_METHOD_STATIC) {
    resolver_error(resolver, (AstNode*)expr, "Can't use '" KEYWORD_BASE "' in a static method.");
  }
  AstId* this_ = get_child_as_id((AstNode*)expr, 0, false);
  AstId* base_ = get_child_as_id((AstNode*)expr, 1, false);
  resolve_variable(resolver, this_);
  resolve_variable(resolver, base_);
}

static void resolve_expr_anon_fn(FnResolver* resolver, AstExpression* expr) {
  resolve_function(resolver, (AstFn*)expr->base.children[0]);
}

static void resolve_expr_ternary(FnResolver* resolver, AstExpression* expr) {
  resolve_children(resolver, (AstNode*)expr);
}

static void resolve_expr_try(FnResolver* resolver, AstExpression* expr) {
  expr->base.scope = new_scope(resolver);
  inject_local(resolver, KEYWORD_ERROR, false);

  AstId* error        = get_child_as_id((AstNode*)expr, 0, false);
  AstNode* try_expr   = expr->base.children[1];
  AstNode* catch_expr = expr->base.children[2];

  resolve_variable(resolver, error);
  resolve_node(resolver, try_expr);
  if (catch_expr != NULL) {
    resolve_node(resolver, catch_expr);
  }
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
      resolver->panic_mode = false;
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
      resolver->panic_mode = false;
      AstStatement* stmt   = (AstStatement*)node;
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
        case EXPR_INVOKE: resolve_expr_invoke(resolver, expr); break;
        case EXPR_SUBS: resolve_expr_subs(resolver, expr); break;
        case EXPR_SLICE: resolve_expr_slice(resolver, expr); break;
        case EXPR_THIS: resolve_expr_this(resolver, expr); break;
        case EXPR_BASE: resolve_expr_base(resolver, expr); break;
        case EXPR_ANONYMOUS_FN: resolve_expr_anon_fn(resolver, expr); break;
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

void verify_late_bound(FnResolver* resolver) {
  // Check for undeclared variables in the root scope - only globals can be late-bound and all of them should be resolved by now.
  for (int i = 0; i < resolver->root_scope->capacity; i++) {
    SymbolEntry* entry = &resolver->root_scope->entries[i];
    if (entry->key == NULL) {
      continue;
    }
    if (entry->value->type == SYMBOL_UNDEFINED) {
      resolver_error(resolver, entry->value->source, "Undefined variable '%s'.", entry->key->chars);
      resolver->panic_mode = false;
    }
  }
}

bool resolve(AstFn* ast, HashTable* global_scope, HashTable* native_scope, bool disable_warnings) {
  resolver_root = ast;

  FnResolver resolver;
  resolver_init(&resolver, NULL, ast);
  inject_local(&resolver, "", true);  // Local slot 0 is not accessible

  resolver.disable_warnings = disable_warnings;
  resolver.root_scope       = resolver.current_scope;  // The root scope is the first scope we create
  resolver.global_scope     = global_scope;
  resolver.native_scope     = native_scope;

  // Skip the first and second children, which are the name and parameters
  INTERNAL_ASSERT(ast->base.count == 3, "Function should have exactly 3 children.");
  resolve_node(&resolver, ast->base.children[2]);
  end_resolver(&resolver);

  verify_late_bound(&resolver);

#ifdef DEBUG_PRINT_SCOPES
  printf("\n\n\n === RESOLVE ===\n\n");
  ast_print_scopes((AstNode*)ast);
#endif

  resolver_root = NULL;
  return !resolver.had_error;
}

void resolver_mark_roots() {
  ast_mark((AstNode*)resolver_root);
}
