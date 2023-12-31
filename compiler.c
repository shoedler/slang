#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "memory.h"
#include "scanner.h"

// A construct holding the parser's state.
typedef struct {
  Token current;    // The current token.
  Token previous;   // The token before the current.
  bool had_error;   // True if there was an error during parsing.
  bool panic_mode;  // True if the parser requires synchronization.
} Parser;

// Precedence levels for comparison expressions.
typedef enum {
  PREC_NONE,
  PREC_ASSIGN,      // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * / %
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

// Signature for a function that parses a prefix or infix expression.
typedef void (*ParseFn)(bool can_assign);

// Precedence-dependent rule for parsing expressions.
typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

// A struct declaring a local variable.
typedef struct {
  Token name;
  int depth;
  bool is_captured;
} Local;

// A struct declaring an upvalue.
typedef struct {
  uint16_t index;
  bool is_local;
} Upvalue;

typedef enum {
  TYPE_FUNCTION,
  TYPE_CONSTRUCTOR,
  TYPE_METHOD,
  TYPE_TOPLEVEL
} FunctionType;

// A construct holding the compiler's state.
typedef struct Compiler {
  struct Compiler* enclosing;
  ObjFunction* function;
  FunctionType type;

  Local locals[2048];
  int local_count;
  Upvalue upvalues[2048];
  int scope_depth;
} Compiler;

typedef struct ClassCompiler {
  struct ClassCompiler* enclosing;
  bool has_baseclass;
} ClassCompiler;

Parser parser;
Compiler* current = NULL;
ClassCompiler* current_class = NULL;
Chunk* compiling_chunk;

// Retrieves the current chunk from the current compiler.
static Chunk* current_chunk() {
  return &current->function->chunk;
}

// Prints the given token as an error message.
// Sets the parser into panic mode to avoid cascading errors.
static void error_at(Token* token, const char* message) {
  if (parser.panic_mode) {
    return;
  }

  parser.panic_mode = true;
  fprintf(stderr, "Compile error at line %d", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.had_error = true;
}

// Prints an error message at the previous token.
static void error(const char* message) {
  error_at(&parser.previous, message);
}

// Prints an error message at the current token.
static void error_at_current(const char* message) {
  error_at(&parser.current, message);
}

// Parse the next token.
// Consumes error tokens, so that on the next call we have a valid token.
static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scan_token();

    if (parser.current.type != TOKEN_ERROR) {
      break;
    }

    error_at_current(parser.current.start);
  }
}

// Assert and cosume the current token according to the provided token type.
// If it doesn't match, print an error message.
static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  error_at_current(message);
}

// Compares the current token with the provided token type.
static bool check(TokenType type) {
  return parser.current.type == type;
}

// Accept the current token if it matches the provided token type, otherwise do
// nothing.
static bool match(TokenType type) {
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}

// Writes an opcode or operand to the current chunk.
static void emit_one(uint16_t data) {
  write_chunk(current_chunk(), data, parser.previous.line);
}

// Writes two opcodes or operands to the current chunk.
static void emit_two(uint16_t data1, uint16_t data2) {
  emit_one(data1);
  emit_one(data2);
}

// Emits a loop instruction.
// The operand is a 16-bit offset.
// It is calculated by subtracting the current chunk's count from the offset of
// the jump instruction.
static void emit_loop(int loop_start) {
  emit_one(OP_LOOP);

  int offset = current_chunk()->count - loop_start + 1;
  if (offset > MAX_JUMP) {
    error("Loop body too large.");
  }

  emit_one((uint16_t)offset);
}

// Emits a jump instruction and returns the offset of the jump instruction.
// Along with the emitted jump instruction, a 16-bit placeholder for the
// operand is emitted.
static int emit_jump(uint16_t instruction) {
  emit_one(instruction);
  emit_one(UINT16_MAX);
  return current_chunk()->count - 1;
}

// Patches a previously emitted jump instruction with the actual jump offset.
// The offset is calculated by subtracting the current chunk's count from the
// offset of the jump instruction.
static void patch_jump(int offset) {
  // -1 to adjust for the bytecode for the jump offset itself.
  int jump = current_chunk()->count - offset - 1;

  if (jump > MAX_JUMP) {
    error("Too much code to jump over.");
  }

  current_chunk()->code[offset] = (uint16_t)jump;
}

