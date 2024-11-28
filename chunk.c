#include "chunk.h"
#include <stddef.h>
#include <stdint.h>
#include "memory.h"
#include "scanner.h"
#include "value.h"
#include "vm.h"

void chunk_init(Chunk* chunk) {
  chunk->count        = 0;
  chunk->capacity     = 0;
  chunk->code         = NULL;
  chunk->source_views = NULL;
  value_array_init(&chunk->constants);
}

void chunk_write(Chunk* chunk, uint16_t data, Token error_start, Token error_end) {
  if (SHOULD_GROW(chunk->count + 1, chunk->capacity)) {
    int old_capacity    = chunk->capacity;
    chunk->capacity     = GROW_CAPACITY(old_capacity);
    chunk->code         = RESIZE_ARRAY(uint16_t, chunk->code, old_capacity, chunk->capacity);
    chunk->source_views = RESIZE_ARRAY(SourceView, chunk->source_views, old_capacity, chunk->capacity);
  }

  const char* start = scanner_get_line_start(error_start);
  const char* end =
      error_start.start == error_end.start ? error_start.start + error_start.length : error_end.start + error_end.length;

  SourceView source_view = {
      .start           = start,
      .error_start_ofs = (uint16_t)(error_start.start - start),
      .error_end_ofs   = (uint16_t)(end - start),
      .line            = error_start.line,
  };

  chunk->code[chunk->count]         = data;
  chunk->source_views[chunk->count] = source_view;
  chunk->count++;
}

void chunk_free(Chunk* chunk) {
  FREE_ARRAY(uint16_t, chunk->code, chunk->capacity);
  FREE_ARRAY(SourceView, chunk->source_views, chunk->capacity);
  value_array_free(&chunk->constants);
  chunk_init(chunk);
}

int chunk_add_constant(Chunk* chunk, Value value) {
  vm_push(value);  // Prevent GC from freeing the value.
  value_array_write(&chunk->constants, value);
  vm_pop();  // Release the value.
  return chunk->constants.count - 1;
}
