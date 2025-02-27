#include "ast.h"
#include <stdlib.h>
#include "common.h"
#include "memory.h"
#include "scope.h"
#include "vm.h"

// Forward declarations
static void ast_node_print(AstNode* node);

AstNode* ast_allocate_node(size_t size, NodeType type, Token start, Token end) {
  AstNode* node     = malloc(size);
  node->type        = type;
  node->token_start = start;
  node->token_end   = end;

  node->children = NULL;
  node->count    = 0;
  node->capacity = 0;

  node->parent = NULL;
  node->scope  = NULL;

  return node;
}

void ast_node_add_child(AstNode* parent, AstNode* child) {
  if (SHOULD_GROW(parent->count + 1, parent->capacity)) {
    int old_capacity = parent->capacity;
    parent->capacity = GROW_CAPACITY(old_capacity);
    parent->children = realloc(parent->children, sizeof(AstNode*) * parent->capacity);
  }

  parent->children[parent->count] = child;
  parent->count++;

  if (child != NULL) {
    child->parent = parent;  // Even if the parent is NULL
  }
}

AstBlock* ast_block_init(Token start) {
  AstBlock* block = (AstBlock*)ast_allocate_node(sizeof(AstBlock), NODE_BLOCK, start, start);
  return block;
}

void ast_block_add(AstBlock* block, AstNode* decl_or_stmt) {
  ast_node_add_child((AstNode*)block, decl_or_stmt);
}

static AstFn* ast_fn_init_(Token start, Token end, FnType type, AstId* name, AstDeclaration* params, AstNode* body) {
  AstFn* fn         = (AstFn*)ast_allocate_node(sizeof(AstFn), NODE_FN, start, end);
  fn->type          = type;
  fn->upvalue_count = 0;
  fn->local_count   = 0;
  fn->is_lambda     = false;

  ast_node_add_child((AstNode*)fn, (AstNode*)name);
  ast_node_add_child((AstNode*)fn, (AstNode*)params);
  ast_node_add_child((AstNode*)fn, body);
  return fn;
}

AstFn* ast_fn_init(Token start, Token end, FnType type, AstId* name, AstDeclaration* params, AstBlock* body) {
  return ast_fn_init_(start, end, type, name, params, (AstNode*)body);
}

AstFn* ast_fn_init2(Token start, Token end, FnType type, AstId* name, AstDeclaration* params, AstExpression* body) {
  AstFn* fn     = ast_fn_init_(start, end, type, name, params, (AstNode*)body);
  fn->is_lambda = true;
  return fn;
}

AstId* ast_id_init(Token id, ObjString* name) {
  AstId* id_ = (AstId*)ast_allocate_node(sizeof(AstId), NODE_ID, id, id);
  id_->name  = name;
  id_->ref   = NULL;
  return id_;
}

//
// Declarations
//

static AstDeclaration* ast_decl_init(Token start, Token end, DeclarationType type) {
  AstDeclaration* decl = (AstDeclaration*)ast_allocate_node(sizeof(AstDeclaration), NODE_DECL, start, end);
  decl->type           = type;
  decl->is_static      = false;
  decl->is_const       = false;
  return decl;
}

AstDeclaration* ast_decl_fn_params_init(Token start, Token end) {
  AstDeclaration* params = ast_decl_init(start, end, DECL_FN_PARAMS);
  return params;
}

void ast_decl_fn_params_add_param(AstDeclaration* params, AstId* id) {
  ast_node_add_child((AstNode*)params, (AstNode*)id);
}

AstDeclaration* ast_decl_fn_init(Token start, Token end, AstFn* fn) {
  AstDeclaration* fn_ = ast_decl_init(start, end, DECL_FN);
  ast_node_add_child((AstNode*)fn_, (AstNode*)fn);
  return fn_;
}

