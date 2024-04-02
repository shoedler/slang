#include <windows.h>
#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

BUILTIN_DECLARE_FN(now);

void register_builtin_perf_module() {
  ObjInstance* perf_module = make_module(NULL, "Perf");
  define_obj(&vm.modules, "Perf", (Obj*)perf_module);

  BUILTIN_REGISTER_FN(perf_module, now, 0);
}

// Native high-precision clock function. Only works on Windows.
BUILTIN_FN_DOC(
    /* Fn Name     */ now,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "High-precision clock function. Returns the current execution time in seconds.");
BUILTIN_FN_IMPL(now) {
  UNUSED(argc);
  UNUSED(argv);
  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);
  return NUMBER_VAL((double)now.QuadPart / frequency.QuadPart);
}