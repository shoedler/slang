#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chunk.h"
#include "common.h"
#include "hashtable.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
#include "stdint.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

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
  PREC_TERNARY,     // ?:
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >= is in
  PREC_TERM,        // + -
  PREC_FACTOR,      // * / %
  PREC_UNARY,       // ! -
  PREC_CALL,        // . () [] grouping with parens
  PREC_PRIMARY
} Precedence;

// Signature for a function that parses a prefix or infix expression.
typedef void (*ParseFn)(bool can_assign);

// Precedence-dependent rule for parsing expressions.
typedef struct {
  ParseFn prefix;  // Prefix, e.g. unary operators. Example: -1, where '-' is the prefix operator.
  ParseFn infix;   // Infix, e.g. binary operators. Example: 1 + 2, where '+' is the infix operator.
  Precedence precedence;
} ParseRule;

// Local variable that lives on the stack.
typedef struct {
  Token name;
  int depth;
  bool is_captured;  // True if the variable is captured by an upvalue.
  bool is_const;     // True if the variable is a constant.
} Local;

typedef struct {
  uint16_t index;
  bool is_local;
} Upvalue;

// Compiler state.
typedef struct Compiler {
  struct Compiler* enclosing;
  ObjFunction* function;
  FunctionType type;

  Local locals[MAX_LOCALS];
  int local_count;
  Upvalue upvalues[MAX_UPVALUES];
  int scope_depth;

  // Brake jumps need to be stored because we don't know the offset of the jump when we compile them.
  // That's why we store them in an array and patch them later.
  int brakes_count;
  int brakes_capacity;
  int* brake_jumps;

  int innermost_loop_start;
  int innermost_loop_scope_depth;
} Compiler;

typedef struct ClassCompiler {
  struct ClassCompiler* enclosing;
  bool has_baseclass;
} ClassCompiler;

Parser parser;
Compiler* current            = NULL;
ClassCompiler* current_class = NULL;
Chunk* compiling_chunk;

#define CONSTNESS_FN_DECLARATION false
#define CONSTNESS_FN_PARAMS false
#define CONSTNESS_CLS_DECLARATION false
#define CONSTNESS_IMPORT_VARS true
#define CONSTNESS_IMPORT_BINDINGS true
#define CONSTNESS_KW_ERROR false
#define CONSTNESS_KW_BASE true

// Keep track of globals that were declared as constants. This is completely uncoupled from the VMs globals,
// as this is only required during compilation (of a module).
ObjString* const_globals[MAX_CONST_GLOBALS];
int const_globals_count;

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
static void consume(TokenKind type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  error_at_current(message);
}

// Compares the current token with the provided token type.
static bool check(TokenKind type) {
  return parser.current.type == type;
}

// Accept the current token if it matches the provided token type, otherwise do
// nothing.
static bool match(TokenKind type) {
  if (!check(type)) {
    return false;
  }
  advance();
  return true;
}

// Checks if the current token is a statement terminator.
// Mainly used for return statements, because they can be followed an expression
static bool check_statement_return_end() {
  return parser.current.is_first_on_line ||  // If the current token is the first on the line
         check(TOKEN_CBRACE) ||  // If the current token is a closing brace, indicating the end of a block ({ ret x` })
         check(TOKEN_ELSE) ||    // If the current token is an else keyword (if a ret b` else ret c)
         check(TOKEN_EOF);       // If the current token is the end of the file
}

// Writes an opcode or operand to the current chunk.
// [error_start] is the token where the error message should start if the Vm encounters a runtime error executing this data.
// It'll span up to the previous token.
static void emit_one(uint16_t data, Token error_start) {
  write_chunk(current_chunk(), data, error_start, parser.previous /* error_end */);
}
// Writes an opcode or operand to the current chunk.
// This is a shorthand for [emit_one], indicating that the offending token is [parser.previous].
static void emit_one_here(uint16_t data) {
  emit_one(data, parser.previous);  // 'current' is actually the next one we want to consume. So we use 'previous'.
}

// Writes two opcodes or operands to the current chunk.
// [error_start] is the token where the error message should start if the Vm encounters a runtime error executing this data.
// It'll span up to the previous token.
static void emit_two(uint16_t data1, uint16_t data2, Token error_start) {
  emit_one(data1, error_start);
  emit_one(data2, error_start);
}
// Writes two opcodes or operands to the current chunk.
// This is a shorthand for [emit_two], indicating that the offending token is [parser.previous].
static void emit_two_here(uint16_t data1, uint16_t data2) {
  emit_two(data1, data2, parser.previous);  // 'current' is actually the next one we want to consume. So we use 'previous'.
}

// Emits a loop instruction.
// The operand is a 16-bit offset.
// It is calculated by subtracting the current chunk's count from the offset of
// the jump instruction.
static void emit_loop(int loop_start) {
  emit_one_here(OP_LOOP);

  int offset = current_chunk()->count - loop_start + 1;
  if (offset > MAX_JUMP) {
    error("Loop body too large.");
  }

  emit_one_here((uint16_t)offset);
}

