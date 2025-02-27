#ifndef file_h
#define file_h

#include "common.h"

#define SLANG_EXTENSION ".sl"
#define SLANG_EXTENSION_LEN (STR_LEN(SLANG_EXTENSION))

// Checks if a file exists.
bool file_exists(const char* path);

// Function to read a file. Returns a pointer to the file's content.
// The caller is responsible for freeing the returned string.
char* file_read(const char* path);

// Function to read a file. Same as file_read, but returns NULL if the file does not exist.
// The caller is responsible for freeing the returned string.
char* file_read_safe(const char* path);

// Write a file. If the file already exists, it will be overwritten.
// If it does not exist, it will be created.
// Returns true on success, false on failure.
bool file_write(const char* path, const char* content);

// Joins two paths together.
// The caller is responsible for freeing the returned string.
char* file_join_path(const char* path_a, const char* path_b);

// Returns the directory of a filepath, or the upper directory if the path is a directory.
// The caller is responsible for freeing the returned string.
char* file_base(const char* path);

// Appends the slang file extension to the provided string. If the string already has the extension, it
// returns a copy of the input string. The caller is responsible for freeing the returned string.
char* file_ensure_slang_extension(const char* path);

// Resolves an import-path to an absolute file-path, based on the current working directory. Returns the absolute path to the
// module. The caller is responsible for freeing the memory.
// - [module_name] can be NULL, if the module is imported solely by [module_path] (relative or absolute).
// - [module_path] can be NULL, if the module is expected to be in the same directory as the importing module and the file is
// named after [module_name].
char* file_resolve_module_path(const char* cwd, const char* module_name, const char* module_path);

#endif
