#include "gc_deque.h"
#include <assert.h>

typedef struct RingBuf {
  int64_t capacity;
  int64_t mask;    // Bit mask for modulo operations
  GCTask* buffer;  // Array of tasks
} RingBuf;

// Create a new ring buffer with given capacity (must be power of 2)
static RingBuf* ring_buf_init(int64_t capacity) {
  assert(capacity > 0 && !(capacity & (capacity - 1)) && "Capacity must be power of 2!");

  RingBuf* rb = (RingBuf*)malloc(sizeof(RingBuf));
  if (!rb) {
    return NULL;
  }

  rb->capacity = capacity;
  rb->mask     = capacity - 1;
  rb->buffer   = (GCTask*)calloc(capacity, sizeof(GCTask));

  if (!rb->buffer) {
    free(rb);
    return NULL;
  }

  return rb;
}

// Store task at modulo index
static void ring_buf_store(RingBuf* rb, int64_t i, GCTask task) {
  rb->buffer[i & rb->mask] = task;
}

// Load task at modulo index
static GCTask ring_buf_load(RingBuf* rb, int64_t i) {
  return rb->buffer[i & rb->mask];
}
// Resize the ring buffer
static RingBuf* ring_buf_resize(RingBuf* rb, int64_t bottom, int64_t top) {
  RingBuf* new_rb = ring_buf_init(rb->capacity * 2);
  if (!new_rb) {
    return NULL;
  }

  for (int64_t i = top; i != bottom; ++i) {
    ring_buf_store(new_rb, i, ring_buf_load(rb, i));
  }

  return new_rb;
}

WorkStealingDeque* ws_deque_init(int64_t initial_capacity) {
  WorkStealingDeque* deque = (WorkStealingDeque*)malloc(sizeof(WorkStealingDeque));
  if (!deque) {
    return NULL;
  }

  RingBuf* rb = ring_buf_init(initial_capacity);
  if (!rb) {
    free(deque);
    return NULL;
  }

  atomic_init(&deque->top, 0);
  atomic_init(&deque->bottom, 0);
  atomic_init(&deque->buffer, (uintptr_t)rb);

  deque->garbage_capacity = GC_DEQUE_INITIAL_GARBAGE_CAPACITY;
  deque->garbage          = (RingBuf**)malloc(sizeof(RingBuf*) * deque->garbage_capacity);
  deque->garbage_size     = 0;

  if (!deque->garbage) {
    free(rb->buffer);
    free(rb);
    free(deque);
    return NULL;
  }

  return deque;
}

size_t ws_deque_size(WorkStealingDeque* deque) {
  int64_t b = atomic_load_explicit(&deque->bottom, memory_order_relaxed);
  int64_t t = atomic_load_explicit(&deque->top, memory_order_relaxed);
  return b >= t ? (size_t)(b - t) : 0;
}

size_t ws_deque_capacity(WorkStealingDeque* deque) {
  return ((RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed))->capacity;
}

bool ws_deque_empty(WorkStealingDeque* deque) {
  return ws_deque_size(deque) == 0;
}

bool ws_deque_push(WorkStealingDeque* deque, GCTask task) {
  int64_t b   = atomic_load_explicit(&deque->bottom, memory_order_relaxed);
  int64_t t   = atomic_load_explicit(&deque->top, memory_order_acquire);
  RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);

  if (rb->capacity < (b - t) + 1) {
    RingBuf* new_rb = ring_buf_resize(rb, b, t);
    if (!new_rb)
      return false;

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

    deque->garbage[deque->garbage_size++] = rb;
    atomic_store_explicit(&deque->buffer, (uintptr_t)new_rb, memory_order_relaxed);
    rb = new_rb;
  }

  ring_buf_store(rb, b, task);
  atomic_thread_fence(memory_order_release);
  atomic_store_explicit(&deque->bottom, b + 1, memory_order_relaxed);
  return true;
}

bool ws_deque_pop(WorkStealingDeque* deque, GCTask* task) {
  int64_t b   = atomic_load_explicit(&deque->bottom, memory_order_relaxed) - 1;
  RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);

  atomic_store_explicit(&deque->bottom, b, memory_order_relaxed);
  atomic_thread_fence(memory_order_seq_cst);

  int64_t t    = atomic_load_explicit(&deque->top, memory_order_relaxed);
  bool success = false;

  if (t <= b) {
    if (t == b) {
      if (!atomic_compare_exchange_strong_explicit(&deque->top, &t, t + 1, memory_order_seq_cst, memory_order_relaxed)) {
        success = false;
      } else {
        *task   = ring_buf_load(rb, b);
        success = true;
      }
      atomic_store_explicit(&deque->bottom, b + 1, memory_order_relaxed);
    } else {
      *task   = ring_buf_load(rb, b);
      success = true;
    }
  } else {
    atomic_store_explicit(&deque->bottom, b + 1, memory_order_relaxed);
    success = false;
  }

  return success;
}

bool ws_deque_steal(WorkStealingDeque* deque, GCTask* task) {
  int64_t t = atomic_load_explicit(&deque->top, memory_order_acquire);
  atomic_thread_fence(memory_order_seq_cst);
  int64_t b = atomic_load_explicit(&deque->bottom, memory_order_acquire);

  if (t < b) {
    RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_consume);
    *task       = ring_buf_load(rb, t);

    if (!atomic_compare_exchange_strong_explicit(&deque->top, &t, t + 1, memory_order_seq_cst, memory_order_relaxed)) {
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

  RingBuf* rb = (RingBuf*)atomic_load_explicit(&deque->buffer, memory_order_relaxed);
  free(rb->buffer);
  free(rb);

  for (size_t i = 0; i < deque->garbage_size; i++) {
    free(deque->garbage[i]->buffer);
    free(deque->garbage[i]);
  }

  free(deque->garbage);
  free(deque);
}
