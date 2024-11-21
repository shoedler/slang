#include "gc.h"
#include <handleapi.h>
#include <minwindef.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include <unistd.h>
#include <windows.h>
#include <winnt.h>
#include "common.h"
#include "gc_deque.h"
#include "hashtable.h"
#include "memory.h"
#include "sys.h"
#include "value.h"
#include "vm.h"

#if defined(DEBUG_GC_WORKER) || defined(DEBUG_GC_SWEEP)
#include <assert.h>
#endif

typedef struct {
  alignas(GC_DEQUE_ALIGNMENT) WorkStealingDeque* deque;
  alignas(GC_DEQUE_ALIGNMENT) pthread_t thread;
  alignas(GC_DEQUE_ALIGNMENT) int id;
  alignas(GC_DEQUE_ALIGNMENT) atomic_bool done;
#ifdef DEBUG_GC_WORKER_STATS
  alignas(GC_DEQUE_ALIGNMENT) GCWorkerStats stats;
#endif
} GCWorker;

typedef struct {
  GCWorker* workers;
  int worker_count;
  atomic_bool shutdown;

  // Add synchronization primitives
  atomic_bool should_work;
  HANDLE work_event;  // Event to signal work
} GCThreadPool;

// Global state
static GCThreadPool gc_thread_pool = {0};

// Thread-local storage
static __thread GCWorker* current_worker = NULL;

void gc_assign_current_worker(int worker_id) {
  if (worker_id == -1) {
    current_worker = NULL;
  } else {
    current_worker = &gc_thread_pool.workers[worker_id];
  }
}

void gc_wake_workers() {
  GC_WORKER_LOG(ANSI_RED_STR("[GC]") " " ANSI_MAGENTA_STR("[WORKERS]") " Waking up workers\n");
  atomic_store(&gc_thread_pool.should_work, true);
  SetEvent(gc_thread_pool.work_event);
}

void gc_workers_put_to_sleep() {
  GC_WORKER_LOG("  Putting workers to sleep\n\n");
  atomic_store(&gc_thread_pool.should_work, false);
}

// Adds a task to the current worker's deque
static void gc_worker_add_task(void (*function)(void*), void* arg) {
  atomic_store(&current_worker->done, false);
  if (!ws_deque_push(current_worker->deque, (GCTask){.function = function, .arg = arg})) {
    INTERNAL_ERROR("Failed to push task to current worker's deque");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }
}

// Work function. This is where the magic happens and where the workers spend most of their time. It's carefully crafted to
// minimize contention and maximize throughput, handle with care. Returns true if there was either own work or stolen work done,
// false otherwise.
static bool gc_worker_do_work() {
  // Do our own work
  GCTask task;
  bool have_task = ws_deque_pop(current_worker->deque, &task);

  // Try to steal work if we don't have any of our own
  // TODO (optimize): Running the aoc sample, it seems that the main thread has not even one successful steal - which kinda makes
  // sense. We might get a little bit of a gain if we skip that part for the main thread, though this is deeply dependent on the
  // workload aswell as the tuning parameters.
  if (!have_task) {
    for (int i = 0; i < gc_thread_pool.worker_count; i++) {
      if (i == current_worker->id) {
        continue;
      }

#ifdef DEBUG_GC_WORKER_STATS
      current_worker->stats.work_steal_attempts++;
#endif
      if (ws_deque_steal(gc_thread_pool.workers[i].deque, &task)) {
#ifdef DEBUG_GC_WORKER_STATS
        current_worker->stats.successful_steals++;
#endif
        GC_WORKER_LOG("    Worker %d stole task %p from worker %d\n", current_worker->id, (void*)&task, i);
        have_task = true;
        break;
      }
    }
  }

  if (have_task) {
    GC_WORKER_LOG("    Worker %d executing task %p\n", current_worker->id, (void*)&task);
    task.function(task.arg);
  }

  return have_task;
}

