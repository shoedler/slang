#ifndef strbuf_h
#define strbuf_h

#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "vm.h"

#define INITIAL_STRBUF_CAPACITY 32
#define GROW_STRBUF_CAPACITY(capacity) \
  ((capacity) < INITIAL_STRBUF_CAPACITY ? INITIAL_STRBUF_CAPACITY : (capacity) * ARRAY_GROWTH_FACTOR)

typedef struct {
  int capacity;
  int count;
  char* chars;
} StringBuffer;

static inline void init_strbuf(StringBuffer* buffer) {
  buffer->capacity = INITIAL_STRBUF_CAPACITY;
  buffer->count    = 0;
  buffer->chars    = (char*)malloc(INITIAL_STRBUF_CAPACITY);
  if (buffer->chars == NULL) {
    INTERNAL_ERROR("Failed to allocate memory for string buffer");
    exit(EMEM_ERROR);
  }
  buffer->chars[0] = '\0';
}

static inline void strbuf_append(StringBuffer* buffer, const char* str, int len) {
  int required_capacity = buffer->count + len + 1;  // +1 for null terminator

  if (SHOULD_GROW(required_capacity, buffer->capacity)) {
    // Grow the buffer linearly until it's large enough
    do {
      int old_capacity = buffer->capacity;
      buffer->capacity = GROW_STRBUF_CAPACITY(old_capacity);
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

// Turns the buffer into a string obj. The returned string is an interned string, which will take over the buffer's memory
// (take_string) instead of copying it. (copy_string). After this call, the buffer should not be used anymore.
static inline ObjString* strbuf_take_string(StringBuffer* buffer) {
  return take_string(buffer->chars, buffer->count);
}

#endif
