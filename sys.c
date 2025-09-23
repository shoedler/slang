#if defined(__linux__)
  #define _GNU_SOURCE  // For pthread_setaffinity_np and CPU_* macros
#endif

#include "sys.h"
#include <stdio.h>
#include <pthread.h>
#include "common.h"

#if SLANG_PLATFORM_WINDOWS
  #include <processthreadsapi.h>
  #include <profileapi.h>
  #include <sysinfoapi.h>
  #include <windows.h>
  #include <winnt.h>
#elif SLANG_PLATFORM_LINUX
  #include <unistd.h>
  #include <time.h>
  #include <sched.h>
  #include <sys/sysinfo.h>
#endif

double get_time() {
#if SLANG_PLATFORM_WINDOWS
  LARGE_INTEGER frequency;
  LARGE_INTEGER now;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&now);
  return (double)now.QuadPart / frequency.QuadPart;
#elif SLANG_PLATFORM_LINUX
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

#if SLANG_PLATFORM_WINDOWS
static void configure_thread(HANDLE thread, int priority, size_t core) {
  if (!SetThreadAffinityMask(thread, (1ULL) << core)) {
    INTERNAL_WARN("Failed to set thread affinity for thread %p to core %zu. Error: %ld", thread, core, GetLastError());
  }
  if (!SetThreadPriority(thread, priority)) {
    INTERNAL_WARN("Failed to set thread priority for thread %p to %d. Error: %ld", thread, priority, GetLastError());
  }
}
#endif

void prioritize_thread(pthread_t thread, int core) {
#if SLANG_PLATFORM_WINDOWS
  HANDLE handle = (HANDLE)pthread_gethandle(thread);
  configure_thread(handle, THREAD_PRIORITY_ABOVE_NORMAL, core);
#elif SLANG_PLATFORM_LINUX
  // Set thread affinity to specific core
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (result != 0) {
    INTERNAL_WARN("Failed to set thread affinity to core %d", core);
  }
  
  // Try to set higher priority (requires proper permissions)
  struct sched_param param;
  param.sched_priority = 1;  // Slightly higher than normal
  result = pthread_setschedparam(thread, SCHED_OTHER, &param);
  if (result != 0) {
    // Silently fail for priority (might not have permissions)
  }
#endif
}

void prioritize_main_thread() {
#if SLANG_PLATFORM_WINDOWS
  configure_thread(GetCurrentThread(), THREAD_PRIORITY_HIGHEST, 0);
#elif SLANG_PLATFORM_LINUX
  // Set main thread to core 0 with higher priority if possible
  prioritize_thread(pthread_self(), 0);
#endif
}

size_t get_cpu_core_count() {
#if SLANG_PLATFORM_WINDOWS
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
#elif SLANG_PLATFORM_LINUX
  return (size_t)get_nprocs();
#endif
}