// Emits a jump instruction and returns the offset of the jump instruction.
// Along with the emitted jump instruction, a 16-bit placeholder for the
// operand is emitted.
static int emit_jump(uint16_t instruction) {
  emit_one_here(instruction);
  emit_one_here(UINT16_MAX);
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

// Patches a previously emitted jump instruction from a break statement.
static void patch_breaks(int jump_start_offset) {
  while (current->brakes_count > 0 && current->brake_jumps[current->brakes_count - 1] > jump_start_offset) {
    patch_jump(current->brake_jumps[current->brakes_count - 1]);
    current->brakes_count--;
  }
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
static void emit_constant(Value value, Token error_start) {
  emit_two(OP_CONSTANT, make_constant(value), error_start);
}
static void emit_constant_here(Value value) {
  emit_constant(value, parser.previous);
}

// Emits a return instruction.
// If the current function is a constructor, the return value is the instance
// (the 'this' pointer). Otherwise, the return value is nil.
static void emit_return() {
  if (current->type == TYPE_CONSTRUCTOR) {
    emit_two_here(OP_GET_LOCAL, 0);  // Return class instance, e.g. 'this'.
  } else {
    emit_one_here(OP_NIL);
  }

  emit_one_here(OP_RETURN);
}

// Initializes a new compiler.
static void init_compiler(Compiler* compiler, FunctionType type) {
  compiler->enclosing = current;
  current             = compiler;

  compiler->function                   = NULL;
  compiler->type                       = type;
  compiler->local_count                = 0;
  compiler->scope_depth                = 0;
  compiler->function                   = new_function();
  compiler->function->globals_context  = vm.module;
  compiler->innermost_loop_start       = -1;
  compiler->innermost_loop_scope_depth = -1;

  compiler->brakes_capacity = 0;
  compiler->brakes_count    = 0;
  compiler->brake_jumps     = NULL;

  // Determine the name of the function via its type.
  switch (type) {
    case TYPE_MODULE: {
      // We use the modules name as the functions name (same reference, no copy). Meaning that the modules
      // toplevel function has the same name as the module. This is useful for debugging and for stacktraces -
      // it let's us easily determine if a frames function is a toplevel function or not.
      Value module_name;
      if (hashtable_get_by_string(&vm.module->fields, vm.special_prop_names[SPECIAL_PROP_MODULE_NAME], &module_name)) {
        current->function->name = AS_STR(module_name);
        break;
      }
      INTERNAL_ERROR("Module name not found in the fields of the active module (module." STR(SP_PROP_MODULE_NAME) ").");
      current->function->name = copy_string("(Unnamed module)", STR_LEN("(Unnamed module)"));
      break;
    }
    case TYPE_ANONYMOUS_FUNCTION: current->function->name = copy_string("(anon)", STR_LEN("(anon)")); break;
    case TYPE_CONSTRUCTOR: current->function->name = vm.special_method_names[SPECIAL_METHOD_CTOR]; break;
    default: current->function->name = copy_string(parser.previous.start, parser.previous.length); break;
  }

  Local* local       = &current->locals[current->local_count++];
  local->depth       = 0;
  local->is_captured = false;

  // Sometimes we need to "inject" the 'this' keyword, depending on the type of function we're compiling.
  if (type == TYPE_CONSTRUCTOR || type == TYPE_METHOD) {
    local->name.start  = KEYWORD_THIS;
    local->name.length = KEYWORD_THIS_LEN;
  } else {
    local->name.start  = "";  // Not accessible.
    local->name.length = 0;
  }
}

// Frees a compiler
static void free_compiler(Compiler* compiler) {
  FREE_ARRAY(int, compiler->brake_jumps, compiler->brakes_capacity);
}

// Ends the current compiler and returns the compiled function.
// Moves up the compiler chain, by setting the current compiler to the enclosing
static ObjFunction* end_compiler() {
  emit_return();
  ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
  if (!parser.had_error) {
    disassemble_chunk(current_chunk(), function->name != NULL ? function->name->chars : "[Toplevel]");

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

  while (current->local_count > 0 && current->locals[current->local_count - 1].depth > current->scope_depth) {
    // Since we're leaving the scope, we don't need the local variables anymore.
    // The ones who got captured by a closure are still needed, and are
    // going to need to live on the heap.
    if (current->locals[current->local_count - 1].is_captured) {
      emit_one_here(OP_CLOSE_UPVALUE);
    } else {
      emit_one_here(OP_POP);
    }
    current->local_count--;
  }
}

static void expression();
static void statement();
static void declaration();
static void block();

static uint16_t argument_list();
static void declaration_let();
static void try_(bool can_assign);

static ParseRule* get_rule(TokenKind type);
static void parse_precedence(Precedence precedence);
static uint16_t identifier_constant(Token* name);
static int resolve_local(Compiler* compiler, Token* name, bool* is_const);
static int resolve_upvalue(Compiler* compiler, Token* name, bool* is_const);
static int add_upvalue(Compiler* compiler, uint16_t index, bool is_local);
static uint16_t parse_variable(const char* error_message, bool is_const);
static void define_variable(uint16_t global, bool is_const);

// Checks if the current token is a compound assigment token.
static bool match_compound_assignment() {
  return match(TOKEN_PLUS_ASSIGN) || match(TOKEN_MINUS_ASSIGN) || match(TOKEN_MULT_ASSIGN) || match(TOKEN_DIV_ASSIGN) ||
         match(TOKEN_MOD_ASSIGN);
}

// Checks if the current token is an inc/dec token.
static bool match_inc_dec() {
  return match(TOKEN_PLUS_PLUS) || match(TOKEN_MINUS_MINUS);
}

// Compiles a compound assignment expression. The lhs bytecode has already been
// emitted. The rhs starts at the current token. The compound assignment
// operator is in the previous token.
static void compound_assignment() {
  TokenKind op_type = parser.previous.type;
  expression();

  switch (op_type) {
    case TOKEN_PLUS_ASSIGN: emit_one_here(OP_ADD); break;
    case TOKEN_MINUS_ASSIGN: emit_one_here(OP_SUBTRACT); break;
    case TOKEN_MULT_ASSIGN: emit_one_here(OP_MULTIPLY); break;
    case TOKEN_DIV_ASSIGN: emit_one_here(OP_DIVIDE); break;
    case TOKEN_MOD_ASSIGN: emit_one_here(OP_MODULO); break;
    default: INTERNAL_ERROR("Unhandled compound assignment operator type: %d", op_type); break;
  }
}

// Compiles an inc/dec expression. The lhs bytecode has already been emitted.
// The inc/dec operator is in the previous token.
static void inc_dec() {
  TokenKind op_type = parser.previous.type;
  emit_constant_here(int_value(1));

  switch (op_type) {
    case TOKEN_PLUS_PLUS: emit_one_here(OP_ADD); break;
    case TOKEN_MINUS_MINUS: emit_one_here(OP_SUBTRACT); break;
    default: INTERNAL_ERROR("Unhandled inc/dec operator type: %d", op_type); break;
  }
}

// Compiles a function call expression.
// The opening parenthesis has already been consumed and is referenced by the
// previous token.
static void call(bool can_assign) {
  UNUSED(can_assign);
  Token error_start  = parser.previous;
  uint16_t arg_count = argument_list();
  emit_two(OP_CALL, arg_count, error_start);
}

// Compiles a dot expression.
// The lhs bytecode has already been emitted. The rhs starts at the current
// token. The dot operator is in the previous token.
static void dot(bool can_assign) {
  if (!match(TOKEN_ID)) {
    consume(TOKEN_CTOR, "Expecting property name after '.'.");
  }

  Token error_start = parser.previous;
  uint16_t name     = identifier_constant(&parser.previous);

  if (can_assign && match(TOKEN_ASSIGN)) {
    expression();
    emit_two(OP_SET_PROPERTY, name, error_start);
  } else if (can_assign && match_inc_dec()) {
    emit_two(OP_DUPE, 0, error_start);  // Duplicate the object.
    emit_two(OP_GET_PROPERTY, name, error_start);
    inc_dec();
    emit_two(OP_SET_PROPERTY, name, error_start);
  } else if (can_assign && match_compound_assignment()) {
    emit_two(OP_DUPE, 0, error_start);  // Duplicate the object.
    emit_two(OP_GET_PROPERTY, name, error_start);
    compound_assignment();
    emit_two(OP_SET_PROPERTY, name, error_start);
  } else if (match(TOKEN_OPAR)) {
    // Shorthand for method calls. This combines two instructions into one:
    // getting a property and calling a method.
    uint16_t arg_count = argument_list();
    emit_two(OP_INVOKE, name, error_start);
    emit_one(arg_count, error_start);
  } else {
    emit_two(OP_GET_PROPERTY, name, error_start);
  }
}

// Compiles a subscripting expression.
// The opening bracket has already been consumed and is referenced by the
// previous token.
static void subscripting(bool can_assign) {
  bool slice_started = false;

  // Start at the '['
  Token error_start = parser.previous;

  // Since the first number in a slice is optional, we need to check if the subscripting starts with a '..'
  if (match(TOKEN_DOTDOT)) {
    slice_started = true;
    emit_constant_here(int_value(0));  // Start index.
  }

  // Either the index or the slice end.
  if (slice_started && check(TOKEN_CBRACK)) {
    emit_one(OP_NIL, error_start);  // Signals that we want to slice until the end.
  } else {
    expression();
  }

  // Handle slices
  if (slice_started || match(TOKEN_DOTDOT)) {
    // If the slice already started, we already have emitted the end aswell.
    if (!slice_started) {
      // The end of the slice is optional too.
      if (check(TOKEN_CBRACK)) {
        emit_one(OP_NIL, error_start);  // Signals that we want to slice until the end.
      } else {
        expression();
      }
    }

    consume(TOKEN_CBRACK, "Expecting ']' after slice.");
    emit_one(OP_GET_SLICE, error_start);

    if (match(TOKEN_ASSIGN)) {
      error("Slices can't be assigned to.");
    }
    return;
  }

  consume(TOKEN_CBRACK, "Expecting ']' after index.");

  if (can_assign && match(TOKEN_ASSIGN)) {
    expression();  // The new value.
    emit_one(OP_SET_SUBSCRIPT, error_start);
  } else if (can_assign && match_inc_dec()) {
    emit_two(OP_DUPE, 1, error_start);  // Duplicate the indexee: [indexee][index] -> [indexee][index][indexee]
    emit_two(OP_DUPE, 1, error_start);  // Duplicate the index:   [indexee][index][indexee] -> [indexee][index][indexee][index]
    emit_one(OP_GET_SUBSCRIPT, error_start);
    inc_dec();
    emit_one(OP_SET_SUBSCRIPT, error_start);
  } else if (can_assign && match_compound_assignment()) {
    emit_two(OP_DUPE, 1, error_start);  // Duplicate the indexee: [indexee][index] -> [indexee][index][indexee]
    emit_two(OP_DUPE, 1, error_start);  // Duplicate the index:  [indexee][index][indexee] -> [indexee][index][indexee][index]
    emit_one(OP_GET_SUBSCRIPT, error_start);
    compound_assignment();
    emit_one(OP_SET_SUBSCRIPT, error_start);
  } else {
    emit_one(OP_GET_SUBSCRIPT, error_start);
  }
}

// Compiles a text literal (true, false, nil).
// The literal has already been consumed and is referenced by the previous
// token.
static void literal(bool can_assign) {
  UNUSED(can_assign);
  TokenKind op_type = parser.previous.type;

  switch (op_type) {
    case TOKEN_FALSE: emit_one_here(OP_FALSE); break;
    case TOKEN_NIL: emit_one_here(OP_NIL); break;
    case TOKEN_TRUE: emit_one_here(OP_TRUE); break;
    default: INTERNAL_ERROR("Unhandled literal: %d", op_type); return;
  }
}

// Compiles a tuple literal.
// The opening parenthesis, aswell as the first element (optional) and the comma (previous token) have been consumed.
// Maybe the first element (expression) has already been consumed too, bc it could also just be a grouping experssion (expression
// in parentheses). Making a tuple of one element is allowed, but it's also a little stupid.
static void tuple_literal(bool can_assign, int already_emitted_items, Token error_start) {
  UNUSED(can_assign);
  int count = 0;

  if (!check(TOKEN_CPAR)) {
    do {
      if (check(TOKEN_CPAR)) {
        break;  // Allow trailing comma.
      }
      expression();
      count++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_CPAR, "Expecting ')' after " STR(TYPENAME_TUPLE) " literal. Or maybe you are missing a ','?");

  if (count <= MAX_TUPLE_LITERAL_ITEMS) {
    emit_two(OP_TUPLE_LITERAL, (uint16_t)(count + already_emitted_items), error_start);
  } else {
    // Still use 'error_at_current' because the message would be very long if we'd start at 'error_start'.
    error_at_current("Can't have more than " STR(MAX_TUPLE_LITERAL_ITEMS) " items in a " STR(TYPENAME_TUPLE) ".");
  }
}

// Compiles an empty tuple literal.
// The opening parenthesis has already been consumed (previous token). Also, the comma has already been consumed.
static void empty_tuple_literal(bool can_assign, Token error_start) {
  UNUSED(can_assign);

  consume(TOKEN_CPAR, "Expecting ')' after " STR(TYPENAME_TUPLE) " literal. ");
  emit_two(OP_TUPLE_LITERAL, (uint16_t)0, error_start);
}

// Compiles a grouping (expression in parentheses).
// The opening parenthesis has already been consumed (previous token)
static void grouping(bool can_assign) {
  UNUSED(can_assign);

  // Start at the '('
  Token error_start = parser.previous;

  if (match(TOKEN_COMMA)) {
    empty_tuple_literal(can_assign, error_start);
    return;
  }

  expression();

  if (match(TOKEN_COMMA)) {
    tuple_literal(can_assign, 1, error_start);
    return;
  }

  consume(TOKEN_CPAR, "Expecting ')' after expression.");
}

// Compiles a seq literal.
// The opening brace has already been consumed (previous token)
static void seq_literal(bool can_assign) {
  UNUSED(can_assign);
  int count = 0;

  // Start at the '['
  Token error_start = parser.previous;

  if (!check(TOKEN_CBRACK)) {
    do {
      if (check(TOKEN_CBRACK)) {
        break;  // Allow trailing comma.
      }
      expression();
      count++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_CBRACK, "Expecting ']' after " STR(TYPENAME_SEQ) " literal. Or maybe you are missing a ','?");

  if (count <= MAX_SEQ_LITERAL_ITEMS) {
    emit_two(OP_SEQ_LITERAL, (uint16_t)count, error_start);
  } else {
    error_at_current("Can't have more than " STR(MAX_SEQ_LITERAL_ITEMS) " items in a " STR(TYPENAME_SEQ) ".");
  }
}

// Compiles an object literal.
// The opening brace has already been consumed (previous token)
static void object_literal(bool can_assign) {
  UNUSED(can_assign);
  int count = 0;

  // Start at the '{'
  Token error_start = parser.previous;

  if (!check(TOKEN_CBRACE)) {
    do {
      if (check(TOKEN_CBRACE)) {
        break;  // Allow trailing comma.
      }
      expression();
      consume(TOKEN_COLON, "Expecting ':' after key.");
      expression();
      count++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_CBRACE, "Expecting '}' after " STR(TYPENAME_OBJ) " literal. Or maybe you are missing a ','?");

  if (count <= MAX_OBJECT_LITERAL_ITEMS) {
    emit_two(OP_OBJECT_LITERAL, (uint16_t)count, error_start);
  } else {
    error_at_current("Can't have more than " STR(MAX_OBJECT_LITERAL_ITEMS) " items in a " STR(TYPENAME_OBJ) ".");
  }
}

// Compiles a number literal and emits it as a number value.
// The number has already been consumed and is referenced by the previous token.
static void number(bool can_assign) {
  UNUSED(can_assign);
  if (parser.previous.start[0] == '0') {
    char kind = parser.previous.start[1];
    // See if it's a hexadecimal, binary, or octal number.
    if ((kind == 'x' || kind == 'X')) {
      long long value = strtoll(parser.previous.start + 2, NULL, 16);
      emit_constant_here(int_value(value));
      return;
    }

    if ((kind == 'b' || kind == 'B')) {
      long long value = strtoll(parser.previous.start + 2, NULL, 2);
      emit_constant_here(int_value(value));
      return;
    }

    if ((kind == 'o' || kind == 'O')) {
      long long value = strtoll(parser.previous.start + 2, NULL, 8);
      emit_constant_here(int_value(value));
      return;
    }
  }

  // Check if the number is a float.
  bool is_float = false;
  for (int i = 0; i < parser.previous.length; i++) {
    if (parser.previous.start[i] == '.') {
      is_float = true;
      break;
    }
  }

  if (is_float) {
    double value = strtod(parser.previous.start, NULL);
    emit_constant_here(float_value(value));
  } else {
    long long int value = strtoll(parser.previous.start, NULL, 10);
    emit_constant_here(int_value(value));
  }
}

// Compiles a string literal and emits it as a string object value.
// The string has already been consumed and is referenced by the previous token.
static void string(bool can_assign) {
  UNUSED(can_assign);
  // Build the string using a flexible array
  size_t str_capacity = 8;
  size_t str_length   = 0;
  char* str_bytes     = (char*)malloc(str_capacity);

#define PUSH_CHAR(c)                                          \
  do {                                                        \
    if (str_capacity < str_length + 1) {                      \
      size_t old   = str_capacity;                            \
      str_capacity = GROW_CAPACITY(old);                      \
      str_bytes    = (char*)realloc(str_bytes, str_capacity); \
    }                                                         \
    str_bytes[str_length++] = c;                              \
  } while (0)

  do {
    const char* cur = parser.previous.start + 1;                           // Skip the opening quote.
    const char* end = parser.previous.start + parser.previous.length - 1;  // Skip the closing quote.

    while (cur < end) {
      if (*cur == '\\') {
        if (cur + 1 > end) {
          error_at_current("Unterminated escape sequence.");
          return;
        }
        switch (cur[1]) {
          case 'f': PUSH_CHAR('\f'); break;  // Form feed
          case 'r': PUSH_CHAR('\r'); break;  // Carriage return
          case 'n': PUSH_CHAR('\n'); break;  // Newline
          case 't': PUSH_CHAR('\t'); break;  // Tab
          case 'v': PUSH_CHAR('\v'); break;  // Vertical tab
          case 'b': PUSH_CHAR('\b'); break;  // Backspace
          case '\\': PUSH_CHAR('\\'); break;
          case '\"': PUSH_CHAR('\"'); break;
          case '\'': PUSH_CHAR('\''); break;
          default: error_at_current("Invalid escape sequence."); return;
        }
        cur += 2;
      } else {
        PUSH_CHAR(*cur);
        cur++;
      }
    }
  } while (match(TOKEN_STRING));

  // Emit the string constant.
  emit_constant_here(str_value(copy_string(str_bytes, (int)str_length)));

  // Cleanup
  free(str_bytes);
#undef PUSH_CHAR
}

// Compiles a unary expression and emits the corresponding instruction.
// The operator has already been consumed and is referenced by the previous
// token.
static void unary(bool can_assign) {
  UNUSED(can_assign);
  TokenKind operator_type = parser.previous.type;
  Token error_start       = parser.previous;

  parse_precedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (operator_type) {
    case TOKEN_NOT: emit_one(OP_NOT, error_start); break;
    case TOKEN_MINUS: emit_one(OP_NEGATE, error_start); break;
    default: INTERNAL_ERROR("Unhandled unary operator type: %d", operator_type); return;
  }
}

// Compiles a binary expression and emits the corresponding instruction.
// The lhs bytecode has already been emitted. The rhs starts at the current
// token. The operator is in the previous token.
static void binary(bool can_assign) {
  UNUSED(can_assign);
  TokenKind op_type = parser.previous.type;
  Token error_start = parser.previous;

  ParseRule* rule = get_rule(op_type);
  parse_precedence((Precedence)(rule->precedence + 1));

  switch (op_type) {
    case TOKEN_NEQ: emit_one(OP_NEQ, error_start); break;
    case TOKEN_EQ: emit_one(OP_EQ, error_start); break;
    case TOKEN_GT: emit_one(OP_GT, error_start); break;
    case TOKEN_GTEQ: emit_one(OP_GTEQ, error_start); break;
    case TOKEN_LT: emit_one(OP_LT, error_start); break;
    case TOKEN_LTEQ: emit_one(OP_LTEQ, error_start); break;
    case TOKEN_PLUS: emit_one(OP_ADD, error_start); break;
    case TOKEN_MINUS: emit_one(OP_SUBTRACT, error_start); break;
    case TOKEN_MULT: emit_one(OP_MULTIPLY, error_start); break;
    case TOKEN_DIV: emit_one(OP_DIVIDE, error_start); break;
    case TOKEN_MOD: emit_one(OP_MODULO, error_start); break;
    default: INTERNAL_ERROR("Unhandled binary operator type: %d", op_type); break;
  }
}

// Compiles a variable. Generates bytecode to load a variable with the given name onto the stack.
// [name] can be a synthetic token, but [error_start] must be the actual token from the source code.
static void named_variable(Token name, bool can_assign, Token error_start) {
  uint16_t get_op;
  uint16_t set_op;
  bool is_var_const = false;
  int arg           = resolve_local(current, &name, &is_var_const);

  if (arg != -1) {
    get_op = OP_GET_LOCAL;
    set_op = OP_SET_LOCAL;
  } else if ((arg = resolve_upvalue(current, &name, &is_var_const)) != -1) {
    get_op = OP_GET_UPVALUE;
    set_op = OP_SET_UPVALUE;
  } else {
    arg = identifier_constant(&name);
    // Get the name we added back from the constant pool and check if it matches (pointer comparison) any of the const globals in
    // this compilation.
    ObjString* name = (ObjString*)current_chunk()->constants.values[arg].as.obj;
    for (int i = 0; i < const_globals_count; i++) {
      if (const_globals[i] == name) {
        is_var_const = true;
        break;
      }
    }

    get_op = OP_GET_GLOBAL;
    set_op = OP_SET_GLOBAL;
  }

  if (can_assign && match(TOKEN_ASSIGN)) {
    if (is_var_const) {
      error_at(&error_start, "Can't reassign a constant.");
    } else {
      expression();
      emit_two(set_op, (uint16_t)arg, error_start);
    }
  } else if (can_assign && match_inc_dec()) {
    if (is_var_const) {
      error_at(&error_start, "Can't reassign a constant.");
    } else {
      emit_two(get_op, (uint16_t)arg, error_start);
      inc_dec();
      emit_two(set_op, (uint16_t)arg, error_start);
    }
  } else if (can_assign && match_compound_assignment()) {
    if (is_var_const) {
      error_at(&error_start, "Can't reassign a constant.");
    } else {
      emit_two(get_op, (uint16_t)arg, error_start);
      compound_assignment();
      emit_two(set_op, (uint16_t)arg, error_start);
    }
  } else {
    emit_two(get_op, (uint16_t)arg, error_start);
  }
}

// Compiles a variable reference.
// The variable has already been consumed and is referenced by the previous
// token.
static void variable(bool can_assign) {
  named_variable(parser.previous, can_assign, parser.previous);
}

// Creates a token from the given text.
// Synthetic refers to the fact that the token is not from the source code.
static Token synthetic_token(const char* text) {
  Token token;
  token.start  = text;
  token.length = (int)strlen(text);
  return token;
}

// Compiles a base expression.
// This is a special expression that allows access to the base class. It also handles the directly following dot operator - which
// is the only operator allowed after a base expression. The base keyword has already been consumed and is referenced by the
// previous token.
static void base_(bool can_assign) {
  UNUSED(can_assign);

  Token error_start = parser.previous;

  if (current_class == NULL) {
    error("Can't use '" KEYWORD_BASE "' outside of a class.");
  } else if (!current_class->has_baseclass) {
    error("Can't use '" KEYWORD_BASE "' in a class with no base class.");
  } else if (current->type == TYPE_METHOD_STATIC) {
    error("Can't use '" KEYWORD_BASE "' in a static method.");
  }

  Token method_name;
  if (check(TOKEN_OPAR)) {
    method_name = synthetic_token(STR(SP_METHOD_CTOR));
  } else {
    consume(TOKEN_DOT, "Expecting '.' after '" KEYWORD_BASE "'.");
    consume(TOKEN_ID, "Expecting base class method name.");
    method_name = parser.previous;
    error_start = parser.previous;  // Use the method name as the error token instead.
  }

  uint16_t name = identifier_constant(&method_name);

  named_variable(synthetic_token(KEYWORD_THIS), false, error_start);
  if (match(TOKEN_OPAR)) {
    // Shorthand for method calls. This combines two instructions into one:
    // getting a property and calling a method.
    uint16_t arg_count = argument_list();

    named_variable(synthetic_token(KEYWORD_BASE), false, error_start);
    emit_two(OP_BASE_INVOKE, name, error_start);
    emit_one(arg_count, error_start);
  } else {
    named_variable(synthetic_token(KEYWORD_BASE), false, error_start);
    emit_two(OP_GET_BASE_METHOD, name, error_start);
  }
}

// Compiles a function.
// The declaration has already been consumed. Here, we start at the function's
// parameters. Therefore is used for all supported functions:
// named functions, anonymous functions, constructors and methods.
static void function(bool can_assign, FunctionType type) {
  UNUSED(can_assign);
  Compiler compiler;
  init_compiler(&compiler, type);
  begin_scope();

  // Parameters
  if (match(TOKEN_OPAR)) {
    if (!check(TOKEN_CPAR)) {  // It's allowed to have "()" with no parameters.
      do {
        current->function->arity++;
        if (current->function->arity > MAX_FN_ARGS) {
          error_at_current("Can't have more than " STR(MAX_FN_ARGS) " parameters.");
        }

        uint16_t constant = parse_variable("Expecting parameter name.", CONSTNESS_FN_PARAMS);
        define_variable(constant, CONSTNESS_FN_PARAMS);
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
    if (type == TYPE_CONSTRUCTOR) {
      error_at_current("Constructors can't be lambda functions.");
    }
    // TODO (optimize): end_compiler() will also emit OP_NIL and OP_RETURN, which are unnecessary (Same goes
    // for non-lambda functions which only have a return) We could optimize this by not emitting those
    // instructions, but that would require some changes to end_compiler() and emit_return().

    expression();
    emit_one_here(OP_RETURN);
  } else {
    error_at_current("Expecting '{' or '->' before function body.");
  }

  ObjFunction* function = end_compiler();  // Also handles end of scope. (end_scope())

  emit_two_here(OP_CLOSURE, make_constant(fn_value((Obj*)function)));
  for (int i = 0; i < function->upvalue_count; i++) {
    emit_one_here(compiler.upvalues[i].is_local ? 1 : 0);
    emit_one_here(compiler.upvalues[i].index);
  }

  free_compiler(&compiler);
}

static void anonymous_function(bool can_assign) {
  function(can_assign /* does not matter */, TYPE_ANONYMOUS_FUNCTION);
}

// Compiles a class method.
// Nothing has been consumed yet.
static void method() {
  FunctionType type = match(TOKEN_STATIC) ? TYPE_METHOD_STATIC : TYPE_METHOD;
  consume(TOKEN_FN, "Expecting method initializer.");
  consume(TOKEN_ID, "Expecting method name.");
  Token error_start = parser.previous;

  uint16_t constant = identifier_constant(&parser.previous);

  function(false /* does not matter */, type);
  emit_two(OP_METHOD, constant, error_start);
  emit_one((uint16_t)type, error_start);
}

static void constructor() {
  consume(TOKEN_CTOR, "Expecting constructor.");
  Token error_start = parser.previous;

  Token ctor        = synthetic_token(STR(SP_METHOD_CTOR));
  uint16_t constant = identifier_constant(&ctor);

  function(false /* does not matter */, TYPE_CONSTRUCTOR);
  emit_two(OP_METHOD, constant, error_start);
  emit_one((uint16_t)TYPE_CONSTRUCTOR, error_start);
}

// Compiles an and expression. And is special in that it acts more lik a control flow construct rather than a
// binary operator. It short-circuits the evaluation of the rhs if the lhs is false by jumping over the rhs.
// The lhs bytecode has already been emitted. The rhs starts at the current token. The and operator is in the
// previous token.
static void and_(bool can_assign) {
  UNUSED(can_assign);
  int end_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one_here(OP_POP);

  parse_precedence(PREC_AND);

  patch_jump(end_jump);
}

// Compiles an or expression.
// Or is special in that it acts more lik a control flow construct rather than
// a binary operator. It short-circuits the evaluation of the rhs if the lhs is
// true by jumping over the rhs. The lhs bytecode has already been emitted.
static void or_(bool can_assign) {
  UNUSED(can_assign);
  // TODO (optimize): We could optimize this by inverting the logic (jumping over the rhs if the lhs is true)
  // which would probably require a new opcode (OP_JUMP_IF_TRUE) and then we could reuse the and_ function -
  // well, it would have to be renamed then and accept a new parameter (the type of jump to emit).
  int else_jump = emit_jump(OP_JUMP_IF_FALSE);
  int end_jump  = emit_jump(OP_JUMP);

  patch_jump(else_jump);
  emit_one_here(OP_POP);

  parse_precedence(PREC_OR);
  patch_jump(end_jump);
}

// Compiles a ternary expression. The condition bytecode has already been emitted. The question mark has been
// consumed and is referenced by the previous token. The true branch starts at the current token.
static void ternary(bool can_assign) {
  UNUSED(can_assign);
  int else_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one_here(OP_POP);  // Discard the condition.

  parse_precedence(PREC_TERNARY);

  consume(TOKEN_COLON, "Expecting ':' after true branch.");
  int end_jump = emit_jump(OP_JUMP);

  patch_jump(else_jump);
  emit_one_here(OP_POP);  // Discard the true branch.

  parse_precedence(PREC_TERNARY);
  patch_jump(end_jump);
}

// Compiles an 'is' expression. The 'is' keyword has already been consumed and is referenced by the previous
// token.
static void is_(bool can_assign) {
  UNUSED(can_assign);
  expression();
  emit_one_here(OP_IS);
}

// Compiles an 'in' expression. The 'in' keyword has already been consumed and is referenced by the previous token.
static void in_(bool can_assign) {
  UNUSED(can_assign);
  expression();
  emit_one_here(OP_IN);
}

// Compiles a 'this' expression.
// The 'this' keyword has already been consumed and is referenced by the previous
// token.
static void this_(bool can_assign) {
  UNUSED(can_assign);
  if (current_class == NULL) {
    error("Can't use '" KEYWORD_THIS "' outside of a class.");
    return;
  }

  if (current->type == TYPE_METHOD_STATIC) {
    error("Can't use '" KEYWORD_THIS "' in a static method.");
    return;
  }

  variable(false);  // Can't assign to 'this'.
}

ParseRule rules[] = {
    [TOKEN_OPAR]    = {grouping, call, PREC_CALL},
    [TOKEN_CPAR]    = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACE]  = {object_literal, NULL, PREC_NONE},
    [TOKEN_CBRACE]  = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACK]  = {seq_literal, subscripting, PREC_CALL},
    [TOKEN_CBRACK]  = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]   = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT]     = {NULL, dot, PREC_CALL},
    [TOKEN_MINUS]   = {unary, binary, PREC_TERM},
    [TOKEN_PLUS]    = {NULL, binary, PREC_TERM},
    [TOKEN_DIV]     = {NULL, binary, PREC_FACTOR},
    [TOKEN_MULT]    = {NULL, binary, PREC_FACTOR},
    [TOKEN_MOD]     = {NULL, binary, PREC_FACTOR},
    [TOKEN_NOT]     = {unary, NULL, PREC_NONE},
    [TOKEN_TERNARY] = {NULL, ternary, PREC_TERNARY},
    [TOKEN_NEQ]     = {NULL, binary, PREC_EQUALITY},
    [TOKEN_ASSIGN]  = {NULL, NULL, PREC_NONE},
    [TOKEN_EQ]      = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GT]      = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GTEQ]    = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LT]      = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LTEQ]    = {NULL, binary, PREC_COMPARISON},
    [TOKEN_ID]      = {variable, NULL, PREC_NONE},
    [TOKEN_STRING]  = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER]  = {number, NULL, PREC_NONE},
    [TOKEN_AND]     = {NULL, and_, PREC_AND},
    [TOKEN_CLASS]   = {NULL, NULL, PREC_NONE},
    [TOKEN_STATIC]  = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]    = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]   = {literal, NULL, PREC_NONE},
    [TOKEN_FOR]     = {NULL, NULL, PREC_NONE},
    [TOKEN_FN]      = {anonymous_function, NULL, PREC_NONE},
    [TOKEN_LAMBDA]  = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]      = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]     = {literal, NULL, PREC_NONE},
    [TOKEN_OR]      = {NULL, or_, PREC_OR},
    [TOKEN_PRINT]   = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]  = {NULL, NULL, PREC_NONE},
    [TOKEN_BASE]    = {base_, NULL, PREC_NONE},
    [TOKEN_TRY]     = {try_, NULL, PREC_NONE},
    [TOKEN_CATCH]   = {NULL, NULL, PREC_NONE},
    [TOKEN_THROW]   = {NULL, NULL, PREC_NONE},
    [TOKEN_IS]      = {NULL, is_, PREC_COMPARISON},
    [TOKEN_IN]      = {NULL, in_, PREC_COMPARISON},
    [TOKEN_THIS]    = {this_, NULL, PREC_NONE},
    [TOKEN_TRUE]    = {literal, NULL, PREC_NONE},
    [TOKEN_LET]     = {NULL, NULL, PREC_NONE},
    [TOKEN_CONST]   = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]   = {NULL, NULL, PREC_NONE},
    [TOKEN_BREAK]   = {NULL, NULL, PREC_NONE},
    [TOKEN_SKIP]    = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR]   = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]     = {NULL, NULL, PREC_NONE},
};

