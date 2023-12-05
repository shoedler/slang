#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct
{
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGN,     // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * / %
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool can_assign);

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk *compiling_chunk;

static Chunk *current_chunk()
{
    return compiling_chunk;
}

static void error_at(Token *token, const char *message)
{
    if (parser.panic_mode)
    {
        return;
    }

    parser.panic_mode = true;
    fprintf(stderr, "[line %d] ERROR", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char *message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char *message)
{
    error_at(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scan_token();

        if (parser.current.type != TOKEN_ERROR)
        {
            break;
        }

        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    error_at_current(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type))
    {
        return false;
    }
    advance();
    return true;
}

static void emit_byte(uint8_t byte)
{
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2)
{
    emit_byte(byte1);
    emit_byte(byte2);
}

static uint8_t make_constant(Value value)
{
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Value value)
{
    emit_bytes(OP_CONSTANT, make_constant(value));
}

static void emit_return()
{
    emit_byte(OP_RETURN);
}

static void end_compiler()
{
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error)
    {
        disassemble_chunk(current_chunk(), "code");
    }
#endif
}

static void expression();
static void statement();
static void declaration();
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);
static uint8_t identifier_constant(Token *name);

static void binary(bool can_assign)
{
    TokenType op_type = parser.previous.type;
    ParseRule *rule = get_rule(op_type);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (op_type)
    {
    case TOKEN_NEQ:
        emit_byte(OP_NEQ);
        break;
    case TOKEN_EQ:
        emit_byte(OP_EQ);
        break;
    case TOKEN_GT:
        emit_byte(OP_GT);
        break;
    case TOKEN_GTEQ:
        emit_byte(OP_GTEQ);
        break;
    case TOKEN_LT:
        emit_byte(OP_LT);
        break;
    case TOKEN_LTEQ:
        emit_byte(OP_LTEQ);
        break;
    case TOKEN_PLUS:
        emit_byte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emit_byte(OP_SUBTRACT);
        break;
    case TOKEN_MULT:
        emit_byte(OP_MULTIPLY);
        break;
    case TOKEN_DIV:
        emit_byte(OP_DIVIDE);
        break;
    default:
        INTERNAL_ERROR("Unhandled binary operator type: %d", op_type);
        break;
    }
}

static void literal(bool can_assign)
{
    TokenType op_type = parser.previous.type;

    switch (op_type)
    {
    case TOKEN_FALSE:
        emit_byte(OP_FALSE);
        break;
    case TOKEN_NIL:
        emit_byte(OP_NIL);
        break;
    case TOKEN_TRUE:
        emit_byte(OP_TRUE);
        break;
    default:
        INTERNAL_ERROR("Unhandled literal: %d", op_type);
        return;
    }
}

static void grouping(bool can_assign)
{
    expression();
    consume(TOKEN_CPAR, "Expected ')' after expression.");
}

static void number(bool can_assign)
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void string(bool can_assign)
{
    emit_constant(OBJ_VAL(copy_string(parser.previous.start + 1,
                                      parser.previous.length - 2)));
}

static void unary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;

    // Compile the operand.
    expression();

    // Emit the operator instruction.
    switch (operator_type)
    {
    case TOKEN_NOT:
        emit_byte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emit_byte(OP_NEGATE);
        break;
    default:
        INTERNAL_ERROR("Unhandled unary operator type: %d", operator_type);
        return;
    }
}

static void named_variable(Token name, bool can_assign)
{
    uint8_t arg = identifier_constant(&name);

    if (can_assign && match(TOKEN_ASSIGN))
    {
        expression();
        emit_bytes(OP_SET_GLOBAL, arg);
    }
    else
    {
        emit_bytes(OP_GET_GLOBAL, arg);
    }
}

static void variable(bool can_assign)
{
    named_variable(parser.previous, can_assign);
}

ParseRule rules[] = {
    [TOKEN_OPAR] = {grouping, NULL, PREC_NONE},
    [TOKEN_CPAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_OBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_CBRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
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
    [TOKEN_AND] = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LAMBDA] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_LET] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_BREAK] = {NULL, NULL, PREC_NONE},
    [TOKEN_CONTINUE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL)
    {
        error("Expected expression.");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGN;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_ASSIGN))
    {
        error("Invalid assignment target.");
    }
}

static uint8_t identifier_constant(Token *name)
{
    return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

static uint8_t parse_variable(const char *error_message)
{
    consume(TOKEN_ID, error_message);
    return identifier_constant(&parser.previous);
}

static void define_variable(uint8_t global)
{
    emit_bytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule *get_rule(TokenType type)
{
    return &rules[type];
}

static void synchronize()
{
    parser.panic_mode = false;

    while (parser.current.type != TOKEN_EOF)
    {
        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FN:
        case TOKEN_LET:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;

        default:; // Do nothing.
        }

        advance();
    }
}

static void expression()
{
    parse_precedence(PREC_ASSIGN);
}

static void declaration_let()
{
    uint8_t global = parse_variable("Expected variable name.");

    if (match(TOKEN_ASSIGN))
    {
        expression();
    }
    else
    {
        emit_byte(OP_NIL);
    }

    define_variable(global);
}
static void statement_print()
{
    expression();
    emit_byte(OP_PRINT);
}

static void statement_expression()
{
    expression();
    emit_byte(OP_POP);
}

static void statement()
{
    if (match(TOKEN_PRINT))
    {
        statement_print();
    }
    else if (match(TOKEN_LET))
    {
        declaration_let();
    }
    else
    {
        statement_expression();
    }

    if (parser.panic_mode)
    {
        synchronize();
    }
}

bool compile(const char *source, Chunk *chunk)
{
    init_scanner(source);
    compiling_chunk = chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(TOKEN_EOF))
    {
        statement();
    }

    end_compiler();

    return !parser.had_error;
}