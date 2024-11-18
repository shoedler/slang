#ifndef GC_DEQUE_H
#define GC_DEQUE_H

#include <stdalign.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GC_DEQUE_INITIAL_GARBAGE_CAPACITY 32
#define GC_DEQUE_ALIGNMENT 64

typedef struct GCTask {
  void (*function)(void* arg);
  void* arg;
} GCTask;

// Ring buffer for the deque
struct RingBuf;

// Work-stealing deque structure
typedef struct WorkStealingDeque {
  // Cache line alignment for atomic variables
  alignas(GC_DEQUE_ALIGNMENT) atomic_int_least64_t top;
  alignas(GC_DEQUE_ALIGNMENT) atomic_int_least64_t bottom;
  alignas(GC_DEQUE_ALIGNMENT) atomic_uintptr_t buffer;  // Points to RingBuf

  // Store old buffers for cleanup
  struct RingBuf** garbage;
  size_t garbage_size;
  size_t garbage_capacity;
} WorkStealingDeque;

// Create a new work-stealing deque
WorkStealingDeque* ws_deque_init(int64_t initial_capacity);

// Cleanup and destroy the deque
void ws_deque_free(WorkStealingDeque* deque);

// Pop a task (only owner thread)
bool ws_deque_pop(WorkStealingDeque* deque, GCTask* task);

// Push a task to the deque (only owner thread)
bool ws_deque_push(WorkStealingDeque* deque, GCTask task);

// Steal a task (any thread)
bool ws_deque_steal(WorkStealingDeque* deque, GCTask* task);

// Get current size
size_t ws_deque_size(WorkStealingDeque* deque);

// Get current capacity
size_t ws_deque_capacity(WorkStealingDeque* deque);

// Check if empty
bool ws_deque_empty(WorkStealingDeque* deque);

#endif  // GC_DEQUE_H
