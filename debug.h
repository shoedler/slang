#ifndef debug_h
#define debug_h

#include "chunk.h"

// Disassembles the given chunk.
// This lists the compiled bytecode instructions in a human-readable format.
void debug_disassemble_chunk(Chunk* chunk, const char* name);

// Disassembles the given instruction by
// printing the instruction info.
int debug_disassemble_instruction(Chunk* chunk, int offset);

#endif