void gc_wait_for_workers() {
  bool all_done = false;
  do {
    if (gc_worker_do_work()) {
      continue;
    }

    // Check if all workers are done - skip worker 0, since that's us
    all_done = true;
    for (int i = 1; i < gc_thread_pool.worker_count; i++) {
      if (!atomic_load(&gc_thread_pool.workers[i].done)) {
        all_done = false;
        break;
      }
    }
  } while (!all_done);
  GC_WORKER_LOG("  All workers done\n");
}

// Worker thread function
static void* gc_worker(void* arg) {
  GCWorker* worker = (GCWorker*)arg;
  current_worker   = worker;
  GC_WORKER_LOG("Worker %d started\n", worker->id);

  while (!atomic_load(&gc_thread_pool.shutdown)) {
    // Between GC cycles - deep sleep with no busy waiting
    if (!atomic_load(&gc_thread_pool.should_work)) {
      WaitForSingleObject(gc_thread_pool.work_event, INFINITE);
      continue;
    }

    atomic_store(&worker->done, false);
    if (!gc_worker_do_work()) {
      atomic_store(&worker->done, true);
      usleep(1000);
    }
  }

  GC_WORKER_LOG("Worker %d shutting down\n", worker->id);
  return NULL;
}

// General range-marking task
static void gc_mark_range_task(void* arg) {
  MarkRangeTaskArg* task_arg = (MarkRangeTaskArg*)arg;

  if (task_arg->type == MARK_ARRAY) {
    ValueArray* array = task_arg->array;
    for (int i = task_arg->start; i < task_arg->end; i++) {
      mark_value(array->values[i]);
    }
  } else {
    HashTable* table = task_arg->table;
    for (int i = task_arg->start; i < task_arg->end; i++) {
      Entry* entry = &table->entries[i];
      if (!is_empty_internal(entry->key)) {
        mark_value(entry->key);
        mark_value(entry->value);
      }
    }
  }

  free(task_arg);
}

void gc_parallel_mark_array(ValueArray* array) {
#ifdef DEBUG_GC_WORKER
  assert(GC_PARALLEL_MARK_ARRAY_THRESHOLD > gc_thread_pool.worker_count * 200 &&
         "Probably not worth parallelizing if a chunk would be smaller than 200 elements. ");
#endif
  int num_chunks = gc_thread_pool.worker_count;
  int chunk_size = (array->count + num_chunks - 1) / num_chunks;

  // Generate tasks in our own deque, since we can't push to other deques directly. Other workers will steal from us.
  for (int i = 0; i < num_chunks; i++) {
    int start = i * chunk_size;
    if (start >= array->count) {
      break;
    }

    int end = MIN(start + chunk_size, array->count);

    MarkRangeTaskArg* arg = malloc(sizeof(MarkRangeTaskArg));
    arg->array            = array;
    arg->start            = start;
    arg->end              = end;
    arg->type             = MARK_ARRAY;

    gc_worker_add_task(gc_mark_range_task, arg);
  }
}

void gc_parallel_mark_hashtable(HashTable* table) {
#ifdef DEBUG_GC_WORKER
  assert(GC_PARALLEL_MARK_HASHTABLE_THRESHOLD > gc_thread_pool.worker_count * 100 &&
         "Probably not worth parallelizing if a chunk would be smaller than 100 elements. ");
#endif
  int num_chunks = gc_thread_pool.worker_count;
  int chunk_size = (table->capacity + num_chunks - 1) / num_chunks;

  // Generate tasks in our own deque, since we can't push to other deques directly. Other workers will steal from us.
  for (int i = 0; i < num_chunks; i++) {
    int start = i * chunk_size;
    if (start >= table->capacity) {
      break;
    }

    int end = MIN(start + chunk_size, table->capacity);

    MarkRangeTaskArg* arg = malloc(sizeof(MarkRangeTaskArg));
    arg->table            = table;
    arg->start            = start;
    arg->end              = end;
    arg->type             = MARK_HASHTABLE;

    gc_worker_add_task(gc_mark_range_task, arg);
  }
}