AstDeclaration* ast_decl_class_init(Token start, Token end, AstId* name, AstId* baseclass_name) {
  AstDeclaration* class_decl = ast_decl_init(start, end, DECL_CLASS);
  ast_node_add_child((AstNode*)class_decl, (AstNode*)name);
  ast_node_add_child((AstNode*)class_decl, (AstNode*)baseclass_name);
  return class_decl;
}

void ast_decl_class_add_method_or_ctor(AstDeclaration* class_decl, AstFn* method_or_ctor) {
  ast_node_add_child((AstNode*)class_decl, (AstNode*)method_or_ctor);
}

AstDeclaration* ast_decl_variable_init(Token start, Token end, bool is_const, AstId* id, AstExpression* initializer_expr) {
  AstDeclaration* var_decl = ast_decl_init(start, end, DECL_VARIABLE);
  var_decl->is_const       = is_const;
  ast_node_add_child((AstNode*)var_decl, (AstNode*)id);
  ast_node_add_child((AstNode*)var_decl, (AstNode*)initializer_expr);
  return var_decl;
}

AstDeclaration* ast_decl_variable_init2(Token start,
                                        Token end,
                                        bool is_const,
                                        AstPattern* pattern,
                                        AstExpression* initializer_expr) {
  AstDeclaration* var_decl = ast_decl_init(start, end, DECL_VARIABLE);
  var_decl->is_const       = is_const;
  ast_node_add_child((AstNode*)var_decl, (AstNode*)pattern);
  ast_node_add_child((AstNode*)var_decl, (AstNode*)initializer_expr);
  return var_decl;
}

//
// Statements
//

static AstStatement* ast_stmt_init(Token start, Token end, StatementType type) {
  AstStatement* stmt  = (AstStatement*)ast_allocate_node(sizeof(AstStatement), NODE_STMT, start, end);
  stmt->type          = type;
  stmt->path          = NULL;
  stmt->locals_to_pop = 0;
  return stmt;
}

AstStatement* ast_stmt_import_init(Token start, Token end, ObjString* path, AstId* id) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_IMPORT);
  stmt->path         = path;
  ast_node_add_child((AstNode*)stmt, (AstNode*)id);
  return stmt;
}

AstStatement* ast_stmt_import_init2(Token start, Token end, ObjString* path, AstPattern* pattern) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_IMPORT);
  stmt->path         = path;
  ast_node_add_child((AstNode*)stmt, (AstNode*)pattern);
  return stmt;
}

AstStatement* ast_stmt_block_init(Token start, Token end, AstBlock* block) {
  AstStatement* block_ = ast_stmt_init(start, end, STMT_BLOCK);
  ast_node_add_child((AstNode*)block_, (AstNode*)block);
  return block_;
}

void ast_stmt_block_add_statement(AstStatement* block, AstNode* decl_or_stmt) {
  ast_node_add_child((AstNode*)block, decl_or_stmt);
}

AstStatement* ast_stmt_if_init(Token start,
                               Token end,
                               AstExpression* condition,
                               AstStatement* then_branch,
                               AstStatement* else_branch) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_IF);
  ast_node_add_child((AstNode*)stmt, (AstNode*)condition);
  ast_node_add_child((AstNode*)stmt, (AstNode*)then_branch);
  ast_node_add_child((AstNode*)stmt, (AstNode*)else_branch);
  return stmt;
}

AstStatement* ast_stmt_while_init(Token start, Token end, AstExpression* condition, AstStatement* body) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_WHILE);
  ast_node_add_child((AstNode*)stmt, (AstNode*)condition);
  ast_node_add_child((AstNode*)stmt, (AstNode*)body);
  return stmt;
}

AstStatement* ast_stmt_for_init(Token start,
                                Token end,
                                AstNode* initializer,
                                AstExpression* condition,
                                AstExpression* increment,
                                AstStatement* body) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_FOR);
  ast_node_add_child((AstNode*)stmt, (AstNode*)initializer);
  ast_node_add_child((AstNode*)stmt, (AstNode*)condition);
  ast_node_add_child((AstNode*)stmt, (AstNode*)increment);
  ast_node_add_child((AstNode*)stmt, (AstNode*)body);
  return stmt;
}

