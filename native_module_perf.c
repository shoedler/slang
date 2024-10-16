#include <windows.h>
#include "builtin.h"
#include "common.h"
#include "file.h"
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

static double get_time() {
  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);

  return (double)now.QuadPart / frequency.QuadPart;
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

  return float_value(get_time() - (double)argv[1].as.float_);
}