typedef struct {
  alignas(GC_DEQUE_ALIGNMENT) Obj* start;   // Inclusive. If NULL, then the whole chunk was freed
  alignas(GC_DEQUE_ALIGNMENT) Obj* end;     // Inclusive too
  alignas(GC_DEQUE_ALIGNMENT) size_t size;  // Number of objects in the chunk. E.g. size=4 means 0=start, 1, 2, 3=end
} SweepChunk;

static void gc_sweep_chunk_task(void* arg) {
  SweepChunk* chunk = (SweepChunk*)arg;

  size_t chunk_size = chunk->size;
  Obj* current      = chunk->start;

  Obj* previous  = NULL;
  Obj* new_start = NULL;
  Obj* new_end   = NULL;

  GC_SWEEP_LOG("  Worker %d: Sweeping chunk %p-%p (%zu objects)\n", current_worker->id, chunk->start, chunk->end, chunk->size);

#ifdef DEBUG_GC_SWEEP
  assert(current != NULL && "Object count mismatch");
#endif
  for (size_t i = 0; i < chunk_size; i++) {
    // Atomically check and reset the is_marked flag
    if (atomic_exchange(&current->is_marked, false)) {  // Object is marked (black), keep it
      // Move new_start to the first marked object
      if (new_start == NULL) {
        new_start = current;
      }

      new_end = current;  // Move end to the last marked object

      previous = current;
      current  = current->next;
    } else {  // Object is unmarked (white), remove it
      Obj* unreached = current;
      current        = current->next;

      // Update the linked list
      if (previous != NULL) {
        previous->next = current;
      }

      free_object(unreached);
    }
  }

  // Update chunk boundaries for list reconstruction
  chunk->start = new_start;  // Could be NULL if all objects in chunk were freed
  chunk->end   = new_end;    // Could be NULL if all objects in chunk were freed

  GC_SWEEP_LOG("  Worker %d: Done sweeping\n", current_worker->id);
}

bool gc_parallel_sweep() {
  GC_SWEEP_LOG(ANSI_RED_STR("[GC]") " " ANSI_CYAN_STR("[SWEEP]") " Sweeping heap parallel\n");
  size_t total_object_count = atomic_load(&vm.object_count);

// Calculate base chunk size and remainder
#ifdef DEBUG_GC_SWEEP
  assert(GC_PARALLEL_SWEEP_THRESHOLD >
             ((2 /* 2 objs per chunk */ * gc_thread_pool.worker_count) * 2 /* we want 2*threadcount chunks */) &&
         "We must have at least two objects per chunk so we can guarantee that start and end are different");
#endif
  int num_chunks         = gc_thread_pool.worker_count * 2;  // *2, so *maybe* the main thread can help out after generating tasks
  size_t base_chunk_size = total_object_count / num_chunks;
  size_t remainder       = total_object_count % num_chunks;

  GC_SWEEP_LOG("  Sweeping %zu objects in %d chunks of size %zu with remainder %zu\n", total_object_count, num_chunks,
               base_chunk_size, remainder);

  // Allocate chunks on heap since they'll be accessed asynchronously
  SweepChunk** chunks = malloc(sizeof(SweepChunk) * num_chunks);
  if (chunks == NULL) {
    return false;  // Indicate that we failed to setup parallel sweep
  }

  // Distribute objects into chunks
  Obj* current = vm.objects;
  for (int chunk_i = 0; chunk_i < num_chunks; chunk_i++) {
    chunks[chunk_i] = malloc(sizeof(SweepChunk));
    if (chunks[chunk_i] == NULL) {
      // Free all previous chunks and the chunks array itself
      for (int j = 0; j < chunk_i; j++) {
        free(chunks[j]);
      }
      free(chunks);
      return false;  // Indicate that we failed to setup parallel sweep
    }

    chunks[chunk_i]->start = current;
    chunks[chunk_i]->size  = base_chunk_size;

    if (remainder > 0) {  // Since remainder is total_object_count % num_chunks, it's always less than num_chunks
      chunks[chunk_i]->size++;
      remainder--;
    }

    // Move to end of this chunk (exclusive)
    for (size_t i = 0; i < chunks[chunk_i]->size - 1; i++) {
      current = current->next;
    }

    chunks[chunk_i]->end = current;
    current              = current->next;  // Move to start of next chunk

    // Submit task to worker
    GC_SWEEP_LOG("  Main thread: Adding sweep task for chunk #%d %p-%p  (%zu objects)\n", chunk_i, chunks[chunk_i]->start,
                 chunks[chunk_i]->end, chunks[chunk_i]->size);
    gc_worker_add_task(gc_sweep_chunk_task, chunks[chunk_i]);
  }
#ifdef DEBUG_GC_SWEEP
  assert(current == NULL && "Object count mismatch, current should be at the end of the obj linked-list");
  GC_SWEEP_LOG("  Main thread: Done submitting sweep tasks\n");
#endif

  // Only begin stitching the list back together after all chunks have been processed
  gc_wait_for_workers();

  GC_SWEEP_LOG("  Main thread: Done waiting for workers, stitching...\n");

  // Stitch the list back together, being careful about NULLs
  vm.objects = NULL;
  Obj* tail  = NULL;
  for (int chunk_i = 0; chunk_i < num_chunks; chunk_i++) {
    if (chunks[chunk_i]->start == NULL) {
      continue;  // Skip empty chunks
    }
    if (vm.objects == NULL) {
      vm.objects = chunks[chunk_i]->start;  // Pick up the first non-NULL chunk
    } else {
      tail->next = chunks[chunk_i]->start;
    }
    tail = chunks[chunk_i]->end;
    free(chunks[chunk_i]);
  }

  free(chunks);

  GC_SWEEP_LOG("  Done sweeping heap parallel\n\n");
  return true;
}