AstStatement* ast_stmt_return_init(Token start, Token end, AstExpression* expression) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_RETURN);
  ast_node_add_child((AstNode*)stmt, (AstNode*)expression);
  return stmt;
}

AstStatement* ast_stmt_print_init(Token start, Token end, AstExpression* expression) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_PRINT);
  ast_node_add_child((AstNode*)stmt, (AstNode*)expression);
  return stmt;
}

AstStatement* ast_stmt_expr_init(Token start, Token end, AstExpression* expression) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_EXPR);
  ast_node_add_child((AstNode*)stmt, (AstNode*)expression);
  return stmt;
}

AstStatement* ast_stmt_break_init(Token start, Token end) {
  return ast_stmt_init(start, end, STMT_BREAK);
}

AstStatement* ast_stmt_skip_init(Token start, Token end) {
  return ast_stmt_init(start, end, STMT_SKIP);
}

AstStatement* ast_stmt_throw_init(Token start, Token end, AstExpression* expression) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_THROW);
  ast_node_add_child((AstNode*)stmt, (AstNode*)expression);
  return stmt;
}

AstStatement* ast_stmt_try_init(Token start, Token end, AstStatement* try_stmt, AstStatement* catch_stmt) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_TRY);
  ast_node_add_child((AstNode*)stmt, (AstNode*)try_stmt);
  ast_node_add_child((AstNode*)stmt, (AstNode*)catch_stmt);
  return stmt;
}

//
// Expressions
//

static AstExpression* ast_expr_init(Token start, Token end, ExpressionType type) {
  AstExpression* expr = (AstExpression*)ast_allocate_node(sizeof(AstExpression), NODE_EXPR, start, end);
  expr->type          = type;
  expr->operator_     = (Token){TOKEN_ERROR, NULL, 0, 0, false};
  return expr;
}

AstExpression* ast_expr_binary_init(Token start, Token end, Token operator_, AstExpression* left, AstExpression* right) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_BINARY);
  expr->operator_     = operator_;
  ast_node_add_child((AstNode*)expr, (AstNode*)left);
  ast_node_add_child((AstNode*)expr, (AstNode*)right);
  return expr;
}

AstExpression* ast_expr_postfix_init(Token start, Token end, Token operator_, AstExpression* inner) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_POSTFIX);
  expr->operator_     = operator_;
  ast_node_add_child((AstNode*)expr, (AstNode*)inner);
  return expr;
}

AstExpression* ast_expr_unary_init(Token start, Token end, Token operator_, AstExpression* inner) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_UNARY);
  expr->operator_     = operator_;
  ast_node_add_child((AstNode*)expr, (AstNode*)inner);
  return expr;
}

AstExpression* ast_expr_grouping_init(Token start, Token end, AstExpression* inner) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_GROUPING);
  ast_node_add_child((AstNode*)expr, (AstNode*)inner);
  return expr;
}

AstExpression* ast_expr_literal_init(Token start, Token end, AstLiteral* literal) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_LITERAL);
  ast_node_add_child((AstNode*)expr, (AstNode*)literal);
  return expr;
}

AstExpression* ast_expr_variable_init(Token start, Token end, AstId* identifier) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_VARIABLE);
  ast_node_add_child((AstNode*)expr, (AstNode*)identifier);
  return expr;
}

AstExpression* ast_expr_assign_init(Token start, Token end, Token operator_, AstExpression* left, AstExpression* right) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_ASSIGN);
  expr->operator_     = operator_;
  ast_node_add_child((AstNode*)expr, (AstNode*)left);
  ast_node_add_child((AstNode*)expr, (AstNode*)right);
  return expr;
}

AstExpression* ast_expr_and_init(Token start, Token end, AstExpression* left, AstExpression* right) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_AND);
  ast_node_add_child((AstNode*)expr, (AstNode*)left);
  ast_node_add_child((AstNode*)expr, (AstNode*)right);
  return expr;
}

