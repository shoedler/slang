#include "sys.h"
#include <stdio.h>
#include "assert.h"
#include "common.h"

double get_time() {
  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);

  return (double)now.QuadPart / frequency.QuadPart;
}

static void configure_thread(HANDLE thread, int priority, size_t core) {
  if (!SetThreadAffinityMask(thread, (1ULL) << core)) {
    DEBUG_WARNING("Failed to set thread affinity for thread %p to core %zu. Error: %ld", thread, core, GetLastError());
  }
  if (!SetThreadPriority(thread, priority)) {
    DEBUG_WARNING("Failed to set thread priority for thread %p to %d. Error: %ld", thread, priority, GetLastError());
  }
}

void prioritize_thread(HANDLE thread, int core) {
  configure_thread(thread, THREAD_PRIORITY_ABOVE_NORMAL, core);
}

void prioritize_main_thread() {
  configure_thread(GetCurrentThread(), THREAD_PRIORITY_HIGHEST, 0);
}

size_t get_cpu_core_count() {
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
}
