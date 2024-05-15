#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "value.h"
#include "vm.h"

#define VALUE_STR_LEN 25

// 6 chars wide
#define PRINT_OFFSET(_offset) printf(ANSI_MAGENTA_STR("%05d "), _offset)

// 6 chars wide
#define PRINT_LINE(_line) printf(ANSI_BLUE_STR("%05d "), _line)
#define PRINT_SAME_LINE() printf(ANSI_BLUE_STR("    | "))

// 16 chars wide
#define PRINT_OPCODE(_name) printf(ANSI_GREEN_STR("%-16.16s"), _name)

// 7 chars wide
#define PRINT_NUMBER(_int) printf(ANSI_RED_STR(" %5d "), _int)
#define PRINT_NO_NUM() printf(ANSI_RED_STR("       "))

// VALUE_STR_LEN chars wide
#define PRINT_VALUE_STR(_str) printf("%-*.*s", VALUE_STR_LEN, VALUE_STR_LEN, _str)

void debug_print_value(Value value) {
  int written = print_value_safe(stdout, value);

  if (written < 0)
    written = 0;

  // Pad or erase to fit VALUE_STR_LEN
  if (written < VALUE_STR_LEN) {
    for (; written < VALUE_STR_LEN; written++) {
      printf(" ");
    }
  }
}

void disassemble_chunk(Chunk* chunk, const char* name) {
  printf("\n== Chunk: %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassemble_instruction(chunk, offset);
    printf("\n");
  }

  printf("== End of chunk: %s ==\n", name);
}

// Prints an instruction that has no operands.
static int simple_instruction(const char* name, int offset) {
  PRINT_OPCODE(name);
  PRINT_NO_NUM();
  PRINT_VALUE_STR("");
  return offset + 1;
}

// Prints an instruction that has one byte-sized operand.
static int byte_instruction(const char* name, Chunk* chunk, int offset) {
  uint16_t slot = chunk->code[offset + 1];
  PRINT_OPCODE(name);
  PRINT_NUMBER(slot);
  PRINT_VALUE_STR("");
  return offset + 2;
}

static int jump_instruction(const char* name, int sign, Chunk* chunk, int offset) {
  uint16_t jump = chunk->code[offset + 1];
  char jmp_str[13];
  sprintf(jmp_str, "%04d -> %04d", offset, offset + 3 + sign * jump);
  PRINT_OPCODE(name);
  PRINT_NO_NUM();
  PRINT_VALUE_STR(jmp_str);
  return offset + 3;
}

// Prints an instruction with one operand that is an index into the constant table.
static int constant_instruction(const char* name, Chunk* chunk, int offset) {
  uint16_t constant_index = chunk->code[offset + 1];
  PRINT_OPCODE(name);
  PRINT_NUMBER(constant_index);
  debug_print_value(chunk->constants.values[constant_index]);
  return offset + 2;
}

// Prints an instruction with two operands that are indices into the constant table.
static int constant_constant_instruction(const char* name, Chunk* chunk, int offset) {
  uint16_t constant_index  = chunk->code[offset + 1];
  uint16_t constant_index2 = chunk->code[offset + 2];
  PRINT_OPCODE(name);
  PRINT_NUMBER(constant_index);
  debug_print_value(chunk->constants.values[constant_index]);

  printf("\n");
  PRINT_OFFSET(offset);
  PRINT_SAME_LINE();

  PRINT_OPCODE("");
  PRINT_NUMBER(constant_index2);
  debug_print_value(chunk->constants.values[constant_index2]);
  return offset + 3;
}

static int closure_instruction(const char* name, Chunk* chunk, int offset) {
  offset++;
  uint16_t constant = chunk->code[offset++];
  PRINT_OPCODE(name);
  PRINT_NUMBER(constant);
  debug_print_value(chunk->constants.values[constant]);

  ObjFunction* function = AS_FUNCTION(chunk->constants.values[constant]);
  for (int j = 0; j < function->upvalue_count; j++) {
    int is_local = chunk->code[offset++];
    int index    = chunk->code[offset++];
    char upval_str[VALUE_STR_LEN];
    sprintf(upval_str, "%s %d", is_local ? "local" : "upvalue", index);

    printf("\n");
    PRINT_OFFSET(offset - 2);
    PRINT_SAME_LINE();
    PRINT_OPCODE("");
    PRINT_NO_NUM();
    PRINT_VALUE_STR(upval_str);
  }

  return offset;
}

static int invoke_instruction(const char* name, Chunk* chunk, int offset) {
  uint16_t constant  = chunk->code[offset + 1];
  uint16_t arg_count = chunk->code[offset + 2];
  PRINT_OPCODE(name);
  PRINT_NUMBER(constant);
  char method_str[VALUE_STR_LEN];

  sprintf(method_str, "%s, %d args", AS_STR(chunk->constants.values[constant])->chars, arg_count);
  PRINT_VALUE_STR(method_str);

  return offset + 3;
}

