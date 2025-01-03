#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

char* file_ensure_slang_extension(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot append extension to NULL path");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  // Maybe the path already has the extension. In that case, return the same path
  size_t path_len = strlen(path);
  if (path_len >= SLANG_EXTENSION_LEN && strcmp(path + path_len - SLANG_EXTENSION_LEN, SLANG_EXTENSION) == 0) {
    return strdup(path);
  }

  // If not, we need to append the extension
  size_t len     = strlen(path);
  char* new_path = (char*)malloc(len + SLANG_EXTENSION_LEN + 1);  // +1 for the null terminator
  if (new_path == NULL) {
    INTERNAL_ERROR("Not enough memory to append extension to \"%s\"", path);
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  strcpy(new_path, path);
  strcat(new_path, SLANG_EXTENSION);

  return new_path;
}

char* file_base(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot get base of NULL path");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  size_t len = strlen(path);
  if (len == 0) {
    return strdup(path);
  }

  char* base = (char*)malloc(len + 1);
  if (base == NULL) {
    INTERNAL_ERROR("Not enough memory to get base of \"%s\"", path);
    exit(SLANG_EXIT_MEMORY_ERROR);
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
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  FILE* file = fopen(path, "r");
  if (file != NULL) {
    fclose(file);
    return true;
  }

  return false;
}

// Cleans a path (windows-style) by replacing all '/' with '\'. Returns a new string with the normalized
// path. The caller is responsible for freeing the returned string.
char* clean_path(const char* path, bool no_prefixed_separators) {
  if (path == NULL || *path == '\0') {
    return strdup("");  // Return an empty string for NULL or empty input
  }

  size_t len   = strlen(path);
  char* result = (char*)malloc(len + 1);  // +1 for the null terminator
  if (result == NULL) {
    return NULL;
  }

  // Normalize slashes and remove redundant slashes
  int write_index = 0;
  for (size_t i = 0; i < len; ++i) {
    if (path[i] == '/' || path[i] == '\\') {
      // Skip slashes at the beginning
      if (write_index != 0 || !no_prefixed_separators) {
        result[write_index++] = '\\';  // Use backslash for Windows
      }

      // Skip additional slashes
      while (i + 1 < len && (path[i + 1] == '\\' || path[i + 1] == '/')) {
        ++i;
      }
    } else {
      result[write_index++] = path[i];
    }
  }
  result[write_index] = '\0';  // Null-terminate the result

  return result;
}

char* file_join_path(const char* path_a, const char* path_b) {
  char* clean_a    = clean_path(path_a, false);
  char* clean_b    = clean_path(path_b, true);
  size_t len_a     = strlen(clean_a);
  size_t total_len = len_a + strlen(clean_b) + 2;  // +2 for possible slash and null terminator

  char* joined_path = (char*)malloc(total_len);
  if (!joined_path) {
    free(clean_a);
    free(clean_b);
    return NULL;
  }

  strcpy(joined_path, clean_a);
  bool a_ends_with_slash   = len_a > 0 && clean_a[len_a - 1] == '\\';
  bool b_starts_with_slash = clean_b[0] == '\\';
  // Add a slash if necessary
  if (!a_ends_with_slash && !b_starts_with_slash) {
    strcat(joined_path, "\\");
  }
  // Remove a slash if necessary

  strcat(joined_path, clean_b);

  free(clean_a);
  free(clean_b);
  return joined_path;
}

// Internal function to read a file. Returns a pointer to the file's content.
// The caller is responsible for freeing the returned string.
// Either exits on error or returns NULL.
static char* internal_read_file(const char* path, bool exit_on_error) {
  if (path == NULL) {
    if (exit_on_error) {
      INTERNAL_ERROR("Cannot open NULL path.");
      exit(SLANG_EXIT_MEMORY_ERROR);
    } else {
      return NULL;
    }
  }

  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    if (exit_on_error) {
      INTERNAL_ERROR("Could not open file \"%s\"", path);
      exit(SLANG_EXIT_IO_ERROR);
    } else {
      return NULL;
    }
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    fclose(file);
    if (exit_on_error) {
      INTERNAL_ERROR("Not enough memory to read \"%s\"\n", path);
      exit(SLANG_EXIT_MEMORY_ERROR);
    } else {
      return NULL;
    }
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    fclose(file);
    if (exit_on_error) {
      INTERNAL_ERROR("Could not read file \"%s\"\n", path);
      exit(SLANG_EXIT_IO_ERROR);
    } else {
      return NULL;
    }
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

char* file_read(const char* path) {
  return internal_read_file(path, true);
}

char* file_read_safe(const char* path) {
  return internal_read_file(path, false);
}

// Write a file. If the file already exists, it will be overwritten.
// If it does not exist, it will be created.
// Returns true on success, false on failure.
bool file_write(const char* path, const char* content) {
  if (path == NULL || content == NULL) {
    return false;
  }

  FILE* file = fopen(path, "wb");
  if (file == NULL) {
    return false;
  }

  size_t content_len   = strlen(content);
  size_t bytes_written = fwrite(content, sizeof(char), content_len, file);
  fclose(file);

  return bytes_written == content_len;
}

char* file_resolve_module_path(const char* cwd, const char* module_name, const char* module_path) {
  if (module_path == NULL && module_name == NULL) {
    INTERNAL_ERROR("Cannot resolve module path. Both module name and path are NULL.");
    exit(SLANG_EXIT_MEMORY_ERROR);
  }

  char* absolute_file_path;

  // Either we have a module path, or we check the current working directory
  if (module_path == NULL) {
    // Just slap the module name + extension onto the cwd
    char* module_file_name = file_ensure_slang_extension(module_name);
    absolute_file_path     = file_join_path(cwd, module_file_name);
    free(module_file_name);
  } else {
    // It's probably a realtive path, we add the extension to the provided path and prepend the cwd
    char* module_path_ = file_ensure_slang_extension(module_path);
    absolute_file_path = file_join_path(cwd, module_path_);
    free(module_path_);

    if (!file_exists(absolute_file_path)) {
      // Clearly, it's not a relative path.
      free(absolute_file_path);

      // We assume it is an absolute path instead, which also has the extension already
      absolute_file_path = strdup(module_path);
    }
  }

  if (absolute_file_path == NULL) {
    INTERNAL_ERROR(
        "Could not produce a valid module path for module '%s'. Cwd is '%s', additional path is "
        "'%s'",
        module_name, cwd, module_path == NULL ? "NULL" : module_path);
    exit(SLANG_EXIT_IO_ERROR);
  }

  return absolute_file_path;
}