AstExpression* ast_expr_or_init(Token start, Token end, AstExpression* left, AstExpression* right) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_OR);
  ast_node_add_child((AstNode*)expr, (AstNode*)left);
  ast_node_add_child((AstNode*)expr, (AstNode*)right);
  return expr;
}

AstExpression* ast_expr_is_init(Token start, Token end, AstExpression* left, AstExpression* right) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_IS);
  ast_node_add_child((AstNode*)expr, (AstNode*)left);
  ast_node_add_child((AstNode*)expr, (AstNode*)right);
  return expr;
}

AstExpression* ast_expr_in_init(Token start, Token end, AstExpression* left, AstExpression* right) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_IN);
  ast_node_add_child((AstNode*)expr, (AstNode*)left);
  ast_node_add_child((AstNode*)expr, (AstNode*)right);
  return expr;
}

AstExpression* ast_expr_call_init(Token start, Token end, AstExpression* target) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_CALL);
  ast_node_add_child((AstNode*)expr, (AstNode*)target);
  return expr;
}

void ast_expr_call_add_argument(AstExpression* call, AstExpression* argument) {
  ast_node_add_child((AstNode*)call, (AstNode*)argument);
}

AstExpression* ast_expr_dot_init(Token start, Token end, AstExpression* target, AstId* property) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_DOT);
  ast_node_add_child((AstNode*)expr, (AstNode*)target);
  ast_node_add_child((AstNode*)expr, (AstNode*)property);
  return expr;
}

AstExpression* ast_expr_invoke_init(Token start, Token end, AstExpression* target, AstId* method) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_INVOKE);
  ast_node_add_child((AstNode*)expr, (AstNode*)target);
  ast_node_add_child((AstNode*)expr, (AstNode*)method);
  return expr;
}

void ast_expr_invoke_add_argument(AstExpression* invoke, AstExpression* argument) {
  ast_node_add_child((AstNode*)invoke, (AstNode*)argument);
}

AstExpression* ast_expr_subs_init(Token start, Token end, AstExpression* target, AstExpression* index) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_SUBS);
  ast_node_add_child((AstNode*)expr, (AstNode*)target);
  ast_node_add_child((AstNode*)expr, (AstNode*)index);
  return expr;
}

AstExpression* ast_expr_slice_init(Token start,
                                   Token end,
                                   AstExpression* target,
                                   AstExpression* start_index,
                                   AstExpression* end_index) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_SLICE);
  ast_node_add_child((AstNode*)expr, (AstNode*)target);
  ast_node_add_child((AstNode*)expr, (AstNode*)start_index);
  ast_node_add_child((AstNode*)expr, (AstNode*)end_index);
  return expr;
}

AstExpression* ast_expr_this_init(Token start, Token end, AstId* this_) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_THIS);
  ast_node_add_child((AstNode*)expr, (AstNode*)this_);
  return expr;
}

AstExpression* ast_expr_base_init(Token start, Token end, AstId* this_, AstId* base_) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_BASE);
  ast_node_add_child((AstNode*)expr, (AstNode*)this_);
  ast_node_add_child((AstNode*)expr, (AstNode*)base_);
  return expr;
}

AstExpression* ast_expr_anon_fn_init(Token start, Token end, AstFn* fn) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_ANONYMOUS_FN);
  ast_node_add_child((AstNode*)expr, (AstNode*)fn);
  return expr;
}

AstExpression* ast_expr_ternary_init(Token start,
                                     Token end,
                                     AstExpression* condition,
                                     AstExpression* then_expr,
                                     AstExpression* else_expr) {
  AstExpression* ternary_expr = ast_expr_init(start, end, EXPR_TERNARY);
  ast_node_add_child((AstNode*)ternary_expr, (AstNode*)condition);
  ast_node_add_child((AstNode*)ternary_expr, (AstNode*)then_expr);
  ast_node_add_child((AstNode*)ternary_expr, (AstNode*)else_expr);
  return ternary_expr;
}

