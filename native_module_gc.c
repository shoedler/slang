#include <stddef.h>
#include <string.h>
#include "common.h"
#include "hashtable.h"
#include "memory.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static Value native_gc_collect(int argc, Value argv[]);
static Value native_gc_stats(int argc, Value argv[]);
static Value native_gc_stress(int argc, Value argv[]);

#define MODULE_NAME Gc

void register_native_gc_module() {
  ObjObject* gc_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(gc_module));

  define_native(&gc_module->fields, "collect", native_gc_collect, 0);
  define_native(&gc_module->fields, "stats", native_gc_stats, 0);
  define_native(&gc_module->fields, "stress", native_gc_stress, 1);
}

/**
 * MODULE_NAME.collect() -> TYPENAME_INT
 * @brief Executes a garbage collection cycle. Returns the number of bytes freed during the cycle.
 */
static Value native_gc_collect(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  collect_garbage();

  return int_value(vm.prev_gc_freed);
}

/**
 * MODULE_NAME.stats() -> TYPENAME_OBJ
 * @brief Returns a TYPENAME_OBJ containing the current garbage collection statistics.
 * The object contains the following fields:
 * - "bytes_allocated": The total number of bytes allocated.
 * - "next_gc":         The number of bytes at which the next garbage collection cycle will be triggered.
 * - "prev_gc_freed":   The number of bytes freed during the previous garbage collection cycle.
 */
static Value native_gc_stats(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  HashTable fields;
  init_hashtable(&fields);

  hashtable_set(&fields, str_value(copy_string("bytes_allocated", (int)strlen("bytes_allocated"))),
                int_value(vm.bytes_allocated));
  hashtable_set(&fields, str_value(copy_string("next_gc", (int)strlen("next_gc"))), int_value(vm.next_gc));
  hashtable_set(&fields, str_value(copy_string("prev_gc_freed", (int)strlen("prev_gc_freed"))), int_value(vm.prev_gc_freed));

  ObjObject* stats = take_object(&fields);

  return instance_value(stats);
}

/**
 * MODULE_NAME.stress(force: TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Toggles the stress-garbage-collector flag. If [force] is true, the garbage collector will be forced to run on every
 * allocation. If [force] is false, the garbage collector will run as usual. Returns the value of the flag before the change.
 */
static Value native_gc_stress(int argc, Value argv[]) {
  UNUSED(argc);

  NATIVE_CHECK_ARG_AT(1, vm.bool_class)

  bool old_value = VM_HAS_FLAG(VM_FLAG_STRESS_GC);
  if (argv[0].as.boolean) {
    VM_SET_FLAG(VM_FLAG_STRESS_GC);
  } else {
    VM_CLEAR_FLAG(VM_FLAG_STRESS_GC);
  }

  return bool_value(old_value);
}
