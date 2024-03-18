#ifndef file_h
#define file_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define SLANG_EXTENSION ".sl"
#define SLANG_EXTENSION_LEN sizeof(SLANG_EXTENSION) - 1

// Checks if a file exists.
bool file_exists(const char* path);

// Function to read a file. Returns a pointer to the file's content.
// The caller is responsible for freeing the returned string.
char* read_file(const char* path);

// Joins two paths together.
// The caller is responsible for freeing the returned string.
char* join_path(const char* path_a, const char* path_b);

// Returns the directory of a filepath, or the upper directory if the path is a directory.
// The caller is responsible for freeing the returned string.
char* base(const char* path);

// Appends the slang file extension to the provided string. If the string already has the extension, it
// returns a copy of the input string. The caller is responsible for freeing the returned string.
char* append_slang_extension(const char* path);

#endif
