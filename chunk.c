#include "chunk.h"
#include "memory.h"
#include "vm.h"

void init_chunk(Chunk* chunk) {
  chunk->count    = 0;
  chunk->capacity = 0;
  chunk->code     = NULL;
  chunk->lines    = NULL;
  init_value_array(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint16_t data, int line) {
  if (SHOULD_GROW(chunk->count + 1, chunk->capacity)) {
    int old_capacity = chunk->capacity;
    chunk->capacity  = GROW_CAPACITY(old_capacity);
    chunk->code      = RESIZE_ARRAY(uint16_t, chunk->code, old_capacity, chunk->capacity);
    chunk->lines     = RESIZE_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
  }

  chunk->code[chunk->count]  = data;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

void free_chunk(Chunk* chunk) {
  FREE_ARRAY(uint16_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

int add_constant(Chunk* chunk, Value value) {
  push(value);  // Prevent GC from freeing the value.
  write_value_array(&chunk->constants, value);
  pop();  // Release the value.
  return chunk->constants.count - 1;
}