// Monitoring and statistics
#ifdef DEBUG_GC_WORKER_STATS
void gc_inc_worker_stats_marked() {
  atomic_fetch_add(&current_worker->stats.objects_marked, 1);
}

void gc_inc_worker_stats_freed() {
  atomic_fetch_add(&current_worker->stats.objects_freed, 1);
}

void gc_print_worker_stats() {
  printf(ANSI_RED_STR("[GC]") " " ANSI_YELLOW_STR("[WORKER-STATS]") " Worker Statistics:\n");
  // Print header with proper alignment
  printf("  %-6s  %-12s  %-12s  %-12s  %-12s  %-12s  %-12s\n", "Worker", "Marked", "Freed", "Steal Tries", "Steals",
         "Success Rate", "Deque Cap");
  printf("  %-6s  %-12s  %-12s  %-12s  %-12s  %-12s  %-12s\n", "------", "------", "-----", "-----------", "------",
         "------------", "---------");

  size_t total_marked = 0;
  size_t total_swept  = 0;

  for (int i = 0; i < gc_thread_pool.worker_count; i++) {
    GCWorker* worker  = &gc_thread_pool.workers[i];
    size_t marked     = atomic_load(&worker->stats.objects_marked);
    size_t swept      = atomic_load(&worker->stats.objects_freed);
    uint64_t attempts = worker->stats.work_steal_attempts;
    uint64_t steals   = worker->stats.successful_steals;
    int64_t deque_cap = ws_deque_capacity(worker->deque);
    double rate       = (attempts > 0) ? (steals * 100.0 / attempts) : 0.0;

    printf("  " ANSI_CYAN_STR("%-6d") "  %-12zu  %-12zu  %-12llu  %-12llu  %11.2f%%  %-12lld\n", i, marked, swept, attempts,
           steals, rate, deque_cap);

    total_marked += marked;
    total_swept += swept;

    // Reset stats
    atomic_store(&worker->stats.objects_marked, 0);
    atomic_store(&worker->stats.objects_freed, 0);
    worker->stats.work_steal_attempts = 0;
    worker->stats.successful_steals   = 0;
  }

  // Totals
  printf("  %-6s  %-12s  %-12s\n", "------", "------", "-----");
  printf("  " ANSI_CYAN_STR("%-6s") "  %-12zu  %-12zu\n", "Total", total_marked, total_swept);

  printf("\n");
}
#endif

