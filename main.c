#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "vm.h"

#define CMD_REPL "repl"
#define CMD_RUN "run"
#define CMD_RUN_OLD "run-old"
#define CMD___VERSION "--version"

#define OPT_STRESS_GC "--stress-gc"
#define OPT_NO_WARN "--no-warn"  // Enable warnings during compilation

typedef struct {
  char** argv;
  int argc;
} Options;

static Options opts;

static void init_options(int argc, char* argv[]) {
  opts.argv = argv + 1;  // Skip program name
  opts.argc = argc - 1;
}

static bool consume_option(const char* name) {
  for (int i = 0; i < opts.argc; i++) {
    if (strcmp(opts.argv[i], name) == 0) {
      memmove(&opts.argv[i], &opts.argv[i + 1], (opts.argc - i - 1) * sizeof(char*));  // Shift the rest of the args left
      opts.argc--;
      return true;
    }
  }
  return false;
}

static char* pop_option(void) {
  if (opts.argc == 0) {
    return NULL;
  }
  return opts.argv[--opts.argc];
}

static bool validate_options(void) {
  return opts.argc == 0;
}

static void usage(const char* error) {
  if (error != NULL) {
    fprintf(stderr, "Error: %s\n", error);
    printf("\n");
  }
  printf("Usage: slang <args>\n");
  printf("  " CMD_RUN "  <options> <path> Run script at <path>\n");
  printf("  " CMD_REPL " <options>        Run REPL\n");
  printf("  " CMD___VERSION "             Print version\n");
  printf("\n");
  printf("  <options>:\n");
  printf("    " OPT_NO_WARN "                 Disable warnings during compilation\n");
  printf("    " OPT_STRESS_GC "               Enable GC stress testing\n");
}

static void configure_vm() {
  vm_init();
  bool gc_stress = consume_option(OPT_STRESS_GC);
  if (gc_stress) {
    INTERNAL_WARN("GC stress testing enabled, can be disabled during runtime using the Gc module.");
    VM_SET_FLAG(VM_FLAG_STRESS_GC);
  }
}

static SlangExitCode repl() {
  configure_vm();

  if (!validate_options()) {
    usage("Unknown options for " CMD_REPL);
    exit(SLANG_EXIT_BAD_USAGE);
  }

  // TODO (misc): Use linenoise for REPLing
  char line[1024];

  // Acquire the cwd to use as the module name
  char cwd[1024];
  if (_getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stderr, "Failed to get cwd\n");
    exit(SLANG_EXIT_IO_ERROR);
  }

  // Create a module initially to act as our toplevel.
  vm_start_module(cwd, CMD_REPL);

  for (;;) {
    printf(ANSI_GRAY_STR("slang > "));

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    vm_interpret(line, NULL, true /* disable_warnings */);
  }

  vm_free();
  return SLANG_EXIT_SUCCESS;
}

static SlangExitCode run_old() {
  configure_vm();

  const char* path = pop_option();
  if (path == NULL) {
    usage("No path provided for " CMD_RUN_OLD);
    exit(SLANG_EXIT_BAD_USAGE);
  }
  if (!validate_options()) {
    usage("Unknown options for " CMD_RUN_OLD);
    exit(SLANG_EXIT_BAD_USAGE);
  }

  vm_run_file_old(path, "main");

  SlangExitCode exit_code = SLANG_EXIT_SUCCESS;
  if (VM_HAS_FLAG(VM_FLAG_HAD_COMPILE_ERROR)) {
    exit_code = SLANG_EXIT_COMPILE_ERROR;
  } else if (VM_HAS_FLAG(VM_FLAG_HAD_UNCAUGHT_RUNTIME_ERROR)) {
    exit_code = SLANG_EXIT_RUNTIME_ERROR;
  }
  vm_free();

  return exit_code;
}

static SlangExitCode run() {
  configure_vm();

  bool disable_warnings = consume_option(OPT_NO_WARN);

  const char* path = pop_option();
  if (path == NULL) {
    usage("No path provided for " CMD_RUN);
    return SLANG_EXIT_BAD_USAGE;
  }
  if (!validate_options()) {
    usage("Unknown options for " CMD_RUN);
    return SLANG_EXIT_BAD_USAGE;
  }

  return vm_run_entry_point(path, disable_warnings);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage("No arguments provided");
    exit(SLANG_EXIT_BAD_USAGE);
  }

  init_options(argc, argv);

  SlangExitCode code;

  if (consume_option(CMD_REPL)) {
    code = repl();
  } else if (consume_option(CMD_RUN)) {
    code = run();
  } else if (consume_option(CMD_RUN_OLD)) {
    code = run_old();
  } else if (consume_option("--version")) {
    printf("slang %s\n", SLANG_VERSION);
    code = SLANG_EXIT_SUCCESS;
  } else {
    usage("Invalid command");
    code = SLANG_EXIT_BAD_USAGE;
  }

  exit(code);
}
