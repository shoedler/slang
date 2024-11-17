#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "vm.h"

static void repl() {
  // TODO (misc): Use linenoise for REPLing
  char line[1024];

  // Acquire the cwd to use as the module name
  char cwd[1024];
  if (_getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stderr, "Failed to get cwd\n");
    exit(SLANG_EXIT_IO_ERROR);
  }

  // Create a module initially to act as our toplevel module.
  start_module(cwd, "repl");

  for (;;) {
    printf("slang > ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line, NULL, NULL);
  }
}

void usage() {
  printf("Usage: slang <args>\n");
  printf("  run  <path> Run script at <path>\n");
  printf("  repl        Run REPL\n");
  printf("  --version   Print version\n");
  exit(SLANG_EXIT_BAD_USAGE);
}

int main(int argc, char* argv[]) {
  // Print args
  if (argc == 2 && strcmp(argv[1], "repl") == 0) {
    init_vm();
    repl();
    free_vm();
  } else if (argc == 3 && strcmp(argv[1], "run") == 0) {
    init_vm();
    run_file(argv[2], "main");
    free_vm();
  } else if (argc == 2 && strcmp(argv[1], "--version") == 0) {
    printf("slang %s\n", SLANG_VERSION);
  } else {
    usage();
  }

  exit(SLANG_EXIT_SUCCESS);
}