AstExpression* ast_expr_try_init(Token start, Token end, AstId* error, AstExpression* expr, AstExpression* else_expr) {
  AstExpression* try_expr = ast_expr_init(start, end, EXPR_TRY);
  ast_node_add_child((AstNode*)try_expr, (AstNode*)error);
  ast_node_add_child((AstNode*)try_expr, (AstNode*)expr);
  ast_node_add_child((AstNode*)try_expr, (AstNode*)else_expr);
  return try_expr;
}

//
// Literals
//

static AstLiteral* ast_lit_init(Token start, Token end, LiteralType type) {
  AstLiteral* lit = (AstLiteral*)ast_allocate_node(sizeof(AstLiteral), NODE_LIT, start, end);
  lit->type       = type;
  lit->value      = nil_value();
  return lit;
}

AstLiteral* ast_lit_str_init(Token start, Token end, ObjString* string) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_STRING);
  lit->value      = str_value(string);
  return lit;
}

AstLiteral* ast_lit_number_init(Token start, Token end, Value number) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_NUMBER);
  lit->value      = number;
  return lit;
}

AstLiteral* ast_lit_bool_init(Token start, Token end, bool boolean) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_BOOL);
  lit->value      = bool_value(boolean);
  return lit;
}

AstLiteral* ast_lit_nil_init(Token start, Token end) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_NIL);
  lit->value      = nil_value();
  return lit;
}

AstLiteral* ast_lit_tuple_init(Token start, Token end) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_TUPLE);
  return lit;
}

void ast_lit_tuple_add_item(AstLiteral* tuple, AstExpression* item) {
  ast_node_add_child((AstNode*)tuple, (AstNode*)item);
}

AstLiteral* ast_lit_seq_init(Token start, Token end) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_SEQ);
  return lit;
}

void ast_lit_seq_add_item(AstLiteral* seq, AstExpression* item) {
  ast_node_add_child((AstNode*)seq, (AstNode*)item);
}

AstLiteral* ast_lit_obj_init(Token start, Token end) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_OBJ);
  return lit;
}

void ast_lit_obj_add_kv_pair(AstLiteral* obj, AstExpression* key, AstExpression* value) {
  ast_node_add_child((AstNode*)obj, (AstNode*)key);
  ast_node_add_child((AstNode*)obj, (AstNode*)value);
}

//
// Patterns
//

AstPattern* ast_pattern_init(Token start, Token end, PatternType type) {
  AstPattern* pattern = (AstPattern*)ast_allocate_node(sizeof(AstPattern), NODE_PATTERN, start, end);
  pattern->type       = type;
  return pattern;
}

void ast_pattern_add_element(AstPattern* pattern, AstPattern* element) {
  ast_node_add_child((AstNode*)pattern, (AstNode*)element);
}

AstPattern* ast_pattern_binding_init(Token start, Token end, AstId* binding) {
  AstPattern* pattern = ast_pattern_init(start, end, PAT_BINDING);
  ast_node_add_child((AstNode*)pattern, (AstNode*)binding);
  return pattern;
}

AstPattern* ast_pattern_rest_init(Token start, Token end, AstId* identifier) {
  AstPattern* rest = ast_pattern_init(start, end, PAT_REST);
  ast_node_add_child((AstNode*)rest, (AstNode*)identifier);
  return rest;
}

void ast_free(AstNode* node) {
  if (node == NULL) {
    return;
  }

  // As always, use DFS - because we want to free the children first
  for (int i = 0; i < node->count; i++) {
    ast_free(node->children[i]);
  }

  // Ids need their symbol reference to be freed
  if (node->type == NODE_ID) {
    AstId* id = (AstId*)node;
    if (id->ref != NULL) {
      free(id->ref);
    }
  }

  // Some nodes have scopes, free them
  if (node->scope != NULL) {
    scope_free(node->scope);
    free(node->scope);
    node->scope = NULL;  // Prevent double free
  }
  free(node->children);
  free(node);
}