// Returns the rule for the given token type.
static ParseRule* get_rule(TokenKind type) {
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

  // Exit early if we're now at a new line. That's it. Nothing more is needed to make newlines meaningful.
  // This allows us to write code like this:
  //   let a = [1,2,3]
  //   [4].map(fn (x) -> log(x))
  //  Which would've been impossible before. That would've been interpreted as let a = [1,2,3][4].map...
  if (parser.current.is_first_on_line) {
    // Except if it's a dot, because chaining method calls on newlines is a common pattern and still allowed.
    if (parser.current.type != TOKEN_DOT) {
      return;
    }
  }

  while (precedence <= get_rule(parser.current.type)->precedence) {
    advance();
    ParseFn infix_rule = get_rule(parser.previous.type)->infix;
    infix_rule(can_assign);
  }

  if (can_assign && match(TOKEN_ASSIGN)) {
    error("Invalid assignment target.");
  } else if (can_assign && match_inc_dec()) {
    error("Invalid inc/dec target.");
  } else if (can_assign && match_compound_assignment()) {
    error("Invalid compound assignment target.");
  }
}

// Adds the token's lexeme to the constant pool and returns its index.
static uint16_t identifier_constant(Token* name) {
  return make_constant(str_value(copy_string(name->start, name->length)));
}

