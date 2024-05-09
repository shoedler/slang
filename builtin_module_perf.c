#include <windows.h>
#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

BUILTIN_DECLARE_FN(now);

#define MODULE_NAME Perf

void register_builtin_perf_module() {
  ObjObject* perf_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(perf_module));

  BUILTIN_REGISTER_FN(perf_module, now, 0);
}

/**
 * MODULE_NAME.now() -> TYPENAME_FLOAT
 * @brief High-precision clock function. Returns the current execution time in seconds.
 */
BUILTIN_FN_IMPL(now) {
  UNUSED(argc);
  UNUSED(argv);

  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);

  return float_value((double)now.QuadPart / frequency.QuadPart);
}