void ast_mark(AstNode* node) {
  if (node == NULL) {
    return;
  }

  if (node->type == NODE_ID) {
    AstId* id = (AstId*)node;
    if (id->name != NULL) {
      mark_obj((Obj*)id->name);
    }
  }
  if (node->type == NODE_STMT) {
    AstStatement* stmt = (AstStatement*)node;
    if (stmt->path != NULL) {
      mark_obj((Obj*)stmt->path);
    }
  }
  if (node->type == NODE_LIT) {
    AstLiteral* lit = (AstLiteral*)node;
    mark_value(lit->value);
  }

  for (int i = 0; i < node->count; i++) {
    ast_mark(node->children[i]);
  }
}

void ast_print(AstNode* node, int indent) {
  for (int i = 0; i < indent; i++) {
    printf(ANSI_GRAY_STR(":  "));
  }

  if (node == NULL) {
    printf(ANSI_YELLOW_STR("NULL") "\n");
    return;
  }

  ast_node_print(node);

  if (node->count > 0) {
    printf(" :");
  }
  printf("\n");

  for (int i = 0; i < node->count; i++) {
    ast_print(node->children[i], indent + 1);
  }
}

//
// Print AST
//

static void print_fn_type(FnType type) {
  const char* fn_type_str = ANSI_RED_STR("unknown");
  switch (type) {
    case FN_TYPE_NAMED_FUNCTION: fn_type_str = ANSI_GREEN_STR("function"); break;
    case FN_TYPE_CONSTRUCTOR: fn_type_str = ANSI_GREEN_STR("constructor"); break;
    case FN_TYPE_METHOD: fn_type_str = ANSI_GREEN_STR("method"); break;
    case FN_TYPE_METHOD_STATIC: fn_type_str = ANSI_GREEN_STR("static method"); break;
    case FN_TYPE_ANONYMOUS_FUNCTION: fn_type_str = ANSI_GREEN_STR("anonymous function"); break;
    case FN_TYPE_MODULE: fn_type_str = ANSI_GREEN_STR("module"); break;
    default: INTERNAL_ERROR("Unknown function type."); break;
  }

  printf(fn_type_str);
}

