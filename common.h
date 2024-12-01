#ifndef common_h
#define common_h

#include <mimalloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//
// Debugging/Logging flags
//

// Scanning & Compilation
// #define DEBUG_PRINT_TOKENS  // Print the tokens the scanner produces
// #define DEBUG_PRINT_CODE  // Print all compiled bytecode chunks

// Virtual Machine
// #define DEBUG_TRACE_EXECUTION  // Print the execution of the Vm, including stack traces. Also checks for leaked error states.

// Garbage Collection
// #define DEBUG_GC_PHASE_TIMES   // Log the time it takes for each phase of the Gc
// #define DEBUG_GC_WORKER_STATS  // Log the statistics of the Gc worker
// #define DEBUG_GC_HEAP_STATS    // Log the heap statistics
// #define DEBUG_GC_WORKER        // Log the Gc worker's activity
// #define DEBUG_GC_SWEEP         // Log the Gc sweep phase

//
// Feature flags
//

// #define SLANG_ENABLE_COLOR_OUTPUT  // Enable ANSI-colors in terminal. Defined as default for release builds - see Makefile

//
// Constants
//

#define SLANG_VERSION "v0.0.2"

//
// Exit codes
//

typedef enum {
  SLANG_EXIT_SUCCESS       = 0,   // Successful termination
  SLANG_EXIT_FAILURE       = 1,   // General error
  SLANG_EXIT_COMPILE_ERROR = 2,   // Compilation error
  SLANG_EXIT_RUNTIME_ERROR = 3,   // Runtime error
  SLANG_EXIT_BAD_USAGE     = 64,  // Command line usage error
  SLANG_EXIT_MEMORY_ERROR  = 70,  // Memory related error
  SLANG_EXIT_IO_ERROR      = 74,  // Input/output error
  SLANG_EXIT_SW_ERROR      = 75   // General internal logic error
} SlangExitCode;

//
// Color output macros
//

#ifdef SLANG_ENABLE_COLOR_OUTPUT
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#else
#define ANSI_COLOR_RED ""
#define ANSI_COLOR_GREEN ""
#define ANSI_COLOR_YELLOW ""
#define ANSI_COLOR_BLUE ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_CYAN ""
#define ANSI_COLOR_RESET ""
#endif

#define ANSI_RED_STR(str) ANSI_COLOR_RED str ANSI_COLOR_RESET
#define ANSI_GREEN_STR(str) ANSI_COLOR_GREEN str ANSI_COLOR_RESET
#define ANSI_YELLOW_STR(str) ANSI_COLOR_YELLOW str ANSI_COLOR_RESET
#define ANSI_BLUE_STR(str) ANSI_COLOR_BLUE str ANSI_COLOR_RESET
#define ANSI_MAGENTA_STR(str) ANSI_COLOR_MAGENTA str ANSI_COLOR_RESET
#define ANSI_CYAN_STR(str) ANSI_COLOR_CYAN str ANSI_COLOR_RESET

//
// Utility macros
//

#define ___PRINT_INTERNAL_MSG_HEADER(type, ANSI_COLOR) \
  fprintf(stderr, ANSI_COLOR type ANSI_COLOR_RESET " at " ANSI_MAGENTA_STR("%s(%u): "), __FILE__, __LINE__);
