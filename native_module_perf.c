#include <stddef.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "sys.h"
#include "value.h"
#include "vm.h"

static Value native_perf_now(int argc, Value argv[]);
static Value native_perf_since(int argc, Value argv[]);

#define MODULE_NAME Perf

void register_native_perf_module() {
  ObjObject* perf_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(perf_module));

  define_native(&perf_module->fields, "now", native_perf_now, 0);
  define_native(&perf_module->fields, "since", native_perf_since, 1);
}

/**
 * MODULE_NAME.now() -> TYPENAME_FLOAT
 * @brief High-precision clock function. Returns the current execution time in seconds.
 */
static Value native_perf_now(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  return float_value(get_time());
}

/**
 * MODULE_NAME.since(start: TYPENAME_FLOAT) -> TYPENAME_FLOAT
 * @brief High-precision clock function. Returns the time elapsed since the start time in seconds.
 */
static Value native_perf_since(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_ARG_AT(1, vm.float_class);

  return float_value(get_time() - argv[1].as.float_);
}