static void ast_node_print(AstNode* node) {
  switch (node->type) {
    case NODE_BLOCK: {
      printf(STR(NODE_BLOCK));
      break;
    }
    case NODE_ID: {
      printf(STR(NODE_ID) " " ANSI_MAGENTA_STR("%s"), ((AstId*)node)->name->chars);
      break;
    }
    case NODE_FN: {
      printf(STR(NODE_FN) " ");
      print_fn_type(((AstFn*)node)->type);
      break;
    }
    case NODE_DECL: {
      switch (((AstDeclaration*)node)->type) {
        case DECL_FN: printf(STR(DECL_FN)); break;
        case DECL_FN_PARAMS: printf(STR(DECL_FN_PARAMS)); break;
        case DECL_CLASS: printf(STR(DECL_CLASS)); break;
        case DECL_VARIABLE:
          printf(STR(DECL_VARIABLE) ANSI_GREEN_STR(" %s"), ((AstDeclaration*)node)->is_const ? "const" : "mutable");
          break;
        default: printf(ANSI_RED_STR("DECL_UNKNOWN %d"), node->type); break;
      }
      break;
    }
    case NODE_STMT: {
      switch (((AstStatement*)node)->type) {
        case STMT_IMPORT: printf(STR(STMT_IMPORT)); break;
        case STMT_BLOCK: printf(STR(STMT_BLOCK)); break;
        case STMT_IF: printf(STR(STMT_IF)); break;
        case STMT_WHILE: printf(STR(STMT_WHILE)); break;
        case STMT_FOR: printf(STR(STMT_FOR)); break;
        case STMT_RETURN: printf(STR(STMT_RETURN)); break;
        case STMT_PRINT: printf(STR(STMT_PRINT)); break;
        case STMT_EXPR: printf(STR(STMT_EXPR)); break;
        case STMT_BREAK: printf(STR(STMT_BREAK)); break;
        case STMT_SKIP: printf(STR(STMT_SKIP)); break;
        case STMT_THROW: printf(STR(STMT_THROW)); break;
        case STMT_TRY: printf(STR(STMT_TRY)); break;
        default: printf(ANSI_RED_STR("STMT_UNKNOWN %d"), node->type); break;
      }
      break;
    }
    case NODE_EXPR: {
      switch (((AstExpression*)node)->type) {
        case EXPR_BINARY: printf(STR(EXPR_BINARY)); break;
        case EXPR_POSTFIX: printf(STR(EXPR_POSTFIX)); break;
        case EXPR_UNARY: printf(STR(EXPR_UNARY)); break;
        case EXPR_GROUPING: printf(STR(EXPR_GROUPING)); break;
        case EXPR_LITERAL: printf(STR(EXPR_LITERAL)); break;
        case EXPR_VARIABLE: printf(STR(EXPR_VARIABLE)); break;
        case EXPR_ASSIGN: printf(STR(EXPR_ASSIGN)); break;
        case EXPR_AND: printf(STR(EXPR_AND)); break;
        case EXPR_OR: printf(STR(EXPR_OR)); break;
        case EXPR_IS: printf(STR(EXPR_IS)); break;
        case EXPR_IN: printf(STR(EXPR_IN)); break;
        case EXPR_CALL: printf(STR(EXPR_CALL)); break;
        case EXPR_DOT: printf(STR(EXPR_DOT)); break;
        case EXPR_INVOKE: printf(STR(EXPR_INVOKE)); break;
        case EXPR_SUBS: printf(STR(EXPR_SUBS)); break;
        case EXPR_SLICE: printf(STR(EXPR_SLICE)); break;
        case EXPR_THIS: printf(STR(EXPR_THIS)); break;
        case EXPR_BASE: printf(STR(EXPR_BASE)); break;
        case EXPR_ANONYMOUS_FN: printf(STR(EXPR_ANONYMOUS_FN)); break;
        case EXPR_TERNARY: printf(STR(EXPR_TERNARY)); break;
        case EXPR_TRY: printf(STR(EXPR_TRY)); break;
        default: printf(ANSI_RED_STR("EXPR_UNKNOWN %d"), node->type); break;
      }
      break;
    }
    case NODE_LIT: {
      switch (((AstLiteral*)node)->type) {
        case LIT_NUMBER: {
          if (is_float(((AstLiteral*)node)->value)) {
            printf(STR(LIT_NUMBER) " " ANSI_BLUE_STR("%g"), ((AstLiteral*)node)->value.as.float_);
          } else {
            printf(STR(LIT_NUMBER) " " ANSI_BLUE_STR("%llu"), ((AstLiteral*)node)->value.as.integer);
          }
          break;
        }
        case LIT_STRING: {
          printf(STR(LIT_STRING) " ");
#ifdef SLANG_ENABLE_COLOR_OUTPUT
          printf(ANSI_COLOR_BLUE);
#endif
          fprint_string_escaped(stdout, AS_STR(((AstLiteral*)node)->value)->chars, 30, true);
#ifdef SLANG_ENABLE_COLOR_OUTPUT
          printf(ANSI_COLOR_RESET);
#endif
          break;
        }
        case LIT_BOOL:
          printf(STR(LIT_BOOL) " " ANSI_BLUE_STR("%s"),
                 (((AstLiteral*)node)->value.as.boolean) ? VALUE_STR_TRUE : VALUE_STR_FALSE);
          break;
        case LIT_NIL: printf(STR(LIT_NIL) " " ANSI_BLUE_STR(VALUE_STR_NIL)); break;
        case LIT_TUPLE: printf(STR(LIT_TUPLE) " " ANSI_BLUE_STR("(" STR(LIT_TUPLE) ")")); break;
        case LIT_SEQ: printf(STR(LIT_SEQ) " " ANSI_BLUE_STR("[" STR(LIT_SEQ) "]")); break;
        case LIT_OBJ: printf(STR(LIT_OBJ) " " ANSI_BLUE_STR("{" STR(LIT_OBJ) "}")); break;
        default: printf(ANSI_RED_STR("LIT_UNKNOWN %d"), node->type); break;
      }
      break;
    }
    case NODE_PATTERN: {
      switch (((AstPattern*)node)->type) {
        case PAT_TUPLE: printf(STR(PAT_TUPLE)); break;
        case PAT_SEQ: printf(STR(PAT_SEQ)); break;
        case PAT_OBJ: printf(STR(PAT_OBJ)); break;
        case PAT_REST: printf(STR(PAT_REST)); break;
        case PAT_BINDING: printf(STR(PAT_BINDING)); break;
        default: printf(ANSI_RED_STR("PAT_UNKNOWN %d"), node->type); break;
      }
      break;
    }
    default: {
      printf(ANSI_RED_STR("NODE_UNKNOWN %d"), node->type);
      break;
    }
  }
}

