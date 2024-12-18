#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "object.h"
#include "scanner.h"
#include "scope.h"
#include "value.h"

#define MAX_LOCALS (1024 * 3)    // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.
#define MAX_UPVALUES (1024 * 3)  // Arbitrary, could to UINT32_MAX theoretically, but gl with an array of that size on the stack.

typedef enum {
  DECL_FN,         // Function declaration
  DECL_FN_PARAMS,  // Function parameters
  DECL_CLASS,      // Class declaration
  DECL_VARIABLE    // Variable declaration (let/const)
} DeclarationType;

typedef enum {
  STMT_IMPORT,  // Import declaration
  STMT_BLOCK,   // Block of statements
  STMT_IF,      // If statement
  STMT_WHILE,   // While loop
  STMT_FOR,     // For loop
  STMT_RETURN,  // Return statement
  STMT_PRINT,   // Print statement
  STMT_EXPR,    // Expression statement
  STMT_BREAK,   // Break statement
  STMT_SKIP,    // Skip (continue) statement
  STMT_THROW,   // Throw statement
  STMT_TRY      // Try statement
} StatementType;

typedef enum {
  EXPR_BINARY,        // Binary operation (+, -, *, /, etc)
  EXPR_POSTFIX,       // Postfix inc/dec (x++, x--)
  EXPR_UNARY,         // Unary operation (!, -) and prefix inc/dec (++x, --x)
  EXPR_GROUPING,      // Parenthesized expression
  EXPR_LITERAL,       // Literal value
  EXPR_VARIABLE,      // Variable reference
  EXPR_ASSIGN,        // Assignment
  EXPR_AND,           // Logical AND
  EXPR_OR,            // Logical OR
  EXPR_IS,            // Type check
  EXPR_IN,            // Contains check
  EXPR_CALL,          // Function call
  EXPR_DOT,           // Property access
  EXPR_INVOKE,        // Property invocation (method call)
  EXPR_SUBS,          // Subscript access
  EXPR_SLICE,         // Get slice
  EXPR_THIS,          // This expression
  EXPR_BASE,          // Base class reference
  EXPR_ANONYMOUS_FN,  // Anonymous function
  EXPR_TERNARY,       // Ternary operation (?:)
  EXPR_TRY,           // Try expression
} ExpressionType;

typedef enum {
  LIT_NUMBER,  // Number literal
  LIT_STRING,  // String literal
  LIT_BOOL,    // Boolean literal
  LIT_NIL,     // Nil literal
  LIT_TUPLE,   // Tuple literal
  LIT_SEQ,     // Sequence literal
  LIT_OBJ,     // Object literal
} LiteralType;

typedef enum {
  PAT_TUPLE,    // Tuple destructuring pattern
  PAT_SEQ,      // Sequence destructuring pattern
  PAT_OBJ,      // Object destructuring pattern
  PAT_BINDING,  // Binding pattern (variable)
  PAT_REST      // Rest pattern (...rest)
} PatternType;

typedef enum {
  NODE_ID,
  NODE_FN,
  NODE_BLOCK,
  NODE_DECL,
  NODE_STMT,
  NODE_EXPR,
  NODE_LIT,
  NODE_PATTERN,
} NodeType;

typedef enum {
  FN_TYPE_UNKNOWN,
  FN_TYPE_FUNCTION,
  FN_TYPE_CONSTRUCTOR,
  FN_TYPE_METHOD,
  FN_TYPE_METHOD_STATIC,
  FN_TYPE_ANONYMOUS_FUNCTION,
  FN_TYPE_MODULE
} FnType;

// The main AST node structure
typedef struct AstNode {
  NodeType type;           // Type of the node
  Token token_start;       // starting Token associated with this node (for error reporting)
  Token token_end;         // ending Token associated with this node (for error reporting) - can be the same as token_start
  struct AstNode* parent;  // Parent node (NULL for the root node)
  Scope* scope;            // Scope associated with this node. Mostly NULL, except for nodes that introduce a new scope

  int count;
  int capacity;
  struct AstNode** children;
} AstNode;