int disassemble_instruction(Chunk* chunk, int offset) {
  PRINT_OFFSET(offset);

  if (offset > 0 && chunk->source_views[offset].line == chunk->source_views[offset - 1].line) {
    PRINT_SAME_LINE();
  } else {
    PRINT_LINE(chunk->source_views[offset].line);
  }

  uint16_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_CONSTANT: return constant_instruction(STR(OP_CONSTANT), chunk, offset);
    case OP_NIL: return simple_instruction(STR(OP_NIL), offset);
    case OP_TRUE: return simple_instruction(STR(OP_TRUE), offset);
    case OP_FALSE: return simple_instruction(STR(OP_FALSE), offset);
    case OP_POP: return simple_instruction(STR(OP_POP), offset);
    case OP_DUPE: return byte_instruction(STR(OP_DUPE), chunk, offset);
    case OP_GET_LOCAL: return byte_instruction(STR(OP_GET_LOCAL), chunk, offset);
    case OP_SET_LOCAL: return byte_instruction(STR(OP_SET_LOCAL), chunk, offset);
    case OP_GET_GLOBAL: return constant_instruction(STR(OP_GET_GLOBAL), chunk, offset);
    case OP_DEFINE_GLOBAL: return constant_instruction(STR(OP_DEFINE_GLOBAL), chunk, offset);
    case OP_SET_GLOBAL: return constant_instruction(STR(OP_SET_GLOBAL), chunk, offset);
    case OP_GET_UPVALUE: return byte_instruction(STR(OP_GET_UPVALUE), chunk, offset);
    case OP_SET_UPVALUE: return byte_instruction(STR(OP_SET_UPVALUE), chunk, offset);
    case OP_GET_SUBSCRIPT: return simple_instruction(STR(OP_GET_SUBSCRIPT), offset);
    case OP_SET_SUBSCRIPT: return simple_instruction(STR(OP_SET_SUBSCRIPT), offset);
    case OP_GET_PROPERTY: return constant_instruction(STR(OP_GET_PROPERTY), chunk, offset);
    case OP_SET_PROPERTY: return constant_instruction(STR(OP_SET_PROPERTY), chunk, offset);
    case OP_GET_BASE_METHOD: return constant_instruction(STR(OP_GET_BASE_METHOD), chunk, offset);
    case OP_EQ: return simple_instruction(STR(OP_EQ), offset);
    case OP_NEQ: return simple_instruction(STR(OP_NEQ), offset);
    case OP_GT: return simple_instruction(STR(OP_GT), offset);
    case OP_LT: return simple_instruction(STR(OP_LT), offset);
    case OP_GTEQ: return simple_instruction(STR(OP_GTEQ), offset);
    case OP_LTEQ: return simple_instruction(STR(OP_LTEQ), offset);
    case OP_NEGATE: return simple_instruction(STR(OP_NEGATE), offset);
    case OP_PRINT: return simple_instruction(STR(OP_PRINT), offset);
    case OP_SEQ_LITERAL: return byte_instruction(STR(OP_SEQ_LITERAL), chunk, offset);
    case OP_TUPLE_LITERAL: return byte_instruction(STR(OP_TUPLE_LITERAL), chunk, offset);
    case OP_OBJECT_LITERAL: return byte_instruction(STR(OP_OBJECT_LITERAL), chunk, offset);
    case OP_JUMP: return jump_instruction(STR(OP_JUMP), 1, chunk, offset);
    case OP_JUMP_IF_FALSE: return jump_instruction(STR(OP_JUMP_IF_FALSE), 1, chunk, offset);
    case OP_TRY: return jump_instruction(STR(OP_TRY), 1, chunk, offset);
    case OP_LOOP: return jump_instruction(STR(OP_LOOP), -1, chunk, offset);
    case OP_CALL: return byte_instruction(STR(OP_CALL), chunk, offset);
    case OP_INVOKE: return invoke_instruction(STR(OP_INVOKE), chunk, offset);
    case OP_BASE_INVOKE: return invoke_instruction(STR(OP_BASE_INVOKE), chunk, offset);
    case OP_CLOSURE: return closure_instruction(STR(OP_CLOSURE), chunk, offset);
    case OP_CLOSE_UPVALUE: return simple_instruction(STR(OP_CLOSE_UPVALUE), offset);
    case OP_RETURN: return simple_instruction(STR(OP_RETURN), offset);
    case OP_CLASS: return constant_instruction(STR(OP_CLASS), chunk, offset);
    case OP_INHERIT: return simple_instruction(STR(OP_INHERIT), offset);
    case OP_FINALIZE: return simple_instruction(STR(OP_FINALIZE), offset);
    case OP_METHOD: return constant_constant_instruction(STR(OP_METHOD), chunk, offset);
    case OP_ADD: return simple_instruction(STR(OP_ADD), offset);
    case OP_SUBTRACT: return simple_instruction(STR(OP_SUBTRACT), offset);
    case OP_MULTIPLY: return simple_instruction(STR(OP_MULTIPLY), offset);
    case OP_DIVIDE: return simple_instruction(STR(OP_DIVIDE), offset);
    case OP_MODULO: return simple_instruction(STR(OP_MODULO), offset);
    case OP_NOT: return simple_instruction(STR(OP_NOT), offset);
    case OP_IMPORT: return constant_instruction(STR(OP_IMPORT), chunk, offset);
    case OP_IMPORT_FROM: return constant_constant_instruction(STR(OP_IMPORT_FROM), chunk, offset);
    case OP_THROW: return simple_instruction(STR(OP_THROW), offset);
    case OP_IS: return simple_instruction(STR(OP_IS), offset);
    case OP_IN: return simple_instruction(STR(OP_IN), offset);
    case OP_GET_SLICE: return byte_instruction(STR(OP_GET_SLICE), chunk, offset);
    default: INTERNAL_ERROR("Unhandled opcode: %d\n", instruction); return offset + 1;
  }
}

#undef PRINT_OFFSET
#undef PRINT_LINE
#undef PRINT_SAME_LINE
#undef PRINT_OPCODE
#undef PRINT_NUMBER
#undef PRINT_JUMP
#undef VALUE_STR_LEN
