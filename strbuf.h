#ifndef strbuf_h
#define strbuf_h

#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "vm.h"

typedef struct {
  int capacity;
  int count;
  char* chars;
} StringBuffer;

static inline void init_string_buffer(StringBuffer* buffer) {
  buffer->capacity = 0;
  buffer->count    = 0;
  buffer->chars    = NULL;
}

static inline void string_buf_append(StringBuffer* buffer, const char* str, int len) {
  int required_capacity = buffer->count + len + 1;  // +1 for null terminator

  if (SHOULD_GROW(required_capacity, buffer->capacity)) {
    // Grow the buffer linearly until it's large enough
    do {
      int old_capacity = buffer->capacity;
      buffer->capacity = GROW_CAPACITY(old_capacity);
    } while (buffer->capacity < required_capacity);

    buffer->chars = (char*)realloc(buffer->chars, buffer->capacity);
    if (buffer->chars == NULL) {
      INTERNAL_ERROR("Failed to allocate memory for string buffer");
      exit(EMEM_ERROR);
    }
  }

  memcpy(buffer->chars + buffer->count, str, len);
  buffer->count += len;
  buffer->chars[buffer->count] = '\0';
}

static inline ObjString* string_buf_take(StringBuffer* buffer) {
  return take_string(buffer->chars, buffer->count);
}

#endif