typedef struct AstId AstId;
typedef struct AstFn AstFn;
typedef struct AstBlock AstBlock;
typedef struct AstDeclaration AstDeclaration;
typedef struct AstStatement AstStatement;
typedef struct AstExpression AstExpression;
typedef struct AstLiteral AstLiteral;
typedef struct AstPattern AstPattern;

typedef struct {
  uint16_t index;
  bool is_local;
} Upvalue;

// Scopless, as opposed to a statement-block
struct AstBlock {
  AstNode base;
};

AstBlock* ast_block_init(Token start);
void ast_block_add(AstBlock* block, AstNode* decl_or_stmt);

struct AstFn {
  AstNode base;
  Upvalue upvalues[MAX_UPVALUES];
  int upvalue_count;  // Number of upvalues in the function, including sub-scopes
  FnType type;
  bool is_lambda;  // True if the function is a lambda function
};

AstFn* ast_fn_init(Token start, Token end, FnType type, AstId* name, AstDeclaration* params, AstBlock* body);
AstFn* ast_fn_init2(Token start, Token end, FnType type, AstId* name, AstDeclaration* params, AstExpression* body);

struct AstId {
  AstNode base;
  ObjString* name;
  Symbol* symbol;  // Symbol data, NULL if not resolved
};

AstId* ast_id_init(Token id, ObjString* name);

//
// Declarations
//

struct AstDeclaration {
  AstNode base;
  DeclarationType type;
  bool is_static;  // DECL_METHOD
  bool is_const;   // DECL_VARIABLE
};

AstDeclaration* ast_decl_fn_params_init(Token start, Token end);
void ast_decl_fn_params_add_param(AstDeclaration* params, AstId* id);
AstDeclaration* ast_decl_fn_init(Token start, Token end, AstFn* fn);
AstDeclaration* ast_decl_class_init(Token start, Token end, AstId* name, AstId* baseclass_name);
void ast_decl_class_add_method_or_ctor(AstDeclaration* class_decl, AstFn* method_or_ctor);
AstDeclaration* ast_decl_variable_init(Token start, Token end, bool is_const, AstId* id, AstExpression* initializer_expr);
AstDeclaration* ast_decl_variable_init2(Token start,
                                        Token end,
                                        bool is_const,
                                        AstPattern* pattern,
                                        AstExpression* initializer_expr);

//
// Statements
//

struct AstStatement {
  AstNode base;
  StatementType type;
  ObjString* path;  // STMT_IMPORT
};

AstStatement* ast_stmt_import_init(Token start, Token end, ObjString* path, AstId* id);
AstStatement* ast_stmt_import_init2(Token start, Token end, ObjString* path, AstPattern* pattern);
AstStatement* ast_stmt_block_init(Token start, Token end, AstBlock* block);
AstStatement* ast_stmt_if_init(Token start,
                               Token end,
                               AstExpression* condition,
                               AstStatement* then_branch,
                               AstStatement* else_branch);
AstStatement* ast_stmt_while_init(Token start, Token end, AstExpression* condition, AstStatement* body);
AstStatement* ast_stmt_for_init(Token start,
                                Token end,
                                AstNode* initializer,
                                AstExpression* condition,
                                AstExpression* increment,
                                AstStatement* body);

AstStatement* ast_stmt_return_init(Token start, Token end, AstExpression* expression);
AstStatement* ast_stmt_print_init(Token start, Token end, AstExpression* expression);
AstStatement* ast_stmt_expr_init(Token start, Token end, AstExpression* expression);
AstStatement* ast_stmt_break_init(Token start, Token end);
AstStatement* ast_stmt_skip_init(Token start, Token end);
AstStatement* ast_stmt_throw_init(Token start, Token end, AstExpression* expression);
AstStatement* ast_stmt_try_init(Token start, Token end, AstStatement* try_stmt, AstStatement* catch_stmt);

//
// Expressions
//

struct AstExpression {
  AstNode base;
  ExpressionType type;
  Token operator_;  // EXPR_ASSIGN, EXPR_UNARY, EXPR_POSTFIX, EXPR_BINARY
};

