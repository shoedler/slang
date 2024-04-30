#include "chunk.h"
#include "memory.h"
#include "vm.h"

void init_chunk(Chunk* chunk) {
  chunk->count    = 0;
  chunk->capacity = 0;
  chunk->code     = NULL;
  chunk->lines    = NULL;
  chunk->start    = NULL;
  chunk->end      = NULL;
  init_value_array(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint16_t data, Token start, Token end, int line) {
  if (SHOULD_GROW(chunk->count + 1, chunk->capacity)) {
    int old_capacity = chunk->capacity;
    chunk->capacity  = GROW_CAPACITY(old_capacity);
    chunk->code      = RESIZE_ARRAY(uint16_t, chunk->code, old_capacity, chunk->capacity);
    chunk->lines     = RESIZE_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    chunk->start     = RESIZE_ARRAY(Token, chunk->start, old_capacity, chunk->capacity);
    chunk->end       = RESIZE_ARRAY(Token, chunk->end, old_capacity, chunk->capacity);
  }

  chunk->code[chunk->count]  = data;
  chunk->lines[chunk->count] = line;
  chunk->start[chunk->count] = start;
  chunk->end[chunk->count]   = end;
  chunk->count++;
}

void free_chunk(Chunk* chunk) {
  FREE_ARRAY(uint16_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  FREE_ARRAY(Token, chunk->start, chunk->capacity);
  FREE_ARRAY(Token, chunk->end, chunk->capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

int add_constant(Chunk* chunk, Value value) {
  push(value);  // Prevent GC from freeing the value.
  write_value_array(&chunk->constants, value);
  pop();  // Release the value.
  return chunk->constants.count - 1;
}
