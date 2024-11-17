#ifndef memory_h
#define memory_h

#include "common.h"
#include "object.h"

// GC Configuration constants

// Heap config
#define GC_HEAP_GROW_FACTOR 2
#define GC_HEAP_GROW_THRESHOLD 0x4000000  // 64 MB
#define GC_HEAP_DEFAULT_THRESHOLD 1024 * 1024 * 2

// Parallelization config
#define GC_PARALLEL_MARK_ARRAY_THRESHOLD 10000     // TODO (optimize): Can probably be reduced a bit after synch refactoring
#define GC_PARALLEL_MARK_HASHTABLE_THRESHOLD 2000  // TODO (optimize): Can probably be reduced a bit after synch refactoring
#define GC_PARALLEL_SWEEP_THRESHOLD 100000
#define GC_DEQUE_INITIAL_CAPACITY 1024
#define GC_DEQUE_INITIAL_GARBAGE_CAPACITY 32

// Allocate memory for an array. Might trigger gc.
#define ALLOCATE_ARRAY(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))

// Allocates a new object of the given type.
#define ALLOCATE_OBJ(type, object_type) (type*)allocate_object(sizeof(type), object_type)

// Free memory of an array by resizing it to 0. Might trigger gc.
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// Factor 2 is used, which is common for dynamic arrays. First-timers:
// https://en.wikipedia.org/wiki/Dynamic_array#Geometric_expansion_and_amortized_cost
#define ARRAY_GROWTH_FACTOR 2

// Initial capacity of a dynamic array, when first writing to it.
#define INITIAL_ARRAY_CAPACITY 8

// Grow the capacity of a dynamic array. We start by setting the capacity to 8, to avoid reallocating too often.
#define GROW_CAPACITY(capacity) ((capacity) < INITIAL_ARRAY_CAPACITY ? INITIAL_ARRAY_CAPACITY : (capacity) * ARRAY_GROWTH_FACTOR)

// Shrink the capacity of a dynamic array.
#define SHRINK_CAPACITY(capacity) ((capacity) / ARRAY_GROWTH_FACTOR)

// Tests whether the dynamic array should be grown.
#define SHOULD_GROW(target_count, current_capacity) (target_count > current_capacity)

// Tests whether the dynamic array should be shrunk.
#define SHOULD_SHRINK(target_count, current_capacity) \
  (target_count < current_capacity / 4 && current_capacity > INITIAL_ARRAY_CAPACITY)

// Grow a dynamic array.
#define RESIZE_ARRAY(type, pointer, old_count, new_count) \
  (type*)reallocate(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))

// Free a dynamic array.
#define FREE_ARRAY(type, pointer, old_count) reallocate(pointer, sizeof(type) * (old_count), 0)

// Reallocate memory. This is the single function which handles dynamic array memory management.
// If old_size == 0 and new_size != 0, then the function allocates memory.
// If old_size != 0 and new_size == 0, then the function frees memory.
// If old_size != 0 and new_size <old_size, then the function shrinks memory.
// If old_size != 0 and new_size > old_size, then the function expands memory.
void* reallocate(void* pointer, size_t old_size, size_t new_size);

// Tri-color gc
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

// Marks a value gray. Only heap allocated values need to be marked. Currently, this is only objects.
void mark_value(Value value);

// Marks an object gray by setting its is_marked field to true. And adding it to the gray stack. (Worklist) The worklist is owned
// by the vm and does not use our own memory allocation functions to decouple it from the garbage collector.
void mark_obj(Obj* object);

// Allocates a new object of the given type and size. It also initializes the object's fields. Might trigger GC, but (obviously)
// won't free the new object to be allocated.
Obj* allocate_object(size_t size, ObjGcType type);

// Frees the vm's linked list of objects.
void free_heap();

// Initializes the thread pool for the garbage collector.
void gc_init_thread_pool(int num_threads);

// Shuts down the thread pool for the garbage collector and frees all resources.
void gc_shutdown_thread_pool();

#endif