AstExpression* ast_expr_binary_init(Token start, Token end, Token operator_, AstExpression* left, AstExpression* right);
AstExpression* ast_expr_postfix_init(Token start, Token end, Token operator_, AstExpression* inner);
AstExpression* ast_expr_unary_init(Token start, Token end, Token operator_, AstExpression* inner);
AstExpression* ast_expr_grouping_init(Token start, Token end, AstExpression* inner);
AstExpression* ast_expr_literal_init(Token start, Token end, AstLiteral* literal);
AstExpression* ast_expr_variable_init(Token start, Token end, AstId* identifier);
AstExpression* ast_expr_assign_init(Token start, Token end, Token operator_, AstExpression* left, AstExpression* right);
AstExpression* ast_expr_and_init(Token start, Token end, AstExpression* left, AstExpression* right);
AstExpression* ast_expr_or_init(Token start, Token end, AstExpression* left, AstExpression* right);
AstExpression* ast_expr_is_init(Token start, Token end, AstExpression* left, AstExpression* right);
AstExpression* ast_expr_in_init(Token start, Token end, AstExpression* left, AstExpression* right);
AstExpression* ast_expr_call_init(Token start, Token end, AstExpression* target);
void ast_expr_call_add_argument(AstExpression* call, AstExpression* argument);
AstExpression* ast_expr_dot_init(Token start, Token end, AstExpression* target, AstId* property);
AstExpression* ast_expr_invoke_init(Token start, Token end, AstExpression* target, AstId* method);
void ast_expr_invoke_add_argument(AstExpression* invoke, AstExpression* argument);
AstExpression* ast_expr_subs_init(Token start, Token end, AstExpression* target, AstExpression* index);
AstExpression* ast_expr_slice_init(Token start,
                                   Token end,
                                   AstExpression* target,
                                   AstExpression* start_index,
                                   AstExpression* end_index);
AstExpression* ast_expr_this_init(Token start, Token end, AstId* this_);
AstExpression* ast_expr_base_init(Token start, Token end, AstId* this_, AstId* base_);
AstExpression* ast_expr_anon_fn_init(Token start, Token end, AstFn* fn);
AstExpression* ast_expr_ternary_init(Token start,
                                     Token end,
                                     AstExpression* condition,
                                     AstExpression* then_expr,
                                     AstExpression* else_expr);
AstExpression* ast_expr_try_init(Token start, Token end, AstExpression* expr, AstExpression* else_expr);

//
// Literals
//

struct AstLiteral {
  AstNode base;
  LiteralType type;
  Value value;        // LIT_NUMBER, LIT_BOOL, LIT_NIL
  ObjString* string;  // LIT_STRING
};

AstLiteral* ast_lit_str_init(Token start, Token end, ObjString* string);
AstLiteral* ast_lit_number_init(Token start, Token end, Value number);
AstLiteral* ast_lit_bool_init(Token start, Token end, Value boolean);
AstLiteral* ast_lit_nil_init(Token start, Token end);
AstLiteral* ast_lit_tuple_init(Token start, Token end);
void ast_lit_tuple_add_item(AstLiteral* tuple, AstExpression* item);
AstLiteral* ast_lit_seq_init(Token start, Token end);
void ast_lit_seq_add_item(AstLiteral* seq, AstExpression* item);
AstLiteral* ast_lit_obj_init(Token start, Token end);
void ast_lit_obj_add_kv_pair(AstLiteral* obj, AstExpression* key, AstExpression* value);

//
// Patterns
//

struct AstPattern {
  AstNode base;
  PatternType type;
};

AstPattern* ast_pattern_init(Token start, Token end, PatternType type);
void ast_pattern_add_element(AstPattern* pattern, AstPattern* element);
AstPattern* ast_pattern_binding_init(Token start, Token end, AstId* binding);
AstPattern* ast_pattern_rest_init(Token start, Token end, AstId* identifier);

// Creates a AST node with the given type and children
AstNode* ast_allocate_node(size_t size, NodeType type, Token start, Token end);

// Adds a child to an AST node
void ast_node_add_child(AstNode* parent, AstNode* child);

// Frees an AST node and all its children
void ast_free(AstNode* node);

// Prints the AST to stdout
void ast_print(AstNode* node, int indent);

const char* ast_fn_type_to_str(FnType type);

#endif  // AST_H