#ifndef file_h
#define file_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

// Function to read a file. Returns a pointer to the file's content.
// The caller is responsible for freeing the returned string.
char* read_file(const char* path);

// Returns the directory of a filepath, or the upper directory if the path is a directory.
// The caller is responsible for freeing the returned string.
char* base(const char* path);

#endif
