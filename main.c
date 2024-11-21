#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "vm.h"

#define CMD_REPL "repl"
#define CMD_RUN "run"
#define CMD_VERSION "--version"
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
  printf("  " CMD_VERSION "             Print version\n");
  printf("\n");
  printf("  <options>:\n");
  printf("    --gc-stress         Enable GC stress testing\n");
}

static void configure_vm() {
  bool gc_stress = consume_option("--gc-stress");
  if (gc_stress) {
    INTERNAL_WARN("GC stress testing enabled, can be disabled during runtime using the Gc module.");
    VM_SET_FLAG(VM_FLAG_PAUSE_GC);
  }
}

static SlangExitCode repl() {
  init_vm();
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

  // Create a module initially to act as our toplevel module.
  start_module(cwd, CMD_REPL);

  for (;;) {
    printf("slang > ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line, NULL, NULL);
  }

  bool has_error = VM_HAS_FLAG(VM_FLAG_HAS_ERROR);
  free_vm();

  return has_error ? SLANG_EXIT_FAILURE : SLANG_EXIT_SUCCESS;
}

static SlangExitCode run() {
  init_vm();
  configure_vm();

  const char* path = pop_option();
  if (path == NULL) {
    usage("No path provided for " CMD_RUN);
    exit(SLANG_EXIT_BAD_USAGE);
  }
  if (!validate_options()) {
    usage("Unknown options for " CMD_RUN);
    exit(SLANG_EXIT_BAD_USAGE);
  }

  run_file(path, "main");

  bool has_error = VM_HAS_FLAG(VM_FLAG_HAS_ERROR);
  free_vm();

  return has_error ? SLANG_EXIT_FAILURE : SLANG_EXIT_SUCCESS;
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
  } else if (consume_option("--version")) {
    printf("slang %s\n", SLANG_VERSION);
    code = SLANG_EXIT_SUCCESS;
  } else {
    usage("Invalid command");
    code = SLANG_EXIT_BAD_USAGE;
  }

  exit(code);
}
