#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static void repl() {
  // TODO (misc): Use linenoise for REPLing
  char line[1024];
  for (;;) {
    printf("slang > ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line, false /* no new local scope */);
  }
}

void usage() {
  printf("Usage: slang <args>\n");
  printf("  run  <path> Run script at <path>\n");
  printf("  repl        Run REPL\n");
  exit(64);
}

int main(int argc, char* argv[]) {
  init_vm();
  run_file("C:\\Projects\\slang\\sample.sl", true);
  free_vm();

  // Print args
  if (argc == 2 && strcmp(argv[1], "repl") == 0) {
    init_vm();
    repl();
    free_vm();
  } else if (argc == 3 && strcmp(argv[1], "run") == 0) {
    init_vm();
    run_file(argv[2], true /* is module */);
    free_vm();
  } else {
    usage();
  }

  return 0;
}
