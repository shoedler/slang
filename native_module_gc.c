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

void native_register_gc_module() {
  ObjObject* gc_module = vm_make_module(NULL, STR(MODULE_NAME));
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
  hashtable_init(&fields);

#define ALLOCD bytes_allocated
#define NEXTGC next_gc
#define PREVGC prev_gc_freed

  VM_SET_FLAG(VM_FLAG_PAUSE_GC);

  hashtable_set(&fields, str_value(copy_string(STR(ALLOCD), STR_LEN(STR(ALLOCD)))), int_value(vm.ALLOCD));
  hashtable_set(&fields, str_value(copy_string(STR(NEXTGC), STR_LEN(STR(NEXTGC)))), int_value(vm.NEXTGC));
  hashtable_set(&fields, str_value(copy_string(STR(PREVGC), STR_LEN(STR(PREVGC)))), int_value(vm.PREVGC));

#undef ALLOCD
#undef NEXTGC
#undef PREVGC

  ObjObject* stats = take_object(&fields);
  Value stats_obj  = instance_value(stats);

  VM_CLEAR_FLAG(VM_FLAG_PAUSE_GC);

  return stats_obj;
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
