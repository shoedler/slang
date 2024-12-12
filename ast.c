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

AstRoot* ast_root_init(Token start) {
  AstRoot* root = (AstRoot*)ast_allocate_node(sizeof(AstRoot), NODE_ROOT, start, start);
  root->globals = NULL;
  return root;
}

void ast_root_add_child(AstRoot* root, AstNode* child) {
  ast_node_add_child((AstNode*)root, child);
}

AstId* ast_id_init(Token id, ObjString* name) {
  AstId* id_  = (AstId*)ast_allocate_node(sizeof(AstId), NODE_ID, id, id);
  id_->name   = name;
  id_->symbol = NULL;
  return id_;
}

//
// Declarations
//

static AstDeclaration* ast_decl_init(Token start, Token end, DeclarationType type) {
  AstDeclaration* decl = (AstDeclaration*)ast_allocate_node(sizeof(AstDeclaration), NODE_DECL, start, end);
  decl->type           = type;
  decl->fn_type        = FN_TYPE_UNKNOWN;
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

AstDeclaration* ast_decl_fn_init(Token start, Token end, AstId* name, FnType type, AstDeclaration* params, AstNode* body) {
  AstDeclaration* fn = ast_decl_init(start, end, DECL_FN);
  fn->fn_type        = type;
  ast_node_add_child((AstNode*)fn, (AstNode*)name);
  ast_node_add_child((AstNode*)fn, (AstNode*)params);
  ast_node_add_child((AstNode*)fn, body);
  return fn;
}

AstDeclaration* ast_decl_method_init(Token start, Token end, bool is_static, AstDeclaration* fn) {
  AstDeclaration* method = ast_decl_init(start, end, DECL_METHOD);
  method->is_static      = is_static;
  ast_node_add_child((AstNode*)method, (AstNode*)fn);
  return method;
}

AstDeclaration* ast_decl_ctor_init(Token start, Token end, AstDeclaration* fn) {
  AstDeclaration* ctor = ast_decl_init(start, end, DECL_CTOR);
  ast_node_add_child((AstNode*)ctor, (AstNode*)fn);
  return ctor;
}

AstDeclaration* ast_decl_class_init(Token start, Token end, AstId* name, AstId* baseclass_name) {
  AstDeclaration* class_decl = ast_decl_init(start, end, DECL_CLASS);
  ast_node_add_child((AstNode*)class_decl, (AstNode*)name);
  ast_node_add_child((AstNode*)class_decl, (AstNode*)baseclass_name);
  return class_decl;
}

void ast_decl_class_add_method_or_ctor(AstDeclaration* class_decl, AstDeclaration* method_or_ctor) {
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
  AstStatement* stmt = (AstStatement*)ast_allocate_node(sizeof(AstStatement), NODE_STMT, start, end);
  stmt->type         = type;
  stmt->path         = NULL;
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

AstStatement* ast_stmt_block_init(Token start, Token end) {
  AstStatement* block = ast_stmt_init(start, end, STMT_BLOCK);
  return block;
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

AstStatement* ast_stmt_try_init(Token start, Token end, AstStatement* try_block, AstStatement* catch_block) {
  AstStatement* stmt = ast_stmt_init(start, end, STMT_TRY);
  ast_node_add_child((AstNode*)stmt, (AstNode*)try_block);
  ast_node_add_child((AstNode*)stmt, (AstNode*)catch_block);
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

AstExpression* ast_expr_this_init(Token start, Token end) {
  return ast_expr_init(start, end, EXPR_THIS);
}

AstExpression* ast_expr_base_init(Token start, Token end) {
  return ast_expr_init(start, end, EXPR_BASE);
}

AstExpression* ast_expr_lambda_init(Token start, Token end, AstDeclaration* fn) {
  AstExpression* expr = ast_expr_init(start, end, EXPR_LAMBDA);
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

AstExpression* ast_expr_try_init(Token start, Token end, AstExpression* expr, AstExpression* else_expr) {
  AstExpression* try_expr = ast_expr_init(start, end, EXPR_TRY);
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
  lit->string     = NULL;
  return lit;
}

AstLiteral* ast_lit_str_init(Token start, Token end, ObjString* string) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_STRING);
  lit->string     = string;
  return lit;
}

AstLiteral* ast_lit_number_init(Token start, Token end, Value number) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_NUMBER);
  lit->value      = number;
  return lit;
}

AstLiteral* ast_lit_bool_init(Token start, Token end, Value boolean) {
  AstLiteral* lit = ast_lit_init(start, end, LIT_BOOL);
  lit->value      = boolean;
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

  if (node->scope != NULL) {
    scope_free(node->scope);
    free(node->scope);
  }
  free(node->children);
  free(node);
}

void ast_print(AstNode* node, int indent) {
  for (int i = 0; i < indent; i++) {
    printf(":  ");
  }

  if (node == NULL) {
    printf(ANSI_YELLOW_STR("NULL") "\n");
    return;
  }

  ast_node_print(node);

  // Print IDs and literals directly
  if (node->count == 1 && node->children[0] != NULL) {
    if (node->children[0]->type == NODE_ID) {
      printf(" -> ");
      ast_node_print(node->children[0]);
      printf("\n");
      return;
    } else if (node->children[0]->type == NODE_LIT) {
      printf(" -> ");
      ast_node_print(node->children[0]);
      printf("\n");
      return;
    }
  }

  if (node->count > 0) {
    printf(" :");
  }
  printf("\n");

  for (int i = 0; i < node->count; i++) {
    ast_print(node->children[i], indent + 1);
  }
}

void print_string_lit(const char* str, int max_len) {
  int count = 0;
  printf(ANSI_COLOR_BLUE "\"");
  for (const char* p = str; *p; p++) {
    if (count++ >= max_len) {
      printf("...");
      break;
    }

    if (*p == '\n') {
      printf("\\n");
    } else if (*p == '\t') {
      printf("\\t");
    } else if (*p == '\r') {
      printf("\\r");
    } else if (*p == '\v') {
      printf("\\v");
    } else if (*p == '\b') {
      printf("\\b");
    } else if (*p == '\f') {
      printf("\\f");
    } else if (*p == '\a') {
      printf("\\a");
    } else if (*p == '\\') {
      printf("\\\\");
    } else if (*p == '\"') {
      printf("\\\"");
    } else if (*p == '\'') {
      printf("\\\'");
    } else {
      printf("%c", *p);
    }
  }
  printf("\"" ANSI_COLOR_RESET);
}

static void ast_node_print(AstNode* node) {
  switch (node->type) {
    case NODE_ROOT: {
      printf(STR(NODE_ROOT));
      break;
    }
    case NODE_ID: {
      printf(ANSI_MAGENTA_STR("%s"), ((AstId*)node)->name->chars);
      break;
    }
    case NODE_DECL: {
      switch (((AstDeclaration*)node)->type) {
        case DECL_FN: printf(STR(DECL_FN)); break;
        case DECL_FN_PARAMS: printf(STR(DECL_FN_PARAMS)); break;
        case DECL_CLASS: printf(STR(DECL_CLASS)); break;
        case DECL_METHOD: printf(STR(DECL_METHOD)); break;
        case DECL_CTOR: printf(STR(DECL_CTOR)); break;
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
        case EXPR_SUBS: printf(STR(EXPR_SUBS)); break;
        case EXPR_SLICE: printf(STR(EXPR_SLICE)); break;
        case EXPR_THIS: printf(STR(EXPR_THIS)); break;
        case EXPR_BASE: printf(STR(EXPR_BASE)); break;
        case EXPR_LAMBDA: printf(STR(EXPR_LAMBDA)); break;
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
            printf(ANSI_BLUE_STR("%g"), ((AstLiteral*)node)->value.as.float_);
          } else {
            printf(ANSI_BLUE_STR("%llu"), ((AstLiteral*)node)->value.as.integer);
          }
          break;
        }
        case LIT_STRING: print_string_lit(((AstLiteral*)node)->string->chars, 30); break;
        case LIT_BOOL:
          printf(ANSI_BLUE_STR("%s"), (((AstLiteral*)node)->value.as.boolean) ? VALUE_STR_TRUE : VALUE_STR_FALSE);
          break;
        case LIT_NIL: printf(ANSI_BLUE_STR(VALUE_STR_NIL)); break;
        case LIT_TUPLE: printf(ANSI_BLUE_STR("(" STR(LIT_TUPLE) ")")); break;
        case LIT_SEQ: printf(ANSI_BLUE_STR("[" STR(LIT_SEQ) "]")); break;
        case LIT_OBJ: printf(ANSI_BLUE_STR("{" STR(LIT_OBJ) "}")); break;
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