//
// Print Scopes
//

static void print_scope_symbolentry(int depth, SymbolEntry* entry, const char* tree_link) {
  for (int i = 0; i < depth; i++) {
    printf(ANSI_GRAY_STR(":  "));
  }

  if (entry->key->length == 0) {
    printf("%s " ANSI_MAGENTA_STR("<reserved>") " ", tree_link);
  } else {
    printf("%s " ANSI_MAGENTA_STR("%s") " ", tree_link, entry->key->chars);
  }

  switch (entry->value->type) {
    case (SYMBOL_LOCAL): {
      printf(ANSI_CYAN_STR("[local]") " ");
      printf("fn-local-idx=%d ", entry->value->function_index);
      printf("local-idx=%d ", entry->value->index);
      printf(entry->value->is_captured ? (ANSI_GREEN_STR("(captured)")) : "");
      break;
    }
    case (SYMBOL_GLOBAL): {
      printf(ANSI_BLUE_STR("[global]") " ");
      break;
    }
    case (SYMBOL_NATIVE): {
      printf(ANSI_RED_STR("[native]") " ");
      break;
    }
    default: {
      INTERNAL_ASSERT(false, "Unknown symbol type: %d", entry->value->type);
      break;
    }
  }

  printf("%s\n", entry->value->is_param ? ANSI_YELLOW_STR("(param)") : "");
}

static void print_scope(Scope* scope) {
  for (int i = 0; i < scope->depth; i++) {
    printf(ANSI_GRAY_STR(":  "));
  }
  printf("SCOPE " ANSI_BLUE_STR("depth=%d") " :\n", scope->depth);

  int total_symbols_printed = 0;

  // Empty
  if (scope->count == 0) {
    for (int i = 0; i < scope->depth; i++) {
      printf(ANSI_GRAY_STR(":  "));
    }

    printf("%s " ANSI_GRAY_STR("empty") "\n", "└");
    return;
  }

  int count = scope->count;
  SymbolEntry entries[count];
  scope_get_all(scope, entries);

  for (SymbolEntry* entry = entries; entry < entries + count; entry++) {
    const char* tree_link = ++total_symbols_printed == scope->count ? "└" : "├";
    print_scope_symbolentry(scope->depth, entry, tree_link);
  }
}

static void print_scope_end(Scope* scope) {
  for (int i = 0; i < scope->depth; i++) {
    printf(ANSI_GRAY_STR(":  "));
  }
  printf("SCOPE_END " ANSI_BLUE_STR("depth=%d") " \n", scope->depth);
}

void ast_print_scopes(AstNode* node) {
  if (node == NULL) {
    return;
  }

  if (node->scope != NULL) {
    print_scope(node->scope);
  }

  for (int i = 0; i < node->count; i++) {
    ast_print_scopes(node->children[i]);
  }

  if (node->scope != NULL) {
    print_scope_end(node->scope);
  }
}