// Checks whether the text content of two tokens is equal.
static bool identifiers_equal(Token* id_a, Token* id_b) {
  if (id_a->length != id_b->length) {
    return false;
  }

  return memcmp(id_a->start, id_b->start, id_a->length) == 0;
}

// Resolves a local variable by looking it up in the current compilers
// local variables. Returns the index of the local variable in the locals array,
// or -1 if it is not found.
// [is_const] is set to true if the resolved local is a constant.
static int resolve_local(Compiler* compiler, Token* name, bool* is_const) {
  // Walk backwards through the locals to shadow outer variables with the same
  // name.
  for (int i = compiler->local_count - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifiers_equal(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      *is_const = local->is_const;
      return i;
    }
  }

  return -1;
}

// Resolves an upvalue.
// - If it is found in the enclosing compiler's locals, it is marked as
// captured and an upvalue is added to the current compiler's upvalues array.
// - If it is not found in the enclosing compiler's locals, it is recursively
// resolved in the outer enclosing compilers.
//
// Returns the index of the upvalue in the current compiler's upvalues array, or
// -1 if it is not found.
// [is_const] is set to true if the resolved upvalue is a constant.
static int resolve_upvalue(Compiler* compiler, Token* name, bool* is_const) {
  if (compiler->enclosing == NULL) {
    return -1;
  }

  bool is_local_const = false;
  int local           = resolve_local(compiler->enclosing, name, &is_local_const);
  if (local != -1) {
    compiler->enclosing->locals[local].is_captured = true;
    *is_const                                      = is_local_const;
    return add_upvalue(compiler, (uint16_t)local, true);
  }

  // Recurse on upper scopes (compilers) to maybe find the variable there
  bool is_upvalue_const = false;
  int upvalue           = resolve_upvalue(compiler->enclosing, name, &is_upvalue_const);
  if (upvalue != -1) {
    *is_const = is_upvalue_const;
    return add_upvalue(compiler, (uint16_t)upvalue, false);
  }

  return -1;
}

