#include "file.h"

char* append_slang_extension(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot append extension to NULL path");
    exit(74);
  }

  // Maybe the path already has the extension, in that case, return the same path
  size_t path_len = strlen(path);
  if (path_len >= SLANG_EXTENSION_LEN &&
      strcmp(path + path_len - SLANG_EXTENSION_LEN, SLANG_EXTENSION) == 0) {
    char* new_path = (char*)malloc(path_len + 1);
    if (new_path == NULL) {
      INTERNAL_ERROR("Not enough memory to append extension to \"%s\"", path);
      exit(74);
    }

    strcpy(new_path, path);
    return new_path;
  }

  // If not, we need to append the extension
  size_t len     = strlen(path);
  char* new_path = (char*)malloc(len + SLANG_EXTENSION_LEN + 1);  // +1 for the null terminator
  if (new_path == NULL) {
    INTERNAL_ERROR("Not enough memory to append extension to \"%s\"", path);
    exit(74);
  }

  strcpy(new_path, path);
  strcat(new_path, SLANG_EXTENSION);

  return new_path;
}

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

bool file_exists(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot check existence of NULL path");
    exit(74);
  }

  FILE* file = fopen(path, "r");
  if (file != NULL) {
    fclose(file);
    return true;
  }

  return false;
}

char* join_path(const char* path_a, const char* path_b) {
  if (path_a == NULL || path_b == NULL) {
    return NULL;  // Safety check
  }

  size_t path_a_len   = strlen(path_a);
  size_t path_b_len   = strlen(path_b);
  size_t total_length = path_a_len + path_b_len + 2;  // +2 for possible slash and null terminator

  // Allocate memory for the new path
  char* new_path = (char*)malloc(total_length);
  if (new_path == NULL) {
    return NULL;  // Failed to allocate memory
  }

  // Copy first path
  strcpy(new_path, path_a);

  // Check if path_a already ends with a separator
  int ends_with_sep = (path_a[path_a_len - 1] == '/' || path_a[path_a_len - 1] == '\\');
  if (!ends_with_sep) {
    // Add a directory separator
#ifdef _WIN32
    strcat(new_path, "\\");
#else
    strcat(new_path, "/");
#endif
  }

  // Concatenate second path
  strcat(new_path, path_b);

  return new_path;
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