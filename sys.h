#ifndef SYS_H
#define SYS_H

#include <stddef.h>
#include <windows.h>

// Sets the main thread's priority to an above normal level.
void prioritize_main_thread();

// Sets a [thread] to an above normal priority level and sets its affinity to [core].
void prioritize_thread(HANDLE thread, int core);

// Returns the number of CPU cores on the system.
size_t get_cpu_core_count();

// Returns a high-resolution timestamp in seconds.
double get_time();

#endif  // SYS_H
