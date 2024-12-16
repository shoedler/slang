#include "compiler2.h"
#include <stdarg.h>
#include <stdlib.h>
#include "ast.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "scope.h"
#include "vm.h"

typedef struct FnCompiler FnCompiler;

struct FnCompiler {
  struct FnCompiler* enclosing;
  AstFn* function;
  ObjFunction* result;

  // Brake jumps need to be stored because we don't know the offset of the jump when we compile them.
  int brakes_count;
  int brakes_capacity;
  int* brake_jumps;

  // Loop state
  int innermost_loop_start;
  int innermost_loop_scope_depth;
};

// Forward declarations
void compile_children(FnCompiler* compiler, AstNode* node);
static void compile_node(FnCompiler* compiler, AstNode* node);
static void emit_return(FnCompiler* compiler, AstNode* source);

static void compiler_init(FnCompiler* compiler, FnCompiler* enclosing, AstFn* function, ObjObject* globals_context) {
  compiler->enclosing               = enclosing;
  compiler->function                = function;
  compiler->result                  = new_function();
  compiler->result->upvalue_count   = function->upvalue_count;
  compiler->result->globals_context = globals_context;
  compiler->result->name            = ((AstId*)function->base.children[0])->name;

  compiler->brakes_count    = 0;
  compiler->brakes_capacity = 0;
  compiler->brake_jumps     = NULL;
}

static ObjFunction* end_compiler(FnCompiler* compiler) {
  emit_return(compiler, (AstNode*)compiler->function);
  ObjFunction* function = compiler->result;
#ifdef DEBUG_COMPILER
  debug_disassemble_chunk(&function->chunk, function->name->chars);
  // Print all locals
  Scope* scope = compiler->function->base.scope;
  for (int i = 0; i < scope->capacity; i++) {
    SymbolEntry* entry = &scope->entries[i];
    if (entry->key == NULL || entry->value->type != SYMBOL_LOCAL) {
      continue;
    }
    printf("Local %d: %s\n", entry->value->function_index, entry->key->chars);
  }
  printf("\n");
#endif

  return function;
}

static void compiler_free(FnCompiler* compiler) {
  FREE_ARRAY(int, compiler->brake_jumps, compiler->brakes_capacity);
}

static inline bool in_global_scope(FnCompiler* compiler) {
  return compiler->enclosing == NULL;
}

// Prints an error message at the offending node.
static void compiler_error(AstNode* offending_node, const char* format, ...) {
  fprintf(stderr, "Compiler error at line %d", offending_node->token_start.line);
  fprintf(stderr, ": " ANSI_COLOR_RED);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, ANSI_COLOR_RESET "\n");
  va_end(args);

  SourceView source = chunk_make_source_view(offending_node->token_start, offending_node->token_end);
  report_error_location(source);
}

//
// Emission
//

// Adds a value to the current constant pool and returns its index.
static uint16_t make_constant(FnCompiler* compiler, Value value, AstNode* source) {
  int constant = chunk_add_constant(&compiler->result->chunk, value);
  if (constant > MAX_CONSTANTS) {
    compiler_error(source, "Too many constants in one chunk. Max is " STR(MAX_CONSTANTS));
    return 0;
  }

  return (uint16_t)constant;
}

// Writes an opcode or operand to the current chunk. [source] is the node that generated the instruction and is
// used for error reporting in the vm via a lightweight source view.
static void emit_one(FnCompiler* compiler, uint16_t data, AstNode* source) {
  chunk_write(&compiler->result->chunk, data, source->token_start, source->token_end);
}
// See emit_one
static void emit_two(FnCompiler* compiler, uint16_t data1, uint16_t data2, AstNode* source) {
  emit_one(compiler, data1, source);
  emit_one(compiler, data2, source);
}
// // See emit_one
// static void emit_three(FnCompiler* compiler, uint16_t data1, uint16_t data2, uint16_t data3, AstNode* source) {
//   emit_one(compiler, data1, source);
//   emit_one(compiler, data2, source);
//   emit_one(compiler, data3, source);
// }

static void emit_return(FnCompiler* compiler, AstNode* source) {
  if (compiler->function->type == FN_TYPE_CONSTRUCTOR) {
    emit_two(compiler, OP_GET_LOCAL, 0, source);  // Return class instance, e.g. 'this'.
  } else {
    emit_one(compiler, OP_NIL, source);
  }

  emit_one(compiler, OP_RETURN, source);
}

static void emit_constant(FnCompiler* compiler, Value value, AstNode* source) {
  emit_two(compiler, OP_CONSTANT, make_constant(compiler, value, source), source);
}