// Adds a value to the current constant pool and returns its index.
// Logs a compile error if the constant pool is full.
static uint16_t make_constant(Value value) {
  int constant = add_constant(current_chunk(), value);
  if (constant > MAX_CONSTANTS) {
    error("Too many constants in one chunk.");
    return 0;
  }

  return (uint16_t)constant;
}

// Adds value to the constant pool and emits a const instruction to load it.
static void emit_constant(Value value) {
  emit_two(OP_CONSTANT, make_constant(value));
}

// Emits a return instruction.
// If the current function is a constructor, the return value is the instance
// (the 'this' pointer). Otherwise, the return value is nil.
static void emit_return() {
  if (current->type == TYPE_CONSTRUCTOR) {
    emit_two(OP_GET_LOCAL, 0);  // Return class instance, e.g. 'this'.
  } else {
    emit_one(OP_NIL);
  }

  emit_one(OP_RETURN);
}

// Initializes a new compiler.
static void init_compiler(Compiler* compiler,
                          FunctionType type,
                          ObjString* name) {
  compiler->enclosing = current;
  compiler->function = NULL;
  compiler->type = type;
  compiler->local_count = 0;
  compiler->scope_depth = 0;
  compiler->function = new_function();
  current = compiler;

  if (type != TYPE_TOPLEVEL) {
    current->function->name = name;
  }

  Local* local = &current->locals[current->local_count++];
  local->depth = 0;
  local->is_captured = false;

  if (type != TYPE_FUNCTION) {
    local->name.start = THIS_KEYWORD;
    local->name.length = THIS_KEYWORD_LENGTH;
  } else {
    local->name.start = "";  // Not accessible.
    local->name.length = 0;
  }
}

// Ends the current compiler and returns the compiled function.
// Moves up the compiler chain, by setting the current compiler to the enclosing
static ObjFunction* end_compiler() {
  emit_return();
  ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
  if (!parser.had_error) {
    disassemble_chunk(current_chunk(), function->name != NULL
                                           ? function->name->chars
                                           : "[Toplevel]");

    if (current->enclosing == NULL) {
      printf("\n== End of compilation ==\n\n");
    }
  }
#endif
  current = current->enclosing;

  return function;
}

// Opens up a new scope.
static void begin_scope() {
  current->scope_depth++;
}

// Closes a scope. Closes over from the scope we're leaving.
static void end_scope() {
  current->scope_depth--;

  while (current->local_count > 0 &&
         current->locals[current->local_count - 1].depth >
             current->scope_depth) {
    // Since we're leaving the scope, we don't need the local variables anymore.
    // The ones who got captured by a closure are still needed, and are
    // going to need to live on the heap.
    if (current->locals[current->local_count - 1].is_captured) {
      emit_one(OP_CLOSE_UPVALUE);
    } else {
      emit_one(OP_POP);
    }
    current->local_count--;
  }
}

static void expression();
static void statement();
static void declaration();
static void block();

static uint16_t argument_list();
static ParseRule* get_rule(TokenType type);
static void parse_precedence(Precedence precedence);
static uint16_t string_constant(Token* name);
static int resolve_local(Compiler* compiler, Token* name);
static int resolve_upvalue(Compiler* compiler, Token* name);
static int add_upvalue(Compiler* compiler, uint16_t index, bool is_local);
static uint16_t parse_variable(const char* error_message);
static void define_variable(uint16_t global);
void declaration_let();

// Compiles a function call expression.
// The opening parenthesis has already been consumed and is referenced by the
// previous token.
static void call(bool can_assign) {
  uint16_t arg_count = argument_list();
  emit_two(OP_CALL, arg_count);
}

// Compiles a dot expression.
// The lhs bytecode has already been emitted. The rhs starts at the current
// token. The dot operator is in the previous token.
static void dot(bool can_assign) {
  if (!match(TOKEN_ID)) {
    consume(TOKEN_CTOR, "Expecting property name after '.'.");
  }

  uint16_t name = string_constant(&parser.previous);

  if (can_assign && match(TOKEN_ASSIGN)) {
    expression();
    emit_two(OP_SET_PROPERTY, name);
  } else if (match(TOKEN_OPAR)) {
    // Shorthand for method calls. This combines two instructions into one:
    // getting a property and calling a method.
    uint16_t arg_count = argument_list();
    emit_two(OP_INVOKE, name);
    emit_one(arg_count);
  } else {
    emit_two(OP_GET_PROPERTY, name);
  }
}

