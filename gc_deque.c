#include "gc_deque.h"
#include <assert.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct RingBuf {
  int64_t capacity;
  int64_t mask;    // Bit mask for modulo operations
  GCTask* buffer;  // Array of tasks
} RingBuf;

// Create a new ring buffer with given capacity (must be power of 2)
static RingBuf* ring_buf_init(int64_t capacity) {
  assert(capacity > 0 && !(capacity & (capacity - 1)) && "Capacity must be power of 2!");

  RingBuf* ring_buf = (RingBuf*)malloc(sizeof(RingBuf));
  if (!ring_buf) {
    return NULL;
  }

  ring_buf->capacity = capacity;
  ring_buf->mask     = capacity - 1;
  ring_buf->buffer   = (GCTask*)calloc(capacity, sizeof(GCTask));

  if (!ring_buf->buffer) {
    free(ring_buf);
    return NULL;
  }

  return ring_buf;
}

// Store task at modulo index
static void ring_buf_store(RingBuf* ring_buf, int64_t index, GCTask task) {
  ring_buf->buffer[index & ring_buf->mask] = task;
}

// Load task at modulo index
static GCTask ring_buf_load(RingBuf* ring_buf, int64_t index) {
  return ring_buf->buffer[index & ring_buf->mask];
}
// Resize the ring buffer
static RingBuf* ring_buf_resize(RingBuf* ring_buf, int64_t bottom, int64_t top) {
  RingBuf* new_rb = ring_buf_init(ring_buf->capacity * 2);
  if (!new_rb) {
    return NULL;
  }

  for (int64_t i = top; i != bottom; ++i) {
    ring_buf_store(new_rb, i, ring_buf_load(ring_buf, i));
  }

  return new_rb;
}

WorkStealingDeque* ws_deque_init(int64_t initial_capacity) {
  WorkStealingDeque* deque = (WorkStealingDeque*)malloc(sizeof(WorkStealingDeque));
  if (!deque) {
    return NULL;
  }

  RingBuf* ring_buf = ring_buf_init(initial_capacity);
  if (!ring_buf) {
    free(deque);
    return NULL;
  }

  atomic_init(&deque->top, 0);
  atomic_init(&deque->bottom, 0);
  atomic_init(&deque->buffer, (uintptr_t)ring_buf);

  deque->garbage_capacity = GC_DEQUE_INITIAL_GARBAGE_CAPACITY;
  deque->garbage          = (RingBuf**)malloc(sizeof(RingBuf*) * deque->garbage_capacity);
  deque->garbage_size     = 0;

  if (!deque->garbage) {
    free(ring_buf->buffer);
    free(ring_buf);
    free(deque);
    return NULL;
  }

  return deque;
}

size_t ws_deque_size(WorkStealingDeque* deque) {
  int64_t bottom = atomic_load_explicit(&deque->bottom, memory_order_relaxed);
  int64_t top    = atomic_load_explicit(&deque->top, memory_order_relaxed);
  return bottom >= top ? (size_t)(bottom - top) : 0;
}

size_t ws_deque_capacity(WorkStealingDeque* deque) {
  return ((RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed))->capacity;
}

bool ws_deque_empty(WorkStealingDeque* deque) {
  return ws_deque_size(deque) == 0;
}

bool ws_deque_push(WorkStealingDeque* deque, GCTask task) {
  int64_t bottom    = atomic_load_explicit(&deque->bottom, memory_order_relaxed);
  int64_t top       = atomic_load_explicit(&deque->top, memory_order_acquire);
  RingBuf* ring_buf = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);

  if (ring_buf->capacity < (bottom - top) + 1) {
    RingBuf* new_rb = ring_buf_resize(ring_buf, bottom, top);
    if (!new_rb) {
      return false;
    }

    if (deque->garbage_size == deque->garbage_capacity) {
      size_t new_capacity   = deque->garbage_capacity * 2;
      RingBuf** new_garbage = (RingBuf**)realloc(deque->garbage, sizeof(RingBuf*) * new_capacity);
      if (!new_garbage) {
        free(new_rb->buffer);
        free(new_rb);
        return false;
      }
      deque->garbage          = new_garbage;
      deque->garbage_capacity = new_capacity;
    }

    deque->garbage[deque->garbage_size++] = ring_buf;
    atomic_store_explicit(&deque->buffer, (uintptr_t)new_rb, memory_order_relaxed);
    ring_buf = new_rb;
  }

  ring_buf_store(ring_buf, bottom, task);
  atomic_thread_fence(memory_order_release);
  atomic_store_explicit(&deque->bottom, bottom + 1, memory_order_relaxed);
  return true;
}

bool ws_deque_pop(WorkStealingDeque* deque, GCTask* task) {
  int64_t bottom    = atomic_load_explicit(&deque->bottom, memory_order_relaxed) - 1;
  RingBuf* ring_buf = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);

  atomic_store_explicit(&deque->bottom, bottom, memory_order_relaxed);
  atomic_thread_fence(memory_order_acq_rel);  // Lighter than seq_cst

  int64_t top  = atomic_load_explicit(&deque->top, memory_order_relaxed);
  bool success = false;

  if (top <= bottom) {
    if (top == bottom) {
      if (!atomic_compare_exchange_strong_explicit(&deque->top, &top, top + 1, memory_order_acq_rel, memory_order_relaxed)) {
        success = false;
      } else {
        *task   = ring_buf_load(ring_buf, bottom);
        success = true;
      }
      atomic_store_explicit(&deque->bottom, bottom + 1, memory_order_relaxed);
    } else {
      *task   = ring_buf_load(ring_buf, bottom);
      success = true;
    }
  } else {
    atomic_store_explicit(&deque->bottom, bottom + 1, memory_order_relaxed);
    success = false;
  }

  return success;
}

bool ws_deque_steal(WorkStealingDeque* deque, GCTask* task) {
  int64_t top = atomic_load_explicit(&deque->top, memory_order_acquire);
  atomic_thread_fence(memory_order_acq_rel);  // Lighter than seq_cst
  int64_t bottom = atomic_load_explicit(&deque->bottom, memory_order_acquire);

  if (top < bottom) {
    RingBuf* ring_buf = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_acquire);
    *task             = ring_buf_load(ring_buf, top);

    if (!atomic_compare_exchange_strong_explicit(&deque->top, &top, top + 1, memory_order_acq_rel, memory_order_relaxed)) {
      return false;
    }

    return true;
  }

  return false;
}

void ws_deque_free(WorkStealingDeque* deque) {
  if (!deque) {
    return;
  }

  RingBuf* ring_buf = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);
  free(ring_buf->buffer);
  free(ring_buf);

  for (size_t i = 0; i < deque->garbage_size; i++) {
    free(deque->garbage[i]->buffer);
    free(deque->garbage[i]);
  }

  free(deque->garbage);
  free(deque);
}