// Emits a jump instruction and returns the offset of the jump instruction. Along with the emitted jump instruction, a 16-bit
// placeholder for the operand is emitted.
static int emit_jump(FnCompiler* compiler, uint16_t instruction, AstNode* source) {
  emit_one(compiler, instruction, source);
  emit_one(compiler, UINT16_MAX, source);
  return compiler->result->chunk.count - 1;
}

// Patches a previously emitted jump instruction with the actual jump offset. The offset is calculated by subtracting the current
// chunk's count from the offset of the jump instruction.
static void patch_jump(FnCompiler* compiler, int offset) {
  // -1 to adjust for the bytecode for the jump offset itself.
  int jump = compiler->result->chunk.count - offset - 1;

  if (jump > MAX_JUMP) {
    compiler_error((AstNode*)compiler->function, "Too much code to jump over, cannot jump over %d opcodes. Max is " STR(MAX_JUMP),
                   jump);
  }

  compiler->result->chunk.code[offset] = (uint16_t)jump;
}

//
// Important stuff
//

// Compiles a function.
static ObjFunction* compile_function(FnCompiler* compiler, AstFn* fn) {
  FnCompiler subcompiler;
  compiler_init(&subcompiler, compiler, fn, compiler->result->globals_context);

  compile_children(&subcompiler, (AstNode*)fn);
  if (fn->is_lambda) {
    emit_one(&subcompiler, OP_RETURN, (AstNode*)fn);
  }

  ObjFunction* function = end_compiler(&subcompiler);
  emit_two(compiler, OP_CLOSURE, make_constant(compiler, fn_value((Obj*)function), (AstNode*)fn), (AstNode*)fn);
  for (int i = 0; i < function->upvalue_count; i++) {
    emit_one(compiler, fn->upvalues[i].is_local ? 1 : 0, (AstNode*)fn);
    emit_one(compiler, fn->upvalues[i].index, (AstNode*)fn);
  }

  compiler_free(&subcompiler);

  return function;
}

//
// Declarations
//

static void compile_declare_function(FnCompiler* compiler, AstDeclaration* decl) {
  printf("compiling declaration\n");
  compile_children(compiler, (AstNode*)decl);
}
static void compile_declare_class(FnCompiler* compiler, AstDeclaration* decl) {
  printf("compiling declaration\n");
  compile_children(compiler, (AstNode*)decl);
}
static void compile_declare_variable(FnCompiler* compiler, AstDeclaration* decl) {
  printf("compiling declaration\n");
  compile_children(compiler, (AstNode*)decl);
}

//
// Statements
//

