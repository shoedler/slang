#ifndef chunk_h
#define chunk_h

#include <stdint.h>
#include "scanner.h"
#include "value.h"

// Functional macro expanding into all of the opcodes of slang. See run() in vm.c for the dispatch table and the synopsis of each
// opcode.
#define OPCODES(X)   \
  X(CONSTANT)        \
  X(NIL)             \
  X(TRUE)            \
  X(FALSE)           \
  X(POP)             \
  X(DUPE)            \
  X(GET_LOCAL)       \
  X(GET_GLOBAL)      \
  X(GET_UPVALUE)     \
  X(DEFINE_GLOBAL)   \
  X(SET_LOCAL)       \
  X(SET_GLOBAL)      \
  X(SET_UPVALUE)     \
  X(GET_SUBSCRIPT)   \
  X(SET_SUBSCRIPT)   \
  X(GET_PROPERTY)    \
  X(SET_PROPERTY)    \
  X(GET_BASE_METHOD) \
  X(GET_SLICE)       \
  X(EQ)              \
  X(NEQ)             \
  X(GT)              \
  X(LT)              \
  X(GTEQ)            \
  X(LTEQ)            \
  X(ADD)             \
  X(SUBTRACT)        \
  X(MULTIPLY)        \
  X(DIVIDE)          \
  X(MODULO)          \
  X(NOT)             \
  X(NEGATE)          \
  X(PRINT)           \
  X(JUMP)            \
  X(JUMP_IF_FALSE)   \
  X(TRY)             \
  X(LOOP)            \
  X(CALL)            \
  X(INVOKE)          \
  X(BASE_INVOKE)     \
  X(CLOSURE)         \
  X(CLOSE_UPVALUE)   \
  X(SEQ_LITERAL)     \
  X(TUPLE_LITERAL)   \
  X(OBJECT_LITERAL)  \
  X(RETURN)          \
  X(CLASS)           \
  X(INHERIT)         \
  X(FINALIZE)        \
  X(METHOD)          \
  X(IMPORT)          \
  X(IMPORT_FROM)     \
  X(THROW)           \
  X(IS)              \
  X(IN)

typedef enum {
#define OPCODE_ENUM(name) OP_##name,
  OPCODES(OPCODE_ENUM)
#undef OPCODE_ENUM
} OpCode;

typedef struct {
  const char* start;         // Pointer to first char of the line on which the error occurred.
  uint16_t error_start_ofs;  // Offset to [start]. Points to first char of the first token that caused the error.
  uint16_t error_end_ofs;    // Offset to [start]. Points to last char of the last token that caused the error.
  int line;                  // Line number on which the error starts.
} SourceView;

// Dynamic array of instructions.
// Provides a cache-friendly, constant-time lookup (and append) dense
// storage for instructions.
typedef struct {
  int count;
  int capacity;
  uint16_t* code;
  SourceView* source_views;
  ValueArray constants;
} Chunk;

// Initialize a chunk.
void chunk_init(Chunk* chunk);

// Free a chunk.
void chunk_free(Chunk* chunk);

// Write data to the chunk.
// This will grow the chunk if necessary.
void chunk_write(Chunk* chunk, uint16_t data, Token error_start, Token error_end);

// Add a value to the chunk's constant pool.
// Returns the index of the value in the constant pool.
int chunk_add_constant(Chunk* chunk, Value value);

#endif
