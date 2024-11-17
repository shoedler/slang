#ifndef GC_H
#define GC_H

#include <stdbool.h>
#include "hashtable.h"
#include "value.h"

#ifdef DEBUG_GC_WORKER_STATS
#include <stdatomic.h>
#endif

// Parallelization config
#define GC_PARALLEL_MARK_ARRAY_THRESHOLD 10000     // TODO (optimize): Can probably be reduced a bit after synch refactoring
#define GC_PARALLEL_MARK_HASHTABLE_THRESHOLD 2000  // TODO (optimize): Can probably be reduced a bit after synch refactoring
#define GC_PARALLEL_SWEEP_THRESHOLD 100000
#define GC_DEQUE_INITIAL_CAPACITY 1024

#ifdef DEBUG_GC_WORKER
#define GC_WORKER_LOG(...) printf(__VA_ARGS__)
#else
#define GC_WORKER_LOG(...)
#endif

#ifdef DEBUG_GC_SWEEP
#define GC_SWEEP_LOG(...) printf(__VA_ARGS__)
#else
#define GC_SWEEP_LOG(...)
#endif

#ifdef DEBUG_GC_WORKER
#define GC_WORKER_LOG(...) printf(__VA_ARGS__)
#else
#define GC_WORKER_LOG(...)
#endif

#ifdef DEBUG_GC_SWEEP
#define GC_SWEEP_LOG(...) printf(__VA_ARGS__)
#else
#define GC_SWEEP_LOG(...)
#endif

#ifdef DEBUG_GC_WORKER_STATS
typedef struct {
  atomic_size_t objects_marked;
  atomic_size_t objects_freed;
  uint64_t work_steal_attempts;
  uint64_t successful_steals;
} GCWorkerStats;
#endif

typedef struct {
  union {
    ValueArray* array;
    HashTable* table;
  };
  int start;
  int end;
  enum {
    MARK_ARRAY,
    MARK_HASHTABLE
  } type;
} MarkRangeTaskArg;

// Initializes the thread pool for the garbage collector.
void gc_init_thread_pool(int num_threads);

// Shuts down the thread pool for the garbage collector and frees all resources.
void gc_shutdown_thread_pool();

// Marks an array of values in parallel.
void gc_parallel_mark_array(ValueArray* array);

// Marks a hashtable in parallel.
void gc_parallel_mark_hashtable(HashTable* table);

// Sweeps the heap in parallel. Returns true if parallel sweep was successful, false if it failed in setting up - indicating that
// the sweep should be done sequentially.
bool gc_parallel_sweep();

// Assigns a thread to a worker. If [worker_id] is -1, the current_worker is set to NULL.
void gc_assign_current_worker(int worker_id);

// Wakes up all workers from sleep.
void gc_wake_workers();

// Puts workers to sleep.
void gc_workers_put_to_sleep();

// Intended to only be called from the main thread. Executes the threads own work and participates by trying to steal work, waits
// for all workers to finish.
void gc_wait_for_workers(void);

#ifdef DEBUG_GC_WORKER_STATS
// Increments the marked object count for the current worker.
void gc_inc_worker_stats_marked();

// Increments the freed object count for the current worker.
void gc_inc_worker_stats_freed();

// Prints worker stats.
void gc_print_worker_stats();

#define GC_WORKER_STATS_INC_MARKED() gc_inc_worker_stats_marked()
#define GC_WORKER_STATS_INC_FREED() gc_inc_worker_stats_freed()
#else
#define GC_WORKER_STATS_INC_MARKED()
#define GC_WORKER_STATS_INC_FREED()
#endif

#endif  // GC_H