// Adds a local variable to the current compiler's local variables array.
static void add_local(Token name, bool is_const) {
  if (current->local_count == MAX_LOCALS) {
    error("Can't have more than " STR(MAX_LOCALS) " local variables in one scope.");
    return;
  }

  Local* local    = &current->locals[current->local_count++];
  local->name     = name;
  local->is_const = is_const;
  local->depth    = -1;  // Means it is not initialized, bc it's value (rvalue) is
                         // not yet evaluated.
  local->is_captured = false;
}

// Adds a global variable - which is just a constant in the constant pool of the outermost compiler. Returns the index of the
// constant in the constant pool. This was mainly added to support constant globals and to easily track creation of globals
static void add_const_global(uint16_t global) {
  INTERNAL_ASSERT(current->scope_depth == 0, "Can't add a global in a local scope.");

  if (const_globals_count == MAX_CONST_GLOBALS) {
    error("Can't have more than " STR(MAX_CONST_GLOBALS) " global constants per compilation.");
    return;
  }

  // Get the name of the global we added back from the constant pool
  ObjString* global_name               = (ObjString*)current_chunk()->constants.values[global].as.obj;
  const_globals[const_globals_count++] = global_name;  // ... and add its pointer to the const_globals array
}

// Adds an upvalue to the current compiler's upvalues array. Checks whether the upvalue is already in the array. If so, it returns
// its index. Otherwise, it adds it to the array and returns the new index. Logs a compile error if the upvalue count exceeds the
// maximum and returns 0.
static int add_upvalue(Compiler* compiler, uint16_t index, bool is_local) {
  int upvalue_count = compiler->function->upvalue_count;

  for (int i = 0; i < upvalue_count; i++) {
    Upvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->is_local == is_local) {
      return i;
    }
  }

  if (upvalue_count == MAX_UPVALUES) {
    error("Can't have more than " STR(MAX_UPVALUES) " closure variables in one function.");
    return 0;
  }

  compiler->upvalues[upvalue_count].is_local = is_local;
  compiler->upvalues[upvalue_count].index    = index;
  return compiler->function->upvalue_count++;
}

