#ifndef chunk_h
#define chunk_h

#include <stdint.h>
#include "scanner.h"
#include "value.h"

typedef enum {
  /**
   * Pushes a constant from the constant pool onto the stack.
   * @note stack: `[...] -> [...][const]`
   * @note synopsis: `OP_CONSTANT, index`
   * @param index index into the constant pool
   */
  OP_CONSTANT,
  /**
   * Pushes a `nil` value onto the stack.
   * @note stack: `[...] -> [...][nil]`
   * @note synopsis: `OP_NIL`
   */
  OP_NIL,
  /**
   * Pushes a `true` value onto the stack.
   * @note stack: `[...] -> [...][true]`
   * @note synopsis: `OP_TRUE`
   */
  OP_TRUE,
  /**
   * Pushes a `false` value onto the stack.
   * @note stack: `[...] -> [...][false]`
   * @note synopsis: `OP_FALSE`
   */
  OP_FALSE,
  /**
   * Pops the top value from the stack.
   * @note stack: `[...][a] -> [...]`
   * @note synopsis: `OP_POP`
   */
  OP_POP,
  /**
   * Duplicates the value at the given index on the stack.
   * @note stack: `[...][a] -> [...][a][a]`
   * @note synopsis: `OP_DUPE, index`
   * @param index index into the vms' stack (0: top, 1: second from top, etc.)
   */
  OP_DUPE,
  /**
   * Gets a local variable and pushes it onto the stack.
   * @note stack: `[...] -> [...][value]`
   * @note synopsis: `OP_GET_LOCAL, slot`
   * @param slot index into the current frames' stack window (aka. slots) (0: top, 1: second from top, etc.)
   */
  OP_GET_LOCAL,
  /**
   * Gets a global variable and pushes it onto the stack.
   * @note stack: `[...] -> [...][value]`
   * @note synopsis: `OP_GET_GLOBAL, str_index`
   * @param str_index index into constant pool to get the name, which is then used to get [value] from the globals hashtable (or
   * the natives lookup table - subject to change though)
   */
  OP_GET_GLOBAL,
  /**
   * Gets an upvalue and pushes it onto the stack.
   * @note stack: `[...] -> [value]`
   * @note synopsis: `OP_GET_UPVALUE, slot`
   * @param slot index into the current frames' closures' upvalues (0: first, 1: second, etc.)
   */
  OP_GET_UPVALUE,
  /**
   * Defines the top value of the stack as a global variable and pops it.
   * @note stack: `[...][value] -> [...]`
   * @note synopsis: `OP_DEFINE_GLOBAL, str_index`
   * @param str_index index into constant pool to get the name, which is then used to set [value] in the globals hashtable.
   */
  OP_DEFINE_GLOBAL,
  /**
   * Sets a local variable to the top value of the stack and leaves it.
   * @note stack: `[...][value] -> [...][value]`
   * @note synopsis: `OP_SET_LOCAL, slot`
   * @param slot index into the current frames' stack window (aka. slots) (0: top, 1: second from top, etc.)
   */
  OP_SET_LOCAL,
  /**
   * Sets a global variable to the top value of the stack and leaves it.
   * @note stack: `[...][value] -> [...][value]`
   * @note synopsis: `OP_SET_GLOBAL, str_index`
   * @param str_index index into constant pool to get the name, which is then used to set [value] in the globals hashtable.
   */
  OP_SET_GLOBAL,
  /**
   * Sets an upvalue to the top value of the stack and leaves it.
   * @note stack: `[...][value] -> [...][value]`
   * @note synopsis: `OP_SET_UPVALUE, slot`
   * @param slot index into the current frames' closures' upvalues (0: first, 1: second, etc.)
   */
  OP_SET_UPVALUE,
  /**
   * Gets a subscript from the top two values on the stack and pushes the result. (Invokes `__get_subs` on the receiver)
   * @note stack: `[...][receiver][index] -> [...][result]`
   * @note synopsis: `OP_GET_SUBSCRIPT`
   */
  OP_GET_SUBSCRIPT,
  /**
   * Sets a subscript on the top three values on the stack and leaves the result. (Invokes `__set_subs` on the receiver)
   * @note stack: `[...][receiver][index][value] -> [...][result]`
   * @note synopsis: `OP_SET_SUBSCRIPT`
   */
  OP_SET_SUBSCRIPT,
  /**
   * Gets a property from the top value on the stack and pushes the result. (Invokes `__get_prop` on the receiver)
   * @note stack: `[...][receiver] -> [...][result]`
   * @note synopsis: `OP_GET_PROPERTY, str_index`
   * @param str_index index into constant pool to get the name, which is then used to get [result] from the receiver.
   */
  OP_GET_PROPERTY,
  /**
   * Sets a property on the 2nd-to-top value on the stack and leaves the result. (Invokes `__set_prop` on the receiver)
   * @note stack: `[...][receiver][value] -> [...][result]`
   * @note synopsis: `OP_SET_PROPERTY, str_index`
   * @param str_index index into constant pool to get the name, which is then used to set [value] in the receiver.
   */
  OP_SET_PROPERTY,
  /**
   * Gets a method from the top value on the stack, binds it to the 2nd-to-top value, and pushes the result.
   * @note stack: `[...][receiver] -> [...][result]`
   * @note synopsis: `OP_GET_METHOD, str_index`
   * @param str_index index into constant pool to get the name, which is then used to get the method from the receiver.
   */
  OP_GET_BASE_METHOD,
  /**
   * Compares the top two values on the stack for equality and pushes the result. (Invokes `__equals` on the values)
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_EQ`
   */
  OP_EQ,
  /**
   * Compares the top two values on the stack for inequality and pushes the result. (Invokes `__equals` on the values)
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_NEQ`
   */
  OP_NEQ,
  /**
   * Compares the top two values on the stack for greater-than and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_GT`
   */
  OP_GT,
  /**
   * Compares the top two values on the stack for less-than and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_LT`
   */
  OP_LT,
  /**
   * Compares the top two values on the stack for greater-than-or-equal and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_GTEQ`
   */
  OP_GTEQ,
  /**
   * Compares the top two values on the stack for less-than-or-equal and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_LTEQ`
   */
  OP_LTEQ,
  /**
   * Adds the top two values on the stack and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_ADD`
   */
  OP_ADD,
  /**
   * Subtracts the top two values on the stack and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_SUBTRACT`
   */
  OP_SUBTRACT,
  /**
   * Multiplies the top two values on the stack and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_MULTIPLY`
   */
  OP_MULTIPLY,
  /**
   * Divides the top two values on the stack and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_DIVIDE`
   */
  OP_DIVIDE,
  /**
   * Modulos the top two values on the stack and pushes the result.
   * @note stack: `[...][a][b] -> [...][result]`
   * @note synopsis: `OP_MODULO`
   */
  OP_MODULO,
  /**
   * Checks if the top value on the stack is falsy and pushes the result.
   * @note stack: `[...][a] -> [...][result]`
   * @note synopsis: `OP_NOT`
   */
  OP_NOT,
  /**
   * Negates the top value on the stack and pushes the result.
   * @note stack: `[...][a] -> [...][result]`
   * @note synopsis: `OP_NEGATE`
   */
  OP_NEGATE,
  /**
   * Prints the top value on the stack and pops it. (Invokes `__to_str` on the value)
   * @note stack: `[...][a] -> [...]`
   * @note synopsis: `OP_PRINT`
   */
  OP_PRINT,
  /**
   * Jumps to the instruction at the given offset (current ip + offset).
   * @note stack: `[...] -> [...]`
   * @note synopsis: `OP_JUMP, offset`
   * @param offset offset to jump to (from the current ip)
   */
  OP_JUMP,
  /**
   * Jumps to the instruction at the given offset if the top value on the stack is falsy and leaves it.
   * @note stack: `[...][a] -> [...][a]`
   * @note synopsis: `OP_JUMP_IF_FALSE, offset`
   * @param offset offset to jump to (from the current ip)
   */
  OP_JUMP_IF_FALSE,
  /**
   * Pushes a handler-value onto the stack (Consists of try-target and the offset from the start of the callframe to the try
   * block).
   * @note stack: `[...] -> [...][handler]`
   * @note synopsis: `OP_TRY, try_target`
   * @param try_target try-target
   */
  OP_TRY,
  /**
   * Loops back to the instruction at the given offset (current ip - offset).
   * @note stack: `[...] -> [...]`
   * @note synopsis: `OP_LOOP, offset`
   * @param offset offset to loop back to (from the current ip)
   */
  OP_LOOP,
  /**
   * Calls the callable at the top of the stack with the given number of arguments.
   * @note stack: `[...][callable][arg_0]...[arg_n] -> [...][result]` (in case of a native function)
   * @note stack: `[...][callable][arg_0]...[arg_n] -> [...][arg_0][arg_1]...[arg_n]` (in case of a managed function)
   * @note synopsis: `OP_CALL, arg_count`
   * @param arg_count number of arguments to pass to the callable
   */
  OP_CALL,
  /**
   * Invokes the callable at the top of the stack with the given number of arguments.
   * @note stack: `[...][receiver][arg_0]...[arg_n] -> [...][result]` (in case of a native function)
   * @note stack: `[...][receiver][arg_0]...[arg_n] -> [...][receiver][arg_0][arg_1]...[arg_n]` (in case of a managed function)
   * @note synopsis: `OP_INVOKE, str_index, arg_count`
   * @param str_index index into constant pool to get the name of the method to invoke
   * @param arg_count number of arguments to pass to the callable
   */
  OP_INVOKE,
  /**
   * Invokes the callable at the top of the stack with the given number of arguments and the base class.
   * @note stack: `[...][receiver][arg_0]...[arg_n][base] -> [...][result]` (in case of a native function)
   * @note stack: `[...][receiver][arg_0]...[arg_n][base] -> [...][receiver][arg_0]...[arg_n]` (in case of a managed function)
   * @note synopsis: `OP_BASE_INVOKE, str_index, arg_count`
   * @param str_index index into constant pool to get the name of the method to invoke
   * @param arg_count number of arguments to pass to the callable
   */
  OP_BASE_INVOKE,
  /**
   * Creates a closure from the function at the given index in the constant pool and pushes it.
   * @note stack: `[...] -> [...][closure]`
   * @note synopsis: `OP_CLOSURE, fn_index, (is_local, index) * upvalue_count`
   * @param fn_index index into constant pool to get the function
   * @param is_local whether the upvalue is local or not
   * @param index index into the current frames' upvalues (0: first, 1: second, etc.)
   */
  OP_CLOSURE,
  /**
   * Closes the topmost value on the stack and pops it.
   * @note stack: `[...][value] -> [...]`
   * @note synopsis: `OP_CLOSE_UPVALUE`
   */
  OP_CLOSE_UPVALUE,
  /**
   * Creates a sequence literal from the top `count` values on the stack and pushes the result.
   * @note stack: `[...][v_0][v_1]...[v_n] -> [...][seq]`
   * @note synopsis: `OP_SEQ_LITERAL, count`
   * @param count number of values to include in the sequence (from the top of the stack)
   */
  OP_SEQ_LITERAL,
  /**
   * Creates a tuple literal from the top `count` values on the stack and pushes the result.
   * @note stack: `[...][v_0][v_1]...[v_n] -> [...][tuple]`
   * @note synopsis: `OP_TUPLE_LITERAL, count`
   * @param count number of values to include in the tuple (from the top of the stack)
   */
  OP_TUPLE_LITERAL,
  /**
   * Creates an object literal from the top `count` * 2 values on the stack (kvp) and pushes the result.
   * @note stack: `[...][k_0][v_0][k_1][v_1]...[k_n][v_n] -> [...][object]`
   * @note synopsis: `OP_OBJECT_LITERAL, count`
   * @param count number of key-value pairs to include in the object (from the top of the stack)
   */
  OP_OBJECT_LITERAL,
  /**
   * Closes all upvalues in the current frames' slots and returns from the current function.
   * @note stack: `[...][fn] -> [...]`
   * @note synopsis: `OP_RETURN`
   */
  OP_RETURN,
  /**
   * Creates a new class and pushes it onto the stack.
   * @note stack: `[...] -> [...][class]`
   * @note synopsis: `OP_CLASS, str_index`
   * @param str_index index into constant pool to get the name of the class
   */
  OP_CLASS,
  /**
   * Inherits the top value on the stack from the 2nd-to-top value and leaves the base class.
   * @note stack: `[...][base][derived] -> [...][base]`
   * @note synopsis: `OP_INHERIT`
   */
  OP_INHERIT,
  /**
   * Finalizes the top value on the stack and leaves it.
   * @note stack: `[...][class] -> [...][class]`
   * @note synopsis: `OP_FINALIZE`
   */
  OP_FINALIZE,
  /**
   * Defines a method in the 2nd-to-top value on the stack and leaves it.
   * @note stack: `[...][class][method] -> [...][class]`
   * @note synopsis: `OP_DEFINE_METHOD, str_index, fn_type`
   * @param str_index index into constant pool to get the name of the method
   * @param fn_type function type of the method
   */
  OP_METHOD,
  /**
   * Imports a module by name.
   * @note stack: `[...] -> [...][module]`
   * @note synopsis: `OP_IMPORT, str_index`
   * @param str_index index into constant pool to get the name of the module to import.
   */
  OP_IMPORT,
  /**
   * Imports a module by name and from a file.
   * @note stack: `[...] -> [...][module]`
   * @note synopsis: `OP_IMPORT_FROM, name_index, file_path_index`
   * @param name_index index into constant pool to get the name of the module to import.
   * @param file_path_index index into constant pool to get the path of the file to import from.
   */
  OP_IMPORT_FROM,
  /**
   * Sets the current error to the top value on the stack (pops it) and puts the VM into error state.
   * @note stack: `[...][error] -> [...]`
   * @note synopsis: `OP_THROW`
   */
  OP_THROW,
  /**
   * Checks if the 2nd-to-top value on the stack is an instance of the top value and pushes the result.
   * @note stack: `[...][instance][class] -> [...][result]`
   * @note synopsis: `OP_IS`
   */
  OP_IS,
  /**
   * Checks if the 2nd-to-top value on the stack is "in" the top value and pushes the result. (Invokes `__has` on the container)
   * @note stack: `[...][value][container] -> [...][result]`
   * @note synopsis: `OP_CALL_METHOD, arg_count`
   * @param arg_count number of arguments to pass to the callable
   */
  OP_IN,
  /**
   * Gets a slice from the 3rd-to-top value on the stack and pushes the result. (Invokes `__slice` on the receiver)
   * @note stack: `[...][receiver][start][end] -> [...][slice]`
   * @note synopsis: `OP_GET_SLICE`
   */
  OP_GET_SLICE,
} OpCode;

typedef struct {
  const char* start;         // Pointer to first char of the line on which the error occurred.
  uint16_t error_start_ofs;  // Offset to [start]. Points to first char of the first token that caused the error.
  uint16_t error_end_ofs;    // Offset to [start]. Points to last char of the last token that caused the error.
  int line;                  // Line number on which the error starts.
} SourceView;

// Dynamic array of instructions.
// Provides a cache-friendly, constant-time lookup (and append) dense
// storage for instructions.
typedef struct {
  int count;
  int capacity;
  uint16_t* code;
  SourceView* source_views;
  ValueArray constants;
} Chunk;

// Initialize a chunk.
void init_chunk(Chunk* chunk);

// Free a chunk.
void free_chunk(Chunk* chunk);

// Write data to the chunk.
// This will grow the chunk if necessary.
void write_chunk(Chunk* chunk, uint16_t data, Token error_start, Token error_end);

// Add a value to the chunk's constant pool.
// Returns the index of the value in the constant pool.
int add_constant(Chunk* chunk, Value value);

#endif
