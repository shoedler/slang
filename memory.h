#ifndef memory_h
#define memory_h

#include <stddef.h>
#include "object.h"
#include "value.h"

// GC Configuration constants

// Heap config
#define HEAP_GROW_FACTOR 2
#define HEAP_GROW_THRESHOLD 0x4000000  // 64 MB
#define HEAP_DEFAULT_THRESHOLD 1024 * 1024 * 2

// Max load factor for tables.
#define TABLE_MAX_LOAD 0.75

// Allocate memory for an array. Might trigger gc.
#define ALLOCATE_ARRAY(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))

// Factor 2 is used, which is common for dynamic arrays. First-timers:
// https://en.wikipedia.org/wiki/Dynamic_array#Geometric_expansion_and_amortized_cost
#define ARRAY_GROWTH_FACTOR 2

// Initial capacity of a dynamic array, when first writing to it.
#define ARRAY_INITIAL_CAPACITY 8

// Grow the capacity of a dynamic array. We start by setting the capacity to 8, to avoid reallocating too often.
#define GROW_CAPACITY(capacity) ((capacity) < ARRAY_INITIAL_CAPACITY ? ARRAY_INITIAL_CAPACITY : (capacity) * ARRAY_GROWTH_FACTOR)

// Shrink the capacity of a dynamic array.
#define SHRINK_CAPACITY(capacity) ((capacity) / ARRAY_GROWTH_FACTOR)

// Tests whether the dynamic array should be grown.
#define SHOULD_GROW(target_count, current_capacity) (target_count > current_capacity)

// Tests whether the dynamic array should be shrunk.
#define SHOULD_SHRINK(target_count, current_capacity) \
  (target_count < current_capacity / 4 && current_capacity > ARRAY_INITIAL_CAPACITY)

// Grow a dynamic array.
#define RESIZE_ARRAY(type, pointer, old_count, new_count) \
  (type*)reallocate(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))

// Free a dynamic array.
#define FREE_ARRAY(type, pointer, old_count) reallocate(pointer, sizeof(type) * (old_count), 0)

#ifdef DEBUG_GC_PHASE_TIMES
typedef struct {
  size_t cycle_count;
  double cycle_start;  // Start timestamp of the current cycle
  double mark_time;
  double remove_white_time;
  double sweep_time;
  double prev_mutator_time;  // Time spent in the mutator before the current cycle
  double runtime;            // Total runtime of the program
} GcPhaseTimes;
#endif

// Reallocate memory. This is the single function which handles dynamic array memory management.
// If old_size == 0 and new_size != 0, then the function allocates memory.
// If old_size != 0 and new_size == 0, then the function frees memory.
// If old_size != 0 and new_size <old_size, then the function shrinks memory.
// If old_size != 0 and new_size > old_size, then the function expands memory.
void* reallocate(void* pointer, size_t old_size, size_t new_size);

// Tri-color (parallel) gc
//
// ░ White: At the beginning of a garbage collection, every object is white. This color means we have not reached or processed the
// object at all.
//
// ▒ Gray: During marking, when we first reach an object, we darken it gray. This color means we know the object itself is
// reachable and should not be collected. But we have not yet traced through it to see what other objects it references. In graph
// algorithm terms, this is the worklist—the set of objects we know about but haven’t processed yet.
//
// █ Black: When we take a gray object and mark all of the objects it references, we then turn the gray object black. This color
// means the mark phase is done processing that object.
void collect_garbage();

// Marks a value gray. Everything that is not an object is ignored.
void mark_value(Value value);

// Marks an object gray by setting its is_marked field to true, indicating that it has been reached by the garbage collector.
void mark_obj(Obj* object);

// Allocates a new object of the given type and size on the heap. It also initializes the object's fields.
Obj* allocate_obj(size_t size, ObjGcType type);

// Frees an object from our heap. How we free an object depends on its type.
void free_obj(Obj* object);

// Frees the vm's linked list of objects.
void free_heap();

#endif