// Declares a local variable in the current scope from the provided token. If we're in the toplevel, nothing happens. This is
// because global variables are late bound. This function is the only place where the compiler records the existence of a local
// variable.
static void declare_local(Token* name, bool is_const) {
  if (current->scope_depth == 0) {
    return;
  }

  for (int i = current->local_count - 1; i >= 0; i--) {
    Local* local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scope_depth) {
      break;
    }

    if (identifiers_equal(name, &local->name)) {
      error("Already a variable with this name in this scope.");
    }
  }
  add_local(*name, is_const);
}

// Consumes a variable name and returns its index in the constant pool. If it is a local variable, the function exits early with a
// dummy index of 0, because there is no need to store it in the constant pool
static uint16_t parse_variable(const char* error_message, bool is_const) {
  consume(TOKEN_ID, error_message);

  declare_local(&parser.previous, is_const);
  if (current->scope_depth > 0) {
    return 0;
  }

  uint16_t global = identifier_constant(&parser.previous);
  if (is_const) {
    add_const_global(global);
  }
  return global;
}

// Marks a local variable as initialized by setting its depth to the current scope depth.
// Locals are first created with a depth of -1, which means they are not yet initialized.
// If we're in global scope, nothing happens.
static void mark_initialized() {
  if (current->scope_depth == 0) {
    return;
  }
  current->locals[current->local_count - 1].depth = current->scope_depth;
}

// Emits a define-global instruction (if we are not in a local scope) for the most recent local.
// The variables' index in the constant pool represents the operand of the
// instruction.
static void define_variable(uint16_t global, bool is_const) {
  if (current->scope_depth > 0) {
    mark_initialized();
    return;
  }

  if (is_const) {
    add_const_global(global);
  }

  emit_two_here(OP_DEFINE_GLOBAL, global);
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
        error("Can't have more than " STR(MAX_FN_ARGS) " arguments.");
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
      case TOKEN_RETURN: return;

      default:;  // Do nothing.
    }

    advance();
  }
}

// Compiles a single expression into bytecode. An expression leaves a value on the stack.
static void expression() {
  parse_precedence(PREC_ASSIGN);
}

// Different types of destructuring assignments.
typedef enum {
  DESTRUCTURE_SEQ,
  DESTRUCTURE_OBJ,
  DESTRUCTURE_TUPLE,
} DestructureType;

// Compiles a destructuring statement.
// The opening token has already been consumed at this point. Currently either let, const or import.
static void destructuring(DestructureType type, bool rhs_is_import, bool is_const) {
  typedef struct {
    Token name;       // The variable name, used for object destructuring.
    uint16_t global;  // The global index of the variable - used if we're in global scope.
    uint16_t local;   // The local index (stack slot index) of the variable - used if we're in local scope.
    bool is_rest;     // Denotes whether the variable is a rest parameter.
    int index;        // The index of the variable in the destructuring pattern.
  } DestructuringVariable;

  TokenKind closing;
  switch (type) {
    case DESTRUCTURE_SEQ: closing = TOKEN_CBRACK; break;
    case DESTRUCTURE_OBJ: closing = TOKEN_CBRACE; break;
    case DESTRUCTURE_TUPLE: closing = TOKEN_CPAR; break;
    default: INTERNAL_ERROR("Unhandled destructuring type."); break;
  }

  DestructuringVariable variables[MAX_DESTRUCTURING_VARS];
  int current_index = 0;
  bool has_rest     = false;
  bool local_scope  = current->scope_depth > 0;

  Token error_start = parser.previous;

  // Parse the left-hand side of the assignment, e.g. the variables.
  while (!check(closing) && !check(TOKEN_EOF)) {
    if (has_rest) {
      error("Rest parameter must be last in destructuring assignment.");
    }
    has_rest = match(TOKEN_DOTDOTDOT);
    if (has_rest && type == DESTRUCTURE_OBJ) {
      error("Rest parameter is not allowed in " STR(TYPENAME_OBJ) " destructuring assignment.");
    }

    // A variable is parsed just like in a normal let declaration. But, because the rhs is not yet compiled,
    // we need to first declare the variables. In a local scope, we emit OP_NIL to define the variable, in global scope,
    // that doesn't matter yet (will be declared later with OP_DEFINE_GLOBAL) We don't want to mark the variables as
    // initialized yet, so we don't use define_variable(global) here. We just emit OP_NIL (if in local scope) to create a
    // placeholder value on the stack for the local.
    uint16_t global;
    if (has_rest) {
      global = parse_variable("Expecting identifier after ellipsis in destructuring assignment.", is_const);
    } else {
      global = parse_variable("Expecting identifier in destructuring assignment.", is_const);
    }
    variables[current_index].is_rest = has_rest;
    variables[current_index].global  = global;
    variables[current_index].local   = (uint16_t)(current->local_count - 1);  // It's just the one on the top.
    variables[current_index].index   = current_index;
    variables[current_index].name    = parser.previous;  // Used for object destructuring.

    // Emit a placeholder value for the variable if we're in a local scope, because locals live on the stack.
    if (local_scope) {
      emit_one(OP_NIL, error_start);  // Define the variable.
    }

    if (++current_index > MAX_DESTRUCTURING_VARS) {
      error("Can't have more than " STR(MAX_DESTRUCTURING_VARS) " variables in destructuring assignment.");
    }

    if (has_rest) {
      break;
    }

    if (!match(TOKEN_COMMA)) {
      break;
    }
  }

  if (!match(closing)) {
    has_rest ? error("Rest parameter must be last in destructuring assignment.") : error("Unterminated destructuring pattern.");
  }

  // Parse the right-hand side of the assignment.
  // Either an expression or an import statement.
  if (rhs_is_import) {
    consume(TOKEN_FROM, "Expecting 'from' after destructuring assignment import.");
    consume(TOKEN_STRING, "Expecting file name.");

    // Since we don't have a module name, we calculate the absolute path of the module here and use it as the module name.
    // TODO: When loading a module from cache, we should compare their absolute paths instead of the module name to avoid
    // loading the same module multiple times.
    ObjString* file_path = copy_string(parser.previous.start + 1, parser.previous.length - 2);  // +1 and -2 to strip the quotes

    Value cwd;
    if (!hashtable_get_by_string(&vm.module->fields, vm.special_prop_names[SPECIAL_PROP_FILE_PATH], &cwd)) {
      INTERNAL_ERROR("Module file path not found in the fields of the active module (module." STR(SP_PROP_FILE_PATH) ").");
      cwd = str_value(copy_string("?", 1));
    }

    char* absolute_path                  = resolve_module_path((ObjString*)cwd.as.obj, NULL, file_path);
    uint16_t absolute_file_path_constant = make_constant(str_value(copy_string(absolute_path, strlen(absolute_path))));
    free(absolute_path);  // since we copied to make sure it's allocated on our managed heap, we need to free it.

    emit_two(OP_IMPORT_FROM, absolute_file_path_constant, parser.previous);  // Use the path as the module name.
    emit_one(absolute_file_path_constant, parser.previous);
  } else {
    consume(TOKEN_ASSIGN, "Expecting '=' in destructuring assignment.");  // Even Js does this.
    expression();
  }

  // Emit code to destructure the rhs value
  for (int i = 0; i < current_index; i++) {
    DestructuringVariable* var = &variables[i];

    emit_two(OP_DUPE, 0, error_start);  // Duplicate the rhs value: [RhsVal] -> [RhsVal][RhsVal]

    // Emit code to get the index from the rhs. For objs, we use the variable name as the operand for OP_GET_SUBSCRIPT. For
    // seqs and tuples, we use the variables index.
    Value payload = type == DESTRUCTURE_OBJ ? str_value(copy_string(var->name.start, var->name.length)) : int_value(var->index);
    if (var->is_rest) {
      emit_constant(payload, error_start);  // [RhsVal][RhsVal] -> [RhsVal][RhsVal][current_index]
      emit_one(OP_NIL, error_start);        //                  -> [RhsVal][RhsVal][current_index][nil]
      emit_one(OP_GET_SLICE, error_start);  //                  -> [RhsVal][slice]
    } else {
      emit_constant(payload, error_start);      // [RhsVal][RhsVal] -> [RhsVal][RhsVal][i]
      emit_one(OP_GET_SUBSCRIPT, error_start);  //                  -> [RhsVal][value]
    }

    // Define the variable. We need to emit different opcodes depending on whether we're in a local or global scope.
    if (local_scope) {
      // Mark the variable as initialized. We cannot use mark_initialized() here, because that would just mark the most
      // recent local. So we need to do it manually.
      current->locals[var->local].depth = current->scope_depth;
      emit_two(OP_SET_LOCAL, var->local, error_start);
      emit_one(OP_POP, error_start);  // Discard the value.
    } else {
      emit_two(OP_DEFINE_GLOBAL, var->global, error_start);  // Also pops the value.
    }
  }

  emit_one(OP_POP, error_start);  // Discard the value.
}

