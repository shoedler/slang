#ifndef common_h
#define common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Debug feature flags

// #define DEBUG_PRINT_TOKENS  // Print the tokens the scanner produces
// #define DEBUG_PRINT_CODE  // Print all compiled bytecode chunks
// #define DEBUG_TRACE_EXECUTION  // Print the execution of the Vm, including stack traces.

#define DEBUG_STRESS_GC  // Force-run the Gc after every allocation
// #define DEBUG_LOG_GC              // Log the Gc's activity
// #define DEBUG_LOG_GC_FREE         // Log what objects are being freed by the Gc
// #define DEBUG_LOG_GC_ALLOCATIONS  // Log what objects are being allocated.

// Feature flags

// #define ENABLE_COLOR_OUTPUT  // Enable ANSI-colored output in terminal

// Exit codes

#define EXIT_SUCCESS 0  // Successful termination
#define EXIT_FAILURE 1  // General error

#define ECOMPILE_ERROR 2  // Compilation error
#define ERUNTIME_ERROR 3  // Runtime error

#define EBAD_USAGE 64  // Command line usage error
#define EMEM_ERROR 70  // Memory related error
#define EIO_ERROR 74   // Input/output error
#define ESW_ERROR 75   // General internal logic error

// Color output macros

#ifdef ENABLE_COLOR_OUTPUT
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

// Utility macros

#define ___PRINT_ERROR_HEADER(type) fprintf(stderr, ANSI_RED_STR(type) " at " ANSI_YELLOW_STR("%s(%u): "), __FILE__, __LINE__);
#define INTERNAL_ERROR(format_literal, ...)              \
  do {                                                   \
    ___PRINT_ERROR_HEADER("INTERNAL ERROR");             \
    fprintf(stderr, format_literal "\n", ##__VA_ARGS__); \
  } while (0)

// Stringification Macro - don't use directly, use STR
#define ___STRINGIFY(x) #x
// Stringification Macro, wraps the argument in quotes
#define STR(x) ___STRINGIFY(x)
#define STR_LEN(x) (sizeof(x) - 1)

// Suppress unused parameter macro
#define UNUSED(x) (void)(x)

#define UINT8_COUNT (UINT8_MAX + 1)

#endif