#define INTERNAL_ERROR(format_literal, ...)                         \
  do {                                                              \
    ___PRINT_INTERNAL_MSG_HEADER("INTERNAL ERROR", ANSI_COLOR_RED); \
    fprintf(stderr, format_literal "\n", ##__VA_ARGS__);            \
  } while (0)

#ifdef _DEBUG
#define INTERNAL_WARN(format_literal, ...)                            \
  do {                                                                \
    ___PRINT_INTERNAL_MSG_HEADER("INTERNAL WARN", ANSI_COLOR_YELLOW); \
    fprintf(stderr, format_literal "\n", ##__VA_ARGS__);              \
  } while (0)
#else
#define INTERNAL_WARN(format_literal, ...)
#endif

#ifdef _DEBUG
#define INTERNAL_ASSERT(condition, format_literal, ...)                 \
  do {                                                                  \
    if (!(condition)) {                                                 \
      ___PRINT_INTERNAL_MSG_HEADER("ASSERTION FAILED", ANSI_COLOR_RED); \
      fprintf(stderr, format_literal "\n", ##__VA_ARGS__);              \
      exit(SLANG_EXIT_SW_ERROR);                                        \
    }                                                                   \
  } while (0)
#else
#define INTERNAL_ASSERT(condition, format_literal, ...)
#endif

// Internal stringification Macro - don't use directly, use STR
#define ___STRINGIFY(x) #x
// Stringification Macro, wraps the argument in quotes
#define STR(x) ___STRINGIFY(x)
#define STR_LEN(x) (sizeof(x) - 1)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Suppress unused parameter macro
#define UNUSED(x) (void)(x)

#define UINT8_COUNT (UINT8_MAX + 1)

//
// Mimalloc Overrides.
// This is pretty much a copy of mimalloc-override.h, but a few defines have been removed do to compilation issues.
//

// Standard C allocation
#define malloc(n) mi_malloc(n)
#define calloc(n, c) mi_calloc(n, c)
#define realloc(p, n) mi_realloc(p, n)
#define free(p) mi_free(p)

#define strdup(s) mi_strdup(s)
#define strndup(s, n) mi_strndup(s, n)
#define realpath(f, n) mi_realpath(f, n)

// Microsoft extensions
#define _expand(p, n) mi_expand(p, n)
#define _recalloc(p, n, c) mi_recalloc(p, n, c)
// #define _msize(p) mi_usable_size(p)

#define _strdup(s) mi_strdup(s)
#define _strndup(s, n) mi_strndup(s, n)
#define _mbsdup(s) mi_mbsdup(s)
#define _dupenv_s(b, n, v) mi_dupenv_s(b, n, v)
// #define _wcsdup(s) (wchar_t*)mi_wcsdup((const unsigned short*)(s))
// #define _wdupenv_s(b,n,v)       mi_wdupenv_s((unsigned short*)(b),n,(const unsigned short*)(v))

// Various Posix and Unix variants
#define reallocf(p, n) mi_reallocf(p, n)
#define malloc_size(p) mi_usable_size(p)
#define malloc_usable_size(p) mi_usable_size(p)
#define malloc_good_size(sz) mi_malloc_good_size(sz)
#define cfree(p) mi_free(p)
#define valloc(n) mi_valloc(n)
#define pvalloc(n) mi_pvalloc(n)
#define reallocarray(p, s, n) mi_reallocarray(p, s, n)
#define reallocarr(p, s, n) mi_reallocarr(p, s, n)
#define memalign(a, n) mi_memalign(a, n)
#define aligned_alloc(a, n) mi_aligned_alloc(a, n)
#define posix_memalign(p, a, n) mi_posix_memalign(p, a, n)
#define _posix_memalign(p, a, n) mi_posix_memalign(p, a, n)

// Microsoft aligned variants
#define _aligned_malloc(n, a) mi_malloc_aligned(n, a)
#define _aligned_realloc(p, n, a) mi_realloc_aligned(p, n, a)
#define _aligned_recalloc(p, s, n, a) mi_aligned_recalloc(p, s, n, a)
#define _aligned_free(p) mi_free(p)
#define _aligned_offset_malloc(n, a, o) mi_malloc_aligned_at(n, a, o)
#define _aligned_offset_realloc(p, n, a, o) mi_realloc_aligned_at(p, n, a, o)
#define _aligned_offset_recalloc(p, s, n, a, o) mi_recalloc_aligned_at(p, s, n, a, o)
// #define _aligned_msize(p, a, o) mi_usable_size(p)

//
// Configuration checks
//

#ifndef _WIN32
#error "Only Windows is supported."
#endif

#endif