// Compiles a print statement.
// The print keyword has already been consumed at this point.
static void statement_print() {
  Token error_start = parser.previous;
  expression();
  emit_one(OP_PRINT, error_start);
}

// Compiles an expression statement.
// Nothing has been consumed yet, so the current token is the first token of the
// expression.
static void statement_expression() {
  Token error_start = parser.current;
  expression();
  emit_one(OP_POP, error_start);
}

// Compiles an if statement.
// The if keyword has already been consumed at this point.
static void statement_if() {
  expression();

  int then_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one_here(OP_POP);  // Discard the result of the condition expression.

  statement();

  int else_jump = emit_jump(OP_JUMP);

  patch_jump(then_jump);
  emit_one_here(OP_POP);

  if (match(TOKEN_ELSE)) {
    statement();
  }

  patch_jump(else_jump);
}

// Compiles a throw statement.
// The throw keyword has already been consumed at this point.
static void statement_throw() {
  Token error_start = parser.previous;
  expression();
  emit_one(OP_THROW, error_start);
}

// Compiles a try statement.
// The try keyword has already been consumed at this point.
static void statement_try(bool is_try_expression) {
  // Make sure we are in a local scope, so that the error variable is not defined globally.
  begin_scope();

  int try_jump = emit_jump(OP_TRY);

  // Contextual variable: Inject the "error" variable into the local scope.
  Token error = synthetic_token(KEYWORD_ERROR);
  add_local(error, CONSTNESS_KW_ERROR);
  define_variable(0, CONSTNESS_KW_ERROR);  // We're never in global scope here, so we can pass 0 as the global index.

  Token error_start = parser.current;

  // Compile the try block / expression.
  if (is_try_expression) {
    expression();  // Leaves the result on the stack.

    // Set the error variable to the expression result.
    bool _is_const = false;  // Discard, we don't care if it's a constant.
    int arg        = resolve_local(current, &error, &_is_const);
    emit_two(OP_SET_LOCAL, (uint16_t)arg, error_start);
  } else {
    statement();
  }

  // If the try block was successful, skip the catch block.
  int success_jump = emit_jump(OP_JUMP);
  patch_jump(try_jump);

  // Compile optional catch block / expression.
  if (is_try_expression) {
    //  If there is no else expression, we push nil as the "else" value.
    match(TOKEN_ELSE) ? expression() : emit_one_here(OP_NIL);

    // Set the error variable to the else expression result or the nil value.
    bool _is_const = false;  // Discard, we don't care if it's a constant.
    int arg        = resolve_local(current, &error, &_is_const);
    emit_two(OP_SET_LOCAL, (uint16_t)arg, error_start);
  } else {
    if (match(TOKEN_CATCH)) {
      statement();
    }
  }

  patch_jump(success_jump);

  end_scope();  // This will pop the error handler variable from the stack.
}

// Compiles a try expression. This is a special form of the try "statement" that returns a value.
// The try keyword has already been consumed at this point.
static void try_(bool can_assign) {
  UNUSED(can_assign);
  statement_try(true /* is_try_expression */);
}

// Compiles a return statement.
// The return keyword has already been consumed at this point.
// Handles illegal return statements in a constructor.
// The return value is an expression or nil.
static void statement_return() {
  Token error_start = parser.previous;

  if (check_statement_return_end()) {
    emit_return();
  } else {
    if (current->type == TYPE_CONSTRUCTOR) {
      error("Can't return a value from a constructor.");
    }

    expression();
    if (!check_statement_return_end()) {
      error_at_current("Expecting newline, '}' or some other statement after return value.");
    }
    emit_one(OP_RETURN, error_start);
  }
}

// Compiles a while statement.
// The while keyword has already been consumed at this point.
static void statement_while() {
  // Save the loop state for continue(skip)/break statements, which might occur in the loop body.
  int surrounding_loop_start          = current->innermost_loop_start;
  int surrounding_loop_scope_depth    = current->innermost_loop_scope_depth;
  current->innermost_loop_start       = current_chunk()->count;
  current->innermost_loop_scope_depth = current->scope_depth;

  // Loop condition
  expression();

  int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_one_here(OP_POP);

  // Loop body
  statement();

  emit_loop(current->innermost_loop_start);

  patch_jump(exit_jump);
  emit_one_here(OP_POP);

  patch_breaks(current->innermost_loop_start);

  // Restore the surrounding loop state.
  current->innermost_loop_start       = surrounding_loop_start;
  current->innermost_loop_scope_depth = surrounding_loop_scope_depth;
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

  // Save the loop state for continue(skip)/break statements, which might occur in the loop body.
  int surrounding_loop_start          = current->innermost_loop_start;
  int surrounding_loop_scope_depth    = current->innermost_loop_scope_depth;
  current->innermost_loop_start       = current_chunk()->count;
  current->innermost_loop_scope_depth = current->scope_depth;

  // Loop condition
  int exit_jump = -1;
  if (!match(TOKEN_SCOLON)) {
    expression();
    consume(TOKEN_SCOLON, "Expecting ';' after loop condition.");

    // Jump out of the loop if the condition is false.
    exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_one_here(OP_POP);  // Discard the result of the condition expression.
  }

  // Loop increment
  if (!match(TOKEN_SCOLON)) {
    int body_jump       = emit_jump(OP_JUMP);
    int increment_start = current_chunk()->count;
    expression();
    emit_one_here(OP_POP);  // Discard the result of the increment expression.
    consume(TOKEN_SCOLON, "Expecting ';' after loop increment.");

    emit_loop(current->innermost_loop_start);
    current->innermost_loop_start = increment_start;
    patch_jump(body_jump);
  }

  // Loop body
  statement();
  emit_loop(current->innermost_loop_start);

  if (exit_jump != -1) {
    patch_jump(exit_jump);
    emit_one_here(OP_POP);  // Discard the result of the condition expression.
  }

  patch_breaks(current->innermost_loop_start);

  // Restore the surrounding loop state.
  current->innermost_loop_start       = surrounding_loop_start;
  current->innermost_loop_scope_depth = surrounding_loop_scope_depth;

  end_scope();
}

