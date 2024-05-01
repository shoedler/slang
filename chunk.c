#include "chunk.h"
#include "memory.h"
#include "vm.h"

void init_chunk(Chunk* chunk) {
  chunk->count        = 0;
  chunk->capacity     = 0;
  chunk->code         = NULL;
  chunk->source_views = NULL;
  init_value_array(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint16_t data, Token error_start, Token error_end) {
  if (SHOULD_GROW(chunk->count + 1, chunk->capacity)) {
    int old_capacity    = chunk->capacity;
    chunk->capacity     = GROW_CAPACITY(old_capacity);
    chunk->code         = RESIZE_ARRAY(uint16_t, chunk->code, old_capacity, chunk->capacity);
    chunk->source_views = RESIZE_ARRAY(SourceView, chunk->source_views, old_capacity, chunk->capacity);
  }

  SourceView source_view = {
      .start       = get_line_start(error_start),
      .error_start = error_start.start,
      .error_end   = error_end.start + error_end.length,
      .line        = error_start.line,
  };

  chunk->code[chunk->count]         = data;
  chunk->source_views[chunk->count] = source_view;
  chunk->count++;
}

void free_chunk(Chunk* chunk) {
  FREE_ARRAY(uint16_t, chunk->code, chunk->capacity);
  FREE_ARRAY(SourceView, chunk->source_views, chunk->capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

int add_constant(Chunk* chunk, Value value) {
  push(value);  // Prevent GC from freeing the value.
  write_value_array(&chunk->constants, value);
  pop();  // Release the value.
  return chunk->constants.count - 1;
}
