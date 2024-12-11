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

  SourceView source_view = chunk_make_source_view(error_start, error_end);

  chunk->code[chunk->count]         = data;
  chunk->source_views[chunk->count] = source_view;
  chunk->count++;
}

SourceView chunk_make_source_view(Token error_start, Token error_end) {
  const char* start = scanner_get_line_start(error_start);
  const char* end =
      error_start.start == error_end.start ? error_start.start + error_start.length : error_end.start + error_end.length;

  return (SourceView){
      .start           = start,
      .error_start_ofs = (uint16_t)(error_start.start - start),
      .error_end_ofs   = (uint16_t)(end - start),
      .line            = error_start.line,
  };
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

void report_error_location(SourceView source) {
  const char* error_end   = source.start + source.error_end_ofs;
  const char* error_start = source.start + source.error_start_ofs;

  fprintf(stderr, "\n %5d | ", source.line);

  // Print the source code line
  for (const char* chr = source.start; chr < error_end || (chr >= error_end && *chr != '\n' && *chr != '\0'); chr++) {
    if (*chr == '\r') {
      continue;
    }

    if (*chr == '\n') {
      fputs("...", stderr);
      break;
    }

    if (*chr == '/' && chr[1] == '/') {
      break;  // Break if we reach a line comment
    }

    fputc(*chr, stderr);
  }

  // Newline and padding
  fputs("\n         ", stderr);
  for (const char* chr = source.start; chr < error_start; chr++) {
    fputc(' ', stderr);
  }

  // Print the squiggly line
  fputs(ANSI_COLOR_RED, stderr);
  for (const char* chr = error_start; chr < error_end; chr++) {
    if (*chr == '\r') {
      continue;
    }

    if (*chr == '\n') {
      break;
    }

    fputc('~', stderr);
  }
  fputs(ANSI_COLOR_RESET, stderr);

  // Done!
  fputs("\n\n", stderr);
}