// Compiles a skip statement.
// The skip keyword has already been consumed at this point.
static void statement_skip() {
  if (current->innermost_loop_start == -1) {
    error("Can't skip outside of a loop.");
  }

  // Discard any locals created in the loop body.
  for (int i = current->local_count - 1; i >= 0 && current->locals[i].depth > current->innermost_loop_scope_depth; i--) {
    emit_one_here(OP_POP);
  }

  // Jump back to the start of the loop.
  emit_loop(current->innermost_loop_start);
}

// Compiles a break statement.
// The break keyword has already been consumed at this point.
static void statement_break() {
  if (current->innermost_loop_start == -1) {
    error("Can't break outside of a loop.");
  }

  // Grow the array if necessary.
  if (SHOULD_GROW(current->brakes_count + 1, current->brakes_capacity)) {
    int old_capacity         = current->brakes_capacity;
    current->brakes_capacity = GROW_CAPACITY(old_capacity);
    current->brake_jumps     = RESIZE_ARRAY(int, current->brake_jumps, current->brakes_count, current->brakes_capacity);
  }

  // Discard any locals created in the loop body.
  for (int i = current->local_count - 1; i >= 0 && current->locals[i].depth > current->innermost_loop_scope_depth; i--) {
    emit_one_here(OP_POP);
  }

  // Jump to the end of the loop.
  current->brake_jumps[current->brakes_count++] = emit_jump(OP_JUMP);
}

// Compiles an import statement.
// The import keyword has already been consumed at this point.
static void statement_import() {
  if (check(TOKEN_ID)) {
    Token error_start = parser.previous;
    consume(TOKEN_ID, "Expecting module name.");

    uint16_t name_constant = identifier_constant(&parser.previous);
    declare_local(&parser.previous, CONSTNESS_IMPORT_VARS);  // Module names are always constants.

    if (match(TOKEN_FROM)) {
      consume(TOKEN_STRING, "Expecting file name.");
      uint16_t file_path_constant =
          make_constant(str_value(copy_string(parser.previous.start + 1,
                                              parser.previous.length - 2)));  // +1 and -2 to strip the quotes
      emit_two(OP_IMPORT_FROM, name_constant, error_start);
      emit_one(file_path_constant, error_start);
    } else {
      emit_two(OP_IMPORT, name_constant, error_start);
    }

    define_variable(name_constant, CONSTNESS_IMPORT_VARS);
  } else if (match(TOKEN_OBRACE)) {
    destructuring(DESTRUCTURE_OBJ, true /* rhs_is_import */, CONSTNESS_IMPORT_BINDINGS);  // Import bindings are always constants.
  } else {
    error_at(&parser.current, "Expecting module name or destructuring assignment after import.");
  }
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
  } else if (match(TOKEN_THROW)) {
    statement_throw();
  } else if (match(TOKEN_TRY)) {
    statement_try(false /* isn't try_expression */);
  } else if (match(TOKEN_WHILE)) {
    statement_while();
  } else if (match(TOKEN_RETURN)) {
    statement_return();
  } else if (match(TOKEN_FOR)) {
    statement_for();
  } else if (match(TOKEN_IMPORT)) {
    statement_import();
  } else if (match(TOKEN_SKIP)) {
    statement_skip();
  } else if (match(TOKEN_BREAK)) {
    statement_break();
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
  bool constness = false;
  if (match(TOKEN_OBRACK)) {
    destructuring(DESTRUCTURE_SEQ, false /* rhs_is_import */, constness);
  } else if (match(TOKEN_OBRACE)) {
    destructuring(DESTRUCTURE_OBJ, false /* rhs_is_import */, constness);
  } else if (match(TOKEN_OPAR)) {
    destructuring(DESTRUCTURE_TUPLE, false /* rhs_is_import */, constness);
  } else {
    uint16_t global = parse_variable("Expecting variable name.", constness);

    if (match(TOKEN_ASSIGN)) {
      expression();
    } else {
      emit_one_here(OP_NIL);
    }

    define_variable(global, constness);
  }
}

static void declaration_const() {
  bool constness = true;
  if (match(TOKEN_OBRACK)) {
    destructuring(DESTRUCTURE_SEQ, false /* rhs_is_import */, constness);
  } else if (match(TOKEN_OBRACE)) {
    destructuring(DESTRUCTURE_OBJ, false /* rhs_is_import */, constness);
  } else if (match(TOKEN_OPAR)) {
    destructuring(DESTRUCTURE_TUPLE, false /* rhs_is_import */, constness);
  } else {
    uint16_t global = parse_variable("Expecting constant name.", constness);

    if (match(TOKEN_ASSIGN)) {
      expression();
    } else {
      error("Expecting '=' after constant name.");
    }

    define_variable(global, constness);
  }
}

// Compiles a function declaration.
// The fn keyword has already been consumed at this point.
// Since functions are first-class, this is similar to a variable declaration.
static void declaration_function() {
  uint16_t global = parse_variable("Expecting function name.", CONSTNESS_FN_DECLARATION);  // Function name

  mark_initialized();
  function(false /* does not matter */, TYPE_FUNCTION);
  define_variable(global, CONSTNESS_FN_DECLARATION);
}

// Compiles a class declaration.
// The class keyword has already been consumed at this point.
// Also handles inheritance.
static void declaration_class() {
  Token error_start = parser.previous;

  consume(TOKEN_ID, "Expecting class name.");
  Token class_name       = parser.previous;
  uint16_t name_constant = identifier_constant(&parser.previous);
  declare_local(&parser.previous, CONSTNESS_CLS_DECLARATION);

  emit_two(OP_CLASS, name_constant, error_start);

  // Define here, so it can be referenced in the class body.
  define_variable(name_constant, CONSTNESS_CLS_DECLARATION);

  ClassCompiler class_compiler;
  class_compiler.has_baseclass = false;
  class_compiler.enclosing     = current_class;
  current_class                = &class_compiler;

  // Inherit from base class
  if (match(TOKEN_COLON)) {
    consume(TOKEN_ID, "Expecting base class name.");
    variable(false);

    if (identifiers_equal(&class_name, &parser.previous)) {
      error("A class can't inherit from itself.");
    }

    begin_scope();

    // Contextual variable: Inject the "base" variable into the scope.
    add_local(synthetic_token(KEYWORD_BASE), CONSTNESS_KW_BASE);
    define_variable(0 /* ignore, we're not in global scope */,
                    CONSTNESS_KW_BASE);  // We're never in global scope here, so we can pass 0 as the global index.

    named_variable(class_name, false, parser.previous);
    emit_one(OP_INHERIT, parser.previous);
    class_compiler.has_baseclass = true;
  }

  named_variable(class_name, false, error_start);

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
  emit_one(OP_FINALIZE, error_start);  // Finalize the class & pop it from the stack.

  if (class_compiler.has_baseclass) {
    end_scope();
  }

  current_class = current_class->enclosing;
}

// Compiles a declaration.
static void declaration() {
  if (match(TOKEN_CLASS)) {  // TODO (optimization): Reorder these to match the frequency of use.
    declaration_class();
  } else if (match(TOKEN_FN)) {
    declaration_function();
  } else if (match(TOKEN_CONST)) {
    declaration_const();
  } else if (match(TOKEN_LET)) {
    declaration_let();
  } else {
    statement();
  }

  if (parser.panic_mode) {
    synchronize();
  }
}

ObjFunction* compile_module(const char* source) {
  init_scanner(source);
  Compiler compiler;
  const_globals_count = 0;

  init_compiler(&compiler, TYPE_MODULE);

#ifdef DEBUG_PRINT_CODE
  printf("== Begin compilation ==\n");
#endif

  parser.had_error  = false;
  parser.panic_mode = false;

  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  ObjFunction* function = end_compiler();
  free_compiler(&compiler);
  return parser.had_error ? NULL : function;
}

void mark_compiler_roots() {
  Compiler* compiler = current;
  while (compiler != NULL) {
    mark_obj((Obj*)compiler->function);
    compiler = compiler->enclosing;
  }
}

#undef CONSTNESS_FN_DECLARATION
#undef CONSTNESS_FN_PARAMS
#undef CONSTNESS_CLS_DECLARATION
#undef CONSTNESS_IMPORT_VARS
#undef CONSTNESS_IMPORT_BINDINGS
#undef CONSTNESS_KW_ERROR
#undef CONSTNESS_KW_BASE