void gc_init_thread_pool(int num_threads) {
  GC_WORKER_LOG("Initializing thread pool with %d threads\n", num_threads);

  if (num_threads <= 0) {
    INTERNAL_ERROR("Invalid number of threads: %d", num_threads);
    exit(SLANG_EXIT_SW_ERROR);
  }

  // Initialize thread pool struct with aligned memory
  gc_thread_pool.worker_count = num_threads;
  size_t alloc_size           = num_threads * sizeof(GCWorker);
  gc_thread_pool.workers      = _aligned_malloc(alloc_size, 64);
  if (!gc_thread_pool.workers) {
    INTERNAL_ERROR("Failed to allocate memory for worker threads");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }
  memset(gc_thread_pool.workers, 0, alloc_size);

  // Initialize sync primitives
  atomic_init(&gc_thread_pool.shutdown, false);
  atomic_init(&gc_thread_pool.should_work, false);
  gc_thread_pool.work_event = CreateEvent(NULL, TRUE, FALSE, NULL);  // Manual reset event

  // Initialize worker structs
  for (int i = 0; i < num_threads; i++) {
    GC_WORKER_LOG("Initializing worker %d\n", i);
    gc_thread_pool.workers[i].id = i;
    atomic_init(&gc_thread_pool.workers[i].done, false);

#ifdef DEBUG_GC_WORKER_STATS
    // Initialize worker stats
    atomic_init(&gc_thread_pool.workers[i].stats.objects_marked, 0);
    atomic_init(&gc_thread_pool.workers[i].stats.objects_freed, 0);
    gc_thread_pool.workers[i].stats.work_steal_attempts = 0;
    gc_thread_pool.workers[i].stats.successful_steals   = 0;
#endif

    gc_thread_pool.workers[i].deque = ws_deque_init(GC_DEQUE_INITIAL_CAPACITY);
    if (!gc_thread_pool.workers[i].deque) {
      INTERNAL_ERROR("Failed to initialize work-stealing deque for worker %d", i);
      gc_shutdown_thread_pool();
      exit(SLANG_EXIT_MEMORY_ERROR);
    }
  }

  // Create worker threads, main thread is worker 0, so we skip that
  for (int i = 1; i < num_threads; i++) {
    int result = pthread_create(&gc_thread_pool.workers[i].thread, NULL, gc_worker, &gc_thread_pool.workers[i]);
    if (result != 0) {
      INTERNAL_ERROR("Failed to create worker thread %d: %s", i, strerror(result));
      gc_shutdown_thread_pool();
      exit(SLANG_EXIT_SW_ERROR);
    }
    prioritize_thread(gc_thread_pool.workers[i].thread, i);
  }
}

void gc_shutdown_thread_pool() {
  if (!gc_thread_pool.workers) {
    return;
  }

  atomic_store(&gc_thread_pool.shutdown, true);
  atomic_store(&gc_thread_pool.should_work, true);

  // Wake up all workers so they can see the shutdown flag
  SetEvent(gc_thread_pool.work_event);

  // Join only threads 1 onwards (not worker[0] which is main thread)
  for (int i = 1; i < gc_thread_pool.worker_count; i++) {
    pthread_join(gc_thread_pool.workers[i].thread, NULL);
  }

  // Free sync resources
  CloseHandle(gc_thread_pool.work_event);

  // Free all deques including worker[0]'s
  for (int i = 0; i < gc_thread_pool.worker_count; i++) {
    ws_deque_free(gc_thread_pool.workers[i].deque);
  }

  _aligned_free(gc_thread_pool.workers);
}
