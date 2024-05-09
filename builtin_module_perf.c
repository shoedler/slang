#include <windows.h>
#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

static Value native_perf_now(int argc, Value argv[]);

#define MODULE_NAME Perf

void register_native_perf_module() {
  ObjObject* perf_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(perf_module));

  define_native(&perf_module->fields, "now", native_perf_now, 0);
}

/**
 * MODULE_NAME.now() -> TYPENAME_FLOAT
 * @brief High-precision clock function. Returns the current execution time in seconds.
 */
static Value native_perf_now(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);

  return float_value((double)now.QuadPart / frequency.QuadPart);
}