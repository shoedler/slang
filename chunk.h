#ifndef chunk_h
#define chunk_h

#include "common.h"
#include "scanner.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_DUPE,
  OP_GET_LOCAL,
  OP_GET_GLOBAL,
  OP_GET_UPVALUE,
  OP_DEFINE_GLOBAL,
  OP_SET_LOCAL,
  OP_SET_GLOBAL,
  OP_SET_UPVALUE,
  OP_GET_INDEX,
  OP_SET_INDEX,
  OP_GET_PROPERTY,
  OP_SET_PROPERTY,
  OP_GET_BASE_METHOD,
  OP_EQ,
  OP_NEQ,
  OP_GT,
  OP_LT,
  OP_GTEQ,
  OP_LTEQ,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULO,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_TRY,
  OP_LOOP,
  OP_CALL,
  OP_INVOKE,
  OP_BASE_INVOKE,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_SEQ_LITERAL,
  OP_TUPLE_LITERAL,
  OP_OBJECT_LITERAL,
  OP_RETURN,
  OP_CLASS,
  OP_INHERIT,
  OP_FINALIZE,
  OP_METHOD,
  OP_IMPORT,
  OP_IMPORT_FROM,
  OP_THROW,
  OP_IS,
  OP_IN,
  OP_GET_SLICE,
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
void init_chunk(Chunk* chunk);

// Free a chunk.
void free_chunk(Chunk* chunk);

// Write data to the chunk.
// This will grow the chunk if necessary.
void write_chunk(Chunk* chunk, uint16_t data, Token error_start, Token error_end);

// Add a value to the chunk's constant pool.
// Returns the index of the value in the constant pool.
int add_constant(Chunk* chunk, Value value);

#endif
