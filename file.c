#include "file.h"

char* base(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot get base of NULL path");
    exit(74);
  }

  size_t len = strlen(path);
  if (len == 0) {
    return path;
  }

  char* base = (char*)malloc(len + 1);
  if (base == NULL) {
    INTERNAL_ERROR("Not enough memory to get base of \"%s\"", path);
    exit(74);
  }

  strcpy(base, path);

  // Remove trailing slashes
  while (len > 0 && (base[len - 1] == '/' || base[len - 1] == '\\')) {
    base[--len] = '\0';
  }

  // Find the last slash
  while (len > 0 && base[len - 1] != '/' && base[len - 1] != '\\') {
    len--;
  }

  if (len == 0) {
    base[0] = '.';
    base[1] = '\0';
  } else {
    base[len] = '\0';
  }

  return base;
}

char* read_file(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot open NULL path \"%s\"", path);
    exit(74);
  }

  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    INTERNAL_ERROR("Could not open file \"%s\"", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    INTERNAL_ERROR("Not enough memory to read \"%s\"\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    INTERNAL_ERROR("Could not read file \"%s\"\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}