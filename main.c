#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vm.h"

static void repl() {
  init_vm();

  // TODO (misc): Use linenoise for REPLing
  char line[1024];
  for (;;) {
    printf("slang > ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line);
  }

  free_vm();
}

static char* read_file(const wchar_t* path) {
  if (path == NULL) {
    WINTERNAL_ERROR(L"Cannot open NULL path \"%s\"", path);
    exit(74);
  }

  FILE* file = _wfopen(path, L"rb");
  if (file == NULL) {
    WINTERNAL_ERROR(L"Could not open file \"%s\"", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    WINTERNAL_ERROR(L"Not enough memory to read \"%s\"\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    WINTERNAL_ERROR(L"Could not read file \"%s\"\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

static void run_file(const wchar_t* path) {
  init_vm();

  char* source = read_file(path);
  InterpretResult result = interpret(source);
  free(source);

  switch (result) {
    case INTERPRET_OK:
      break;
    case INTERPRET_COMPILE_ERROR:
      exit(65);
      break;
    case INTERPRET_RUNTIME_ERROR:
      exit(70);
      break;
    default:
      printf(ANSI_RED_STR("INTERNAL ERROR\n"));
      INTERNAL_ERROR("Unhandled InterpretResult");
      break;
  }

  free_vm();
}

void usage() {
  printf("Usage: slang <args>\n");
  printf("  run  <path> Run script at <path>\n");
  printf("  repl        Run REPL\n");
  exit(64);
}

int wmain(int argc, wchar_t* argv[]) {
  if (argc == 2 && wcscmp(argv[1], L"repl") == 0) {
    repl();
  } else if (argc == 3 && wcscmp(argv[1], L"run") == 0) {
    run_file(argv[2]);
  } else {
    usage();
  }

  return 0;
}