static void compile_statement_import(FnCompiler* compiler, AstStatement* stmt) {
  printf("compiling statement\n");
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_block(FnCompiler* compiler, AstStatement* stmt) {
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_if(FnCompiler* compiler, AstStatement* stmt) {
  printf("compiling statement\n");
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_while(FnCompiler* compiler, AstStatement* stmt) {
  printf("compiling statement\n");
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_for(FnCompiler* compiler, AstStatement* stmt) {
  printf("compiling statement\n");
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_return(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  if (expr != NULL) {
    compile_node(compiler, expr);
    emit_one(compiler, OP_RETURN, expr);
  } else {
    emit_return(compiler, (AstNode*)stmt);
  }
}
static void compile_statement_print(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  compile_node(compiler, expr);
  emit_one(compiler, OP_PRINT, expr);
}
static void compile_statement_expr(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  compile_node(compiler, expr);
  emit_one(compiler, OP_POP, expr);
}
static void compile_statement_break(FnCompiler* compiler, AstStatement* stmt) {
  printf("compiling statement\n");
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_skip(FnCompiler* compiler, AstStatement* stmt) {
  // INTERNAL_ASSERT(compiler->innermost_loop_start != -1, "Skip statement outside of loop. That's resolver business.");
  printf("compiling statement\n");
  compile_children(compiler, (AstNode*)stmt);
}
static void compile_statement_throw(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  compile_node(compiler, expr);
  emit_one(compiler, OP_THROW, expr);
}
static void compile_statement_try(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* try_stmt   = stmt->base.children[0];
  AstNode* catch_stmt = stmt->base.children[1];

  int try_jump = emit_jump(compiler, OP_TRY, (AstNode*)stmt);
  compile_node(compiler, try_stmt);  // Try statement

  // If the try stmt was successful, skip the catch stmt.
  int success_jump = emit_jump(compiler, OP_JUMP, try_stmt);
  patch_jump(compiler, try_jump);

  if (catch_stmt != NULL) {
    compile_node(compiler, catch_stmt);  // Catch statement
  }
  patch_jump(compiler, success_jump);  // Skip the catch block if the try block was successful.
}

//
// Expressions
//

static void compile_expr_binary(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  compile_node(compiler, right);
  switch (expr->operator_.type) {
    case TOKEN_NEQ: emit_one(compiler, OP_NEQ, (AstNode*)expr); break;
    case TOKEN_EQ: emit_one(compiler, OP_EQ, (AstNode*)expr); break;
    case TOKEN_GT: emit_one(compiler, OP_GT, (AstNode*)expr); break;
    case TOKEN_GTEQ: emit_one(compiler, OP_GTEQ, (AstNode*)expr); break;
    case TOKEN_LT: emit_one(compiler, OP_LT, (AstNode*)expr); break;
    case TOKEN_LTEQ: emit_one(compiler, OP_LTEQ, (AstNode*)expr); break;
    case TOKEN_PLUS: emit_one(compiler, OP_ADD, (AstNode*)expr); break;
    case TOKEN_MINUS: emit_one(compiler, OP_SUBTRACT, (AstNode*)expr); break;
    case TOKEN_MULT: emit_one(compiler, OP_MULTIPLY, (AstNode*)expr); break;
    case TOKEN_DIV: emit_one(compiler, OP_DIVIDE, (AstNode*)expr); break;
    case TOKEN_MOD: emit_one(compiler, OP_MODULO, (AstNode*)expr); break;
    default: INTERNAL_ERROR("Unhandled binary operator type: %d", expr->operator_.type); break;
  }
}
static void compile_expr_postfix(FnCompiler* compiler, AstExpression* expr) {
  // AstNode* inner = expr->base.children[0];

  // compile_node(compiler, inner);
  // emit_constant(compiler, int_value(1), (AstNode*)expr);
  // switch (expr->operator_.type) {
  //   case TOKEN_PLUS_PLUS: emit_one(compiler, OP_ADD, (AstNode*)expr); break;
  //   case TOKEN_MINUS_MINUS: emit_one(compiler, OP_SUBTRACT, (AstNode*)expr); break;
  //   default: INTERNAL_ERROR("Unhandled postfix operator type: %d", expr->operator_.type); break;
  // }
  // TODO: Emit the set instruction.
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_unary(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_grouping(FnCompiler* compiler, AstExpression* expr) {
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_literal(FnCompiler* compiler, AstExpression* expr) {
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_variable(FnCompiler* compiler, AstExpression* expr) {
  // AstId* var = (AstId*)expr->base.children[0];
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_assign(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_and(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  int end_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, left);
  emit_one(compiler, OP_POP, left);  // Discard the left value.

  compile_node(compiler, right);
  patch_jump(compiler, end_jump);
}
static void compile_expr_or(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  int else_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, left);
  int end_jump  = emit_jump(compiler, OP_JUMP, left);

  patch_jump(compiler, else_jump);
  emit_one(compiler, OP_POP, left);  // Discard the left value.

  compile_node(compiler, right);
  patch_jump(compiler, end_jump);
}
static void compile_expr_is(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  compile_node(compiler, right);
  emit_one(compiler, OP_IS, (AstNode*)expr);
}
static void compile_expr_in(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  compile_node(compiler, right);
  emit_one(compiler, OP_IN, (AstNode*)expr);
}
static void compile_expr_call(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_dot(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_subs(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_slice(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_this(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_base(FnCompiler* compiler, AstExpression* expr) {
  printf("compiling expression\n");
  compile_children(compiler, (AstNode*)expr);
}
static void compile_expr_anon_fn(FnCompiler* compiler, AstExpression* expr) {
  compile_function(compiler, (AstFn*)expr->base.children[0]);
}
static void compile_expr_ternary(FnCompiler* compiler, AstExpression* expr) {
  AstNode* condition    = expr->base.children[0];
  AstNode* true_branch  = expr->base.children[1];
  AstNode* false_branch = expr->base.children[2];

  compile_node(compiler, condition);  // Condition
  int else_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, condition);
  emit_one(compiler, OP_POP, condition);  // Discard the condition.

  compile_node(compiler, true_branch);  // True branch
  int end_jump = emit_jump(compiler, OP_JUMP, true_branch);

  patch_jump(compiler, else_jump);
  emit_one(compiler, OP_POP, true_branch);  // Discard the true branch.

  compile_node(compiler, false_branch);  // False branch
  patch_jump(compiler, end_jump);
}
static void compile_expr_try(FnCompiler* compiler, AstExpression* expr) {
  AstNode* try_expr   = expr->base.children[0];
  AstNode* catch_expr = expr->base.children[1];

  int error_index = scope_get(expr->base.scope, copy_string(KEYWORD_ERROR, STR_LEN(KEYWORD_ERROR)))
                        ->function_index;  // TODO (optimize): Use lookup with cached string.

  int try_jump = emit_jump(compiler, OP_TRY, (AstNode*)expr);
  compile_node(compiler, try_expr);                         // Try expression
  emit_two(compiler, OP_SET_LOCAL, error_index, try_expr);  // Set the error variable to the result of the try expression.

  // If the try block was successful, skip the catch block.
  int success_jump = emit_jump(compiler, OP_JUMP, try_expr);
  patch_jump(compiler, try_jump);

  if (catch_expr == NULL) {
    emit_one(compiler, OP_NIL, try_expr);  // Push nil as the "else" value.
  } else {
    compile_node(compiler, catch_expr);  // Catch expression
  }
  emit_two(compiler, OP_SET_LOCAL, error_index, (AstNode*)expr);  // Set the error variable to the result of the catch expression.
  patch_jump(compiler, success_jump);                             // Skip the catch block if the try block was successful.
}

//
// Literals
//

static void compile_lit_number(FnCompiler* compiler, AstLiteral* lit) {
  emit_constant(compiler, lit->value, (AstNode*)lit);
}
static void compile_lit_string(FnCompiler* compiler, AstLiteral* lit) {
  emit_constant(compiler, lit->value, (AstNode*)lit);
}
static void compile_lit_bool(FnCompiler* compiler, AstLiteral* lit) {
  emit_one(compiler, lit->value.as.boolean ? OP_TRUE : OP_FALSE, (AstNode*)lit);
}
static void compile_lit_nil(FnCompiler* compiler, AstLiteral* lit) {
  emit_one(compiler, OP_NIL, (AstNode*)lit);
}
static void compile_lit_tuple(FnCompiler* compiler, AstLiteral* lit) {
  compile_children(compiler, (AstNode*)lit);
  emit_two(compiler, OP_TUPLE_LITERAL, (uint16_t)lit->base.count, (AstNode*)lit);
}
static void compile_lit_seq(FnCompiler* compiler, AstLiteral* lit) {
  compile_children(compiler, (AstNode*)lit);
  emit_two(compiler, OP_SEQ_LITERAL, (uint16_t)lit->base.count, (AstNode*)lit);
}
static void compile_lit_obj(FnCompiler* compiler, AstLiteral* lit) {
  compile_children(compiler, (AstNode*)lit);
  emit_two(compiler, OP_OBJECT_LITERAL, (uint16_t)lit->base.count, (AstNode*)lit);
}

static void compile_node(FnCompiler* compiler, AstNode* node) {
  switch (node->type) {
    case NODE_BLOCK: compile_children(compiler, node); break;
    case NODE_FN: compile_function(compiler, (AstFn*)node); break;
    case NODE_ID: INTERNAL_ERROR("Should not compile " STR(NODE_ID) " directly."); break;
    case NODE_DECL: {
      AstDeclaration* decl = (AstDeclaration*)node;
      switch (decl->type) {
        case DECL_FN: compile_declare_function(compiler, decl); break;
        case DECL_FN_PARAMS: INTERNAL_ERROR("Should not compile " STR(DECL_FN_PARAMS) " directly."); break;
        case DECL_CLASS: compile_declare_class(compiler, decl); break;
        case DECL_VARIABLE: compile_declare_variable(compiler, decl); break;
        default: INTERNAL_ERROR("Unhandled declaration type."); break;
      }
      break;
    }
    case NODE_STMT: {
      AstStatement* stmt = (AstStatement*)node;
      switch (stmt->type) {
        case STMT_IMPORT: compile_statement_import(compiler, stmt); break;
        case STMT_BLOCK: compile_statement_block(compiler, stmt); break;
        case STMT_IF: compile_statement_if(compiler, stmt); break;
        case STMT_WHILE: compile_statement_while(compiler, stmt); break;
        case STMT_FOR: compile_statement_for(compiler, stmt); break;
        case STMT_RETURN: compile_statement_return(compiler, stmt); break;
        case STMT_PRINT: compile_statement_print(compiler, stmt); break;
        case STMT_EXPR: compile_statement_expr(compiler, stmt); break;
        case STMT_BREAK: compile_statement_break(compiler, stmt); break;
        case STMT_SKIP: compile_statement_skip(compiler, stmt); break;
        case STMT_THROW: compile_statement_throw(compiler, stmt); break;
        case STMT_TRY: compile_statement_try(compiler, stmt); break;
        default: INTERNAL_ERROR("Unhandled statement type."); break;
      }
      break;
    }
    case NODE_EXPR: {
      AstExpression* expr = (AstExpression*)node;
      switch (expr->type) {
        case EXPR_BINARY: compile_expr_binary(compiler, expr); break;
        case EXPR_POSTFIX: compile_expr_postfix(compiler, expr); break;
        case EXPR_UNARY: compile_expr_unary(compiler, expr); break;
        case EXPR_GROUPING: compile_expr_grouping(compiler, expr); break;
        case EXPR_LITERAL: compile_expr_literal(compiler, expr); break;
        case EXPR_VARIABLE: compile_expr_variable(compiler, expr); break;
        case EXPR_ASSIGN: compile_expr_assign(compiler, expr); break;
        case EXPR_AND: compile_expr_and(compiler, expr); break;
        case EXPR_OR: compile_expr_or(compiler, expr); break;
        case EXPR_IS: compile_expr_is(compiler, expr); break;
        case EXPR_IN: compile_expr_in(compiler, expr); break;
        case EXPR_CALL: compile_expr_call(compiler, expr); break;
        case EXPR_DOT: compile_expr_dot(compiler, expr); break;
        case EXPR_SUBS: compile_expr_subs(compiler, expr); break;
        case EXPR_SLICE: compile_expr_slice(compiler, expr); break;
        case EXPR_THIS: compile_expr_this(compiler, expr); break;
        case EXPR_BASE: compile_expr_base(compiler, expr); break;
        case EXPR_ANONYMOUS_FN: compile_expr_anon_fn(compiler, expr); break;
        case EXPR_TERNARY: compile_expr_ternary(compiler, expr); break;
        case EXPR_TRY: compile_expr_try(compiler, expr); break;
        default: INTERNAL_ERROR("Unhandled expression type."); break;
      }
      break;
    }
    case NODE_LIT: {
      AstLiteral* lit = (AstLiteral*)node;
      switch (lit->type) {
        case LIT_NUMBER: compile_lit_number(compiler, lit); break;
        case LIT_STRING: compile_lit_string(compiler, lit); break;
        case LIT_BOOL: compile_lit_bool(compiler, lit); break;
        case LIT_NIL: compile_lit_nil(compiler, lit); break;
        case LIT_TUPLE: compile_lit_tuple(compiler, lit); break;
        case LIT_SEQ: compile_lit_seq(compiler, lit); break;
        case LIT_OBJ: compile_lit_obj(compiler, lit); break;
        default: INTERNAL_ERROR("Unhandled literal type."); break;
      }
      break;
    }
    case NODE_PATTERN: INTERNAL_ERROR("Should not compile " STR(NODE_PATTERN) " directly."); break;
  }

  // Exit the scope, if we entered one - no need to do that for functions though, since their locals are popped when the function
  // returns.
  if (node->scope != NULL && node->type != NODE_FN) {
    // Since we're exiting a scope, we don't need its local variables anymore.
    // The ones who got captured by closures are still needed, and are going to need to live on the heap.
    int current_local = node->scope->local_count - 1;
    while (current_local >= 0) {  // Only scopes that have locals are handled here.
      int before = current_local;
      for (int i = 0; i < node->scope->capacity; i++) {
        SymbolEntry* entry = &node->scope->entries[i];
        if (entry->key == NULL || entry->value->type != SYMBOL_LOCAL || entry->value->index != current_local) {
          continue;
        }
        if (entry->value->is_captured) {
          emit_one(compiler, OP_CLOSE_UPVALUE, node);
        } else {
          emit_one(compiler, OP_POP, node);
        }
        current_local--;
        break;
      }
      INTERNAL_ASSERT(before != current_local,
                      "Could not find local variable. Something's wrong with indexing locals in the scope.");
    }
  }

#ifdef DEBUG_COMPILER
  if (in_global_scope(compiler)) {  // We're at the top-level
    printf("\n== End of compilation ==\n\n");
  }
#endif
}

void compile_children(FnCompiler* compiler, AstNode* node) {
  for (int i = 0; i < node->count; i++) {
    AstNode* child = node->children[i];
    if (child != NULL) {
      compile_node(compiler, child);
    }
  }
}

ObjFunction* compile(AstFn* root, ObjObject* globals_context) {
  FnCompiler compiler;
  compiler_init(&compiler, NULL, root, globals_context);
#ifdef DEBUG_COMPILER
  printf("\n\n\n === COMPILE ===\n\n");
#endif
  return compile_function(&compiler, root);
}