// Compiles an indexing expression.
// The opening bracket has already been consumed and is referenced by the
// previous token.
static void indexing(bool can_assign) {
  expression();
  consume(TOKEN_CBRACK, "Expecting ']' after index.");

  if (can_assign && match(TOKEN_ASSIGN)) {
    expression();  // The new value.
    emit_one(OP_SET_INDEX);
  } else {
    emit_one(OP_GET_INDEX);
  }
}

// Compiles a text literal (true, false, nil).
// The literal has already been consumed and is referenced by the previous
// token.
static void literal(bool can_assign) {
  TokenType op_type = parser.previous.type;

  switch (op_type) {
    case TOKEN_FALSE:
      emit_one(OP_FALSE);
      break;
    case TOKEN_NIL:
      emit_one(OP_NIL);
      break;
    case TOKEN_TRUE:
      emit_one(OP_TRUE);
      break;
    default:
      INTERNAL_ERROR("Unhandled literal: %d", op_type);
      return;
  }
}

// Compiles a grouping (expression in parentheses).
// The opening parenthesis has already been consumed (previous token)
static void grouping(bool can_assign) {
  expression();
  consume(TOKEN_CPAR, "Expecting ')' after expression.");
}

// Compiles a list literal.
// The opening brace has already been consumed (previous token)
static void list_literal(bool can_assign) {
  int count = 0;

  if (!check(TOKEN_CBRACK)) {
    do {
      expression();
      count++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_CBRACK, "Expecting ']' after list literal.");

  if (count <= MAX_LIST_ITEMS) {
    emit_two(OP_LIST_LITERAL, (uint16_t)count);
  } else {
    error_at_current(
        "Can't have more than MAX_LIST_ITEMS items in a list.");  // TODO
                                                                  // (enhance):
                                                                  // Interpolate
                                                                  // MAX_LIST_ITEMS
  }
}

// Compiles a number literal and emits it as a number value.
// The number has already been consumed and is referenced by the previous token.
static void number(bool can_assign) {
  double value = strtod(parser.previous.start, NULL);
  emit_constant(NUMBER_VAL(value));
}

// Compiles a string literal and emits it as a string object value.
// The string has already been consumed and is referenced by the previous token.
static void string(bool can_assign) {
  // TODO (enhance): Handle escape sequences here.
  emit_constant(OBJ_VAL(
      copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

// Compiles a unary expression and emits the corresponding instruction.
// The operator has already been consumed and is referenced by the previous
// token.
static void unary(bool can_assign) {
  TokenType operator_type = parser.previous.type;

  parse_precedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (operator_type) {
    case TOKEN_NOT:
      emit_one(OP_NOT);
      break;
    case TOKEN_MINUS:
      emit_one(OP_NEGATE);
      break;
    default:
      INTERNAL_ERROR("Unhandled unary operator type: %d", operator_type);
      return;
  }
}

// Compiles a binary expression and emits the corresponding instruction.
// The lhs bytecode has already been emitted. The rhs starts at the current
// token. The operator is in the previous token.
static void binary(bool can_assign) {
  TokenType op_type = parser.previous.type;
  ParseRule* rule = get_rule(op_type);
  parse_precedence((Precedence)(rule->precedence + 1));

  switch (op_type) {
    case TOKEN_NEQ:
      emit_one(OP_NEQ);
      break;
    case TOKEN_EQ:
      emit_one(OP_EQ);
      break;
    case TOKEN_GT:
      emit_one(OP_GT);
      break;
    case TOKEN_GTEQ:
      emit_one(OP_GTEQ);
      break;
    case TOKEN_LT:
      emit_one(OP_LT);
      break;
    case TOKEN_LTEQ:
      emit_one(OP_LTEQ);
      break;
    case TOKEN_PLUS:
      emit_one(OP_ADD);
      break;
    case TOKEN_MINUS:
      emit_one(OP_SUBTRACT);
      break;
    case TOKEN_MULT:
      emit_one(OP_MULTIPLY);
      break;
    case TOKEN_DIV:
      emit_one(OP_DIVIDE);
      break;
    default:
      INTERNAL_ERROR("Unhandled binary operator type: %d", op_type);
      break;
  }
}

// Compiles a variable. Generates bytecode to load a variable with the given
// name onto the stack.
static void named_variable(Token name, bool can_assign) {
  uint16_t get_op, set_op;
  int arg = resolve_local(current, &name);
  if (arg != -1) {
    get_op = OP_GET_LOCAL;
    set_op = OP_SET_LOCAL;
  } else if ((arg = resolve_upvalue(current, &name)) != -1) {
    get_op = OP_GET_UPVALUE;
    set_op = OP_SET_UPVALUE;
  } else {
    arg = string_constant(&name);
    get_op = OP_GET_GLOBAL;
    set_op = OP_SET_GLOBAL;
  }

  if (can_assign && match(TOKEN_ASSIGN)) {
    expression();
    emit_two(set_op, (uint16_t)arg);
  } else {
    emit_two(get_op, (uint16_t)arg);
  }
}

// Compiles a variable reference.
// The variable has already been consumed and is referenced by the previous
// token.
static void variable(bool can_assign) {
  named_variable(parser.previous, can_assign);
}

// Creates a token from the given text.
// Synthetic refers to the fact that the token is not from the source code.
static Token synthetic_token(const char* text) {
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

// Compiles a base expression.
// This is a special expression that allows access to the base class.
// It also handles the directly following dot operator - which is the only
// operator allowed after a base expression.
// The next token to be parsed is the dot operator.
static void base_(bool can_assign) {
  if (current_class == NULL) {
    error("Can't use '" BASE_CLASS_KEYWORD "' outside of a class.");
  } else if (!current_class->has_baseclass) {
    error("Can't use '" BASE_CLASS_KEYWORD "' in a class with no base class.");
  }

  consume(TOKEN_DOT, "Expecting '.' after '" BASE_CLASS_KEYWORD "'.");
  if (!match(TOKEN_ID)) {
    consume(TOKEN_CTOR, "Expecting base class method name.");
  }
  uint16_t name = string_constant(&parser.previous);

  named_variable(synthetic_token(THIS_KEYWORD), false);
  if (match(TOKEN_OPAR)) {
    // Shorthand for method calls. This combines two instructions into one:
    // getting a property and calling a method.
    uint16_t arg_count = argument_list();
    named_variable(synthetic_token(BASE_CLASS_KEYWORD), false);
    emit_two(OP_BASE_INVOKE, name);
    emit_one(arg_count);
  } else {
    named_variable(synthetic_token(BASE_CLASS_KEYWORD), false);
    emit_two(OP_GET_BASE_METHOD, name);
  }
}

// Compiles a function.
// The declaration has already been consumed. Here, we start at the function's
// parameters. Therefore is used for all supported functions:
// named functions, anonymous functions, constructors and methods.
static void function(bool can_assign, FunctionType type, ObjString* name) {
  Compiler compiler;
  init_compiler(&compiler, type, name);
  begin_scope();

  // Parameters
  if (match(TOKEN_OPAR)) {
    if (!check(TOKEN_CPAR)) {  // It's allowed to have "()" with no parameters.
      do {
        current->function->arity++;
        if (current->function->arity > MAX_FN_ARGS) {
          error_at_current(
              "Can't have more than MAX_FN_ARGS parameters.");  // TODO
                                                                // (enhance):
                                                                // Interpolate
                                                                // MAX_FN_ARGS
        }
        uint16_t constant = parse_variable("Expecting parameter name.");
        define_variable(constant);
      } while (match(TOKEN_COMMA));
    }

    if (!match(TOKEN_CPAR)) {
      error_at_current("Expecting ')' after parameters.");
    }
  }

  // Body
  if (match(TOKEN_OBRACE)) {
    block();
  } else if (match(TOKEN_LAMBDA)) {
    // TODO (optimize): end_compiler() will also emit OP_NIL and OP_RETURN,
    // which we don't are unnecessary (Same goes for non-lambda functions which
    // only have a return) We could optimize this by not emitting those
    // instructions, but that would require some changes to end_compiler() and
    // emit_return().

    // TODO (syntax): Only allowing expressions here is kinda sad. Things like
    // `let f = fn -> print 123` are not possible anymore, because the `print`
    // is a statement. Obviously, statements do not return a value - maybe we
    // could relax this restriction, forcing us - however - to answer questions
    // like "what does a for loop return?"

    expression();
    emit_one(OP_RETURN);
  } else {
    error_at_current("Expecting '{' before function body.");
  }

  ObjFunction* function =
      end_compiler();  // Also handles end of scope. (end_scope())

  emit_two(OP_CLOSURE, make_constant(OBJ_VAL(function)));
  for (int i = 0; i < function->upvalue_count; i++) {
    emit_one(compiler.upvalues[i].is_local ? 1 : 0);
    emit_one(compiler.upvalues[i].index);
  }
}

static void anonymous_function(bool can_assign) {
  function(can_assign /* does not matter */, TYPE_FUNCTION,
           copy_string("<Anon>", 7));
}

// Compiles a class method.
// Nothing has been consumed yet.
static void method() {
  consume(TOKEN_FN, "Expecting method initializer.");
  consume(TOKEN_ID, "Expecting method name.");
  uint16_t constant = string_constant(&parser.previous);
  ObjString* method_name =
      copy_string(parser.previous.start, parser.previous.length);

  function(false /* does not matter */, TYPE_METHOD, method_name);
  emit_two(OP_METHOD, constant);
}

static void constructor() {
  consume(TOKEN_CTOR, "Expecting constructor.");
  uint16_t constant = string_constant(&parser.previous);
  // TODO (optimize): Maybe preload this? A constructor is always called the
  // the same name - so we could just load it once and then reuse it.
  ObjString* ctor_name =
      copy_string(parser.previous.start, parser.previous.length);

  function(false /* does not matter */, TYPE_CONSTRUCTOR, ctor_name);
  emit_two(OP_METHOD, constant);
}

// Compiles an and expression.
// And is special in that it acts more lik a control flow construct rather than
// a binary operator. It short-circuits the evaluation of the rhs if the lhs is
// false by jumping over the rhs. The lhs bytecode has already been emitted. The
// rhs starts at the current token. The and operator is in the previous token.
static void and_(bool can_assign) {
  int end_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one(OP_POP);

  parse_precedence(PREC_AND);

  patch_jump(end_jump);
}

// Compiles an or expression.
// Or is special in that it acts more lik a control flow construct rather than
// a binary operator. It short-circuits the evaluation of the rhs if the lhs is
// true by jumping over the rhs. The lhs bytecode has already been emitted. The
static void or_(bool can_assign) {
  // TODO (optimize): We could optimize this by inverting the logic
  // (jumping over the rhs if the lhs is true) which would probably require a
  // new opcode (OP_JUMP_IF_TRUE) and then we could reuse the and_ function -
  // well, it would have to be renamed then and accept a new parameter (the type
  // of jump to emit).
  int else_jump = emit_jump(OP_JUMP_IF_FALSE);
  int end_jump = emit_jump(OP_JUMP);

  patch_jump(else_jump);
  emit_one(OP_POP);

  parse_precedence(PREC_OR);
  patch_jump(end_jump);
}

// Compiles a this expression.
// The this keyword has already been consumed and is referenced by the previous
// token.
static void this_(bool can_assign) {
  if (current_class == NULL) {
    error("Can't use 'this' outside of a class.");
    return;
  }

  variable(false);  // Can't assign to 'this'.
}

ParseRule rules[] = {
    [TOKEN_OPAR] = {grouping, call, PREC_CALL},
    [TOKEN_CPAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_CBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACK] = {list_literal, indexing, PREC_CALL},
    [TOKEN_CBRACK] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, dot, PREC_CALL},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_DIV] = {NULL, binary, PREC_FACTOR},
    [TOKEN_MULT] = {NULL, binary, PREC_FACTOR},
    [TOKEN_NOT] = {unary, NULL, PREC_NONE},
    [TOKEN_NEQ] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_ASSIGN] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQ] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GT] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GTEQ] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LT] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LTEQ] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_ID] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, and_, PREC_AND},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FN] = {anonymous_function, NULL, PREC_NONE},
    [TOKEN_LAMBDA] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, or_, PREC_OR},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_BASE] = {base_, NULL, PREC_NONE},
    [TOKEN_THIS] = {this_, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_LET] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_BREAK] = {NULL, NULL, PREC_NONE},
    [TOKEN_CONTINUE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

// Returns the rule for the given token type.
static ParseRule* get_rule(TokenType type) {
  return &rules[type];
}

// Parses any expression with a precedence higher or equal to the provided
// precedence. Handles prefix and infix expressions and determines whether the
// expression can be assigned to.
static void parse_precedence(Precedence precedence) {
  advance();
  ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
  if (prefix_rule == NULL) {
    error("Expecting expression.");
    return;
  }

  bool can_assign = precedence <= PREC_ASSIGN;
  prefix_rule(can_assign);

  while (precedence <= get_rule(parser.current.type)->precedence) {
    advance();
    ParseFn infix_rule = get_rule(parser.previous.type)->infix;
    infix_rule(can_assign);
  }

  if (can_assign && match(TOKEN_ASSIGN)) {
    error("Invalid assignment target.");
  }
}

// Adds the token's lexeme to the constant pool and returns its index.
static uint16_t string_constant(Token* name) {
  return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

// Checks whether the text content of two tokens is equal.
static bool identifiers_equal(Token* a, Token* b) {
  if (a->length != b->length) {
    return false;
  }

  return memcmp(a->start, b->start, a->length) == 0;
}

// Resolves a local variable by looking it up in the current compilers
// local variables. Returns the index of the local variable in the locals array,
// or -1 if it is not found.
static int resolve_local(Compiler* compiler, Token* name) {
  // Walk backwards through the locals to shadow outer variables with the same
  // name.
  for (int i = compiler->local_count - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifiers_equal(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

// Resolves an upvalue.
// - If it is found in the enclosing compiler's locals, it is marked as
// captured and an upvalue is added to the current compiler's upvalues array.
// - If it is not found in the enclosing compiler's locals, it is recursively
// resolved in the enclosing compilers.
//
// Returns the index of the upvalue in the current compiler's upvalues array, or
// -1 if it is not found.
static int resolve_upvalue(Compiler* compiler, Token* name) {
  if (compiler->enclosing == NULL)
    return -1;

  int local = resolve_local(compiler->enclosing, name);
  if (local != -1) {
    compiler->enclosing->locals[local].is_captured = true;
    return add_upvalue(compiler, (uint16_t)local, true);
  }

  // Recurse on outer scopes (compilers) to maybe find the variable there
  int upvalue = resolve_upvalue(compiler->enclosing, name);
  if (upvalue != -1) {
    return add_upvalue(compiler, (uint16_t)upvalue, false);
  }

  return -1;
}

// Adds a local variable to the current compiler's local variables array.
// Logs a compile error if the local variable count exceeds the maximum.
static void add_local(Token name) {
  if (current->local_count == (UINT32_MAX - 1)) {
    error("Too many local variables in this scope.");
    return;
  }

  Local* local = &current->locals[current->local_count++];
  local->name = name;
  local->depth = -1;  // Means it is not initialized, bc it's value (rvalue) is
                      // not yet evaluated.
  local->is_captured = false;
}

// Adds an upvalue to the current compiler's upvalues array.
// Checks whether the upvalue is already in the array. If so, it returns its
// index. Otherwise, it adds it to the array and returns the new index.
// Logs a compile error if the upvalue count exceeds the maximum and returns 0.
static int add_upvalue(Compiler* compiler, uint16_t index, bool is_local) {
  int upvalue_count = compiler->function->upvalue_count;

  for (int i = 0; i < upvalue_count; i++) {
    Upvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->is_local == is_local) {
      return i;
    }
  }

  if (upvalue_count == (UINT32_MAX - 1)) {
    error("Too many closure variables in function.");
    return 0;
  }

  compiler->upvalues[upvalue_count].is_local = is_local;
  compiler->upvalues[upvalue_count].index = index;
  return compiler->function->upvalue_count++;
}

// Declares a local variable in the current scope.
// If we're in the toplevel, nothing happens. This is because global variables
// are late bound.
// This function is the only place where the compiler records the existence
// of a local variable.
static void declare_local() {
  if (current->scope_depth == 0) {
    return;
  }

  Token* name = &parser.previous;

  for (int i = current->local_count - 1; i >= 0; i--) {
    Local* local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scope_depth) {
      break;
    }

    if (identifiers_equal(name, &local->name)) {
      error("Already a variable with this name in this scope.");
    }
  }
  add_local(*name);
}

// Consumes a variable name and returns its index in the constant pool.
// If it is a local variable, the function exits early with a dummy index of 0,
// because there is no need to store it in the constant pool
static uint16_t parse_variable(const char* error_message) {
  consume(TOKEN_ID, error_message);

  declare_local();
  if (current->scope_depth > 0) {
    return 0;
  }

  return string_constant(&parser.previous);
}

static void mark_initialized() {
  if (current->scope_depth == 0) {
    return;
  }
  current->locals[current->local_count - 1].depth = current->scope_depth;
}

// Emits a define-global instruction if we are not in a local scope.
// The variables index in the constant pool represents the operand of the
// instruction.
static void define_variable(uint16_t global) {
  if (current->scope_depth > 0) {
    mark_initialized();
    return;
  }

  emit_two(OP_DEFINE_GLOBAL, global);
}

// Compiles an argument list.
// The opening parenthesis has already been consumed and is referenced by the
// previous token.
static uint16_t argument_list() {
  uint16_t arg_count = 0;
  if (!check(TOKEN_CPAR)) {
    do {
      expression();
      if (arg_count == MAX_FN_ARGS) {
        error("Can't have more than MAX_FN_ARGS arguments.");
      }
      arg_count++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_CPAR, "Expecting ')' after arguments.");
  return arg_count;
}

// Synchronizes the parser after a syntax error, by skipping tokens until we
// reached a statement or declaration boundary.
static void synchronize() {
  parser.panic_mode = false;

  while (parser.current.type != TOKEN_EOF) {
    switch (parser.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FN:
      case TOKEN_LET:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;

      default:;  // Do nothing.
    }

    advance();
  }
}

// Compiles a single expression into bytecode.
static void expression() {
  parse_precedence(PREC_ASSIGN);
}

// Compiles a print statement.
// The print keyword has already been consumed at this point.
static void statement_print() {
  expression();
  emit_one(OP_PRINT);
}

// Compiles an expression statement.
// Nothing has been consumed yet, so the current token is the first token of the
// expression.
static void statement_expression() {
  expression();
  emit_one(OP_POP);
}

// Compiles an if statement.
// The if keyword has already been consumed at this point.
static void statement_if() {
  expression();

  int then_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one(OP_POP);

  statement();

  int else_jump = emit_jump(OP_JUMP);

  patch_jump(then_jump);
  emit_one(OP_POP);

  if (match(TOKEN_ELSE)) {
    statement();
  }

  patch_jump(else_jump);
}

// Compiles a return statement.
// The return keyword has already been consumed at this point.
// Handles illegal return statements (in toplevel or in a constructor).
// The return value is an expression or nil.
static void statement_return() {
  if (current->type == TYPE_TOPLEVEL) {
    error("Can't return from top-level code.");
  }

  // TODO (syntax): We don't want semicolons after return statements - but it's
  // almost impossible to get rid of them here Since there could literally come
  // anything after a return statement.

  if (match(TOKEN_SCOLON)) {
    emit_return();
  } else {
    if (current->type == TYPE_CONSTRUCTOR) {
      error("Can't return a value from a constructor.");
    }

    expression();
    consume(TOKEN_SCOLON, "Expecting ';' after return value.");
    emit_one(OP_RETURN);
  }
}

// Compiles a while statement.
// The while keyword has already been consumed at this point.
static void statement_while() {
  int loop_start = current_chunk()->count;

  expression();

  int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one(OP_POP);

  statement();
  emit_loop(loop_start);

  patch_jump(exit_jump);
  emit_one(OP_POP);
}

// Compiles a for statement.
// The for keyword has already been consumed at this point.
static void statement_for() {
  begin_scope();

  // Initializer
  if (match(TOKEN_SCOLON)) {
    // No initializer.
  } else {
    if (match(TOKEN_LET)) {
      declaration_let();
    } else {
      statement_expression();
    }
    consume(TOKEN_SCOLON, "Expecting ';' after loop initializer.");
  }

  int loop_start = current_chunk()->count;

  // Loop condition
  int exit_jump = -1;
  if (!match(TOKEN_SCOLON)) {
    expression();
    consume(TOKEN_SCOLON, "Expecting ';' after loop condition.");

    // Jump out of the loop if the condition is false.
    exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_one(OP_POP);  // Discard the result of the condition expression.
  }

  // Loop increment
  if (!match(TOKEN_SCOLON)) {
    int body_jump = emit_jump(OP_JUMP);
    int incrementStart = current_chunk()->count;
    expression();
    emit_one(OP_POP);  // Discard the result of the increment expression.
    consume(TOKEN_SCOLON, "Expecting ';' after loop increment.");

    emit_loop(loop_start);
    loop_start = incrementStart;
    patch_jump(body_jump);
  }

  // Loop body
  statement();
  emit_loop(loop_start);

  if (exit_jump != -1) {
    patch_jump(exit_jump);
    emit_one(OP_POP);  // Discard the result of the condition expression.
  }

  end_scope();
}

// Compiles a block.
static void block() {
  while (!check(TOKEN_CBRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_CBRACE, "Expecting '}' after block.");
}

// Compiles a statement.
static void statement() {
  if (match(TOKEN_PRINT)) {
    statement_print();
  } else if (match(TOKEN_IF)) {
    statement_if();
  } else if (match(TOKEN_WHILE)) {
    statement_while();
  } else if (match(TOKEN_RETURN)) {
    statement_return();
  } else if (match(TOKEN_FOR)) {
    statement_for();
  } else if (match(TOKEN_OBRACE)) {
    begin_scope();
    block();
    end_scope();
  } else {
    statement_expression();
  }
}

// Compiles a let declaration.
// The let keyword has already been consumed at this point.
static void declaration_let() {
  uint16_t global = parse_variable("Expecting variable name.");

  if (match(TOKEN_ASSIGN)) {
    expression();
  } else {
    emit_one(OP_NIL);
  }

  define_variable(global);
}

// Compiles a function declaration.
// The fn keyword has already been consumed at this point.
// Since functions are first-class, this is similar to a variable declaration.
static void declaration_function() {
  uint16_t global = parse_variable("Expecting variable name.");
  ObjString* fn_name =
      copy_string(parser.previous.start, parser.previous.length);

  mark_initialized();
  function(false /* does not matter */, TYPE_FUNCTION, fn_name);
  define_variable(global);
}

// Compiles a class declaration.
// The class keyword has already been consumed at this point.
// Also handles inheritance.
static void declaration_class() {
  consume(TOKEN_ID, "Expecting class name.");
  Token class_name = parser.previous;
  uint16_t name_constant = string_constant(&parser.previous);
  declare_local();

  emit_two(OP_CLASS, name_constant);

  // Define here, so it can be referenced in the class body.
  define_variable(name_constant);

  ClassCompiler class_compiler;
  class_compiler.has_baseclass = false;
  class_compiler.enclosing = current_class;
  current_class = &class_compiler;

  // Inherit from base class
  if (match(TOKEN_COLON)) {
    consume(TOKEN_ID, "Expecting base class name.");
    variable(false);

    if (identifiers_equal(&class_name, &parser.previous)) {
      error("A class can't inherit from itself.");
    }

    begin_scope();
    add_local(synthetic_token(
        BASE_CLASS_KEYWORD));  // TODO (optimize): Maybe use BASE_CLASS_KEYWORD
                               // as a lexeme, then synthetic_token is not
                               // needed.
    define_variable(0);

    named_variable(class_name, false);
    emit_one(OP_INHERIT);
    class_compiler.has_baseclass = true;
  }

  named_variable(class_name, false);

  // Body
  bool has_ctor = false;
  consume(TOKEN_OBRACE, "Expecting '{' before class body.");
  while (!check(TOKEN_CBRACE) && !check(TOKEN_EOF) && !parser.panic_mode) {
    if (check(TOKEN_CTOR)) {
      if (has_ctor) {
        error("Can't have more than one constructor.");
      }
      constructor();
      has_ctor = true;
    } else {
      method();
    }
  }
  consume(TOKEN_CBRACE, "Expecting '}' after class body.");
  emit_one(OP_POP);

  if (class_compiler.has_baseclass) {
    end_scope();
  }

  current_class = current_class->enclosing;
}

// Compiles a declaration.
static void declaration() {
  if (match(TOKEN_CLASS)) {
    declaration_class();
  } else if (match(TOKEN_FN)) {
    declaration_function();
  } else if (match(TOKEN_LET)) {
    declaration_let();
  } else {
    statement();
  }

  if (parser.panic_mode) {
    synchronize();
  }
}

ObjFunction* compile(const char* source) {
  init_scanner(source);
  Compiler compiler;
  init_compiler(&compiler, TYPE_TOPLEVEL, NULL);

#ifdef DEBUG_PRINT_CODE
  printf("== Begin compilation ==\n");
#endif

  parser.had_error = false;
  parser.panic_mode = false;

  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  ObjFunction* function = end_compiler();
  return parser.had_error ? NULL : function;
}

void mark_compiler_roots() {
  Compiler* compiler = current;
  while (compiler != NULL) {
    mark_obj((Obj*)compiler->function);
    compiler = compiler->enclosing;
  }
}
