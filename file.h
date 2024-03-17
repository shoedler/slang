#ifndef file_h
#define file_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

// Function to read a file. Returns a pointer to the file's content.
// The caller is responsible for freeing the memory.
char* read_file(const char* path);

#endif
