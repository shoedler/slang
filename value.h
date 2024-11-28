#ifndef value_h
#define value_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjSeq ObjSeq;
typedef struct ObjTuple ObjTuple;
typedef struct ObjClass ObjClass;

#define TYPENAME_VALUE Value
#define TYPENAME_OBJ Obj
#define TYPENAME_FLOAT Float
#define TYPENAME_INT Int
#define TYPENAME_NUM Num
#define TYPENAME_STRING Str
#define TYPENAME_BOOL Bool
#define TYPENAME_NIL Nil
#define TYPENAME_SEQ Seq
#define TYPENAME_TUPLE Tuple
#define TYPENAME_BOUND_METHOD BoundMethod
#define TYPENAME_CLASS Class
#define TYPENAME_CLOSURE Fn
#define TYPENAME_FUNCTION Fn
#define TYPENAME_INSTANCE Instance
#define TYPENAME_NATIVE NativeFn
#define TYPENAME_UPVALUE Upvalue

#define TYPENAME_MODULE Module

// On windows long long (%lld) is the type to use if you're casting from a double.
#define VALUE_STR_INT "%lld"
#define VALUE_STR_FLOAT "%.14f"  // TODO (optimize): Kinda arbitrary, but after .14, there seems to be some floating point errors.
#define VALUE_STR_TRUE "true"
#define VALUE_STR_FALSE "false"
#define VALUE_STR_NIL "nil"
#define VALUE_STR_EMPTY_INTERNAL "EMPTY_INTERNAL"

#define VALUE_STRFMT_FUNCTION "<Fn %s>"
#define VALUE_STRFMT_FUNCTION_LEN (sizeof(VALUE_STRFMT_FUNCTION) - 2)
#define VALUE_STRFMT_NATIVE "<NativeFn %s>"
#define VALUE_STRFMT_NATIVE_LEN (sizeof(VALUE_STRFMT_NATIVE) - 2)
#define VALUE_STRFMT_CLASS "<%s>"
#define VALUE_STRFMT_CLASS_LEN (sizeof(VALUE_STRFMT_CLASS) - 2)
#define VALUE_STRFTM_INSTANCE "<Instance of %s>"
#define VALUE_STRFTM_INSTANCE_LEN (sizeof(VALUE_STRFTM_INSTANCE) - 2)
#define VALUE_STRFMT_BOUND_METHOD "<BoundMethod %s>"
#define VALUE_STRFMT_BOUND_METHOD_LEN (sizeof(VALUE_STRFMT_BOUND_METHOD) - 2)
#define VALUE_STRFMT_HANDLER "<Try -> %hu>"  // The handler is a 16-bit unsigned integer
#define VALUE_STRFMT_OBJ "<%s at %p>"
#define VALUE_STRFMT_OBJ_LEN (sizeof(VALUE_STRFMT_OBJ) - 4)
#define VALUE_STR_SEQ_START "["
#define VALUE_STR_SEQ_DELIM ", "
#define VALUE_STR_SEQ_END "]"
#define VALUE_STR_TUPLE_START "("
#define VALUE_STR_TUPLE_DELIM ", "
#define VALUE_STR_TUPLE_END ")"
#define VALUE_STR_OBJECT_START "{"
#define VALUE_STR_OBJECT_END "}"
#define VALUE_STR_OBJECT_SEPARATOR ": "
#define VALUE_STR_OBJECT_DELIM ", "
#define VALUE_STR_UPVALUE "<Upvalue>"

// The single value construct used to represent all values in the language.
typedef struct {
  ObjClass* type;
  union {
    bool boolean;
    double float_;
    long long integer;
    uint16_t handler;
    Obj* obj;
  } as;
} Value;

// Dynamic array of values.
// See https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.4
typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

// Initialize a value array.
void value_array_init(ValueArray* array);

// Creates, initializes and allocates a new value array. Initializes the value array and capacity to add [count] values without
// needing to resize. It's intended to add items directly to .values[idx], no need to use value_array_write. You MUST fill the
// array up to [count] with some sort of value after calling this function (Or reduce the count manually, maybe?). Might trigger
// garbage collection.
//
// Note: The returned value array will be initialized with .count = 0. This is because the preallocated .values
// array contains garbage memory. IF you trigger the Gc during filling the array and count would already be at [count], the Gc
// would try to free the garbage memory - doing so requires you to increment .count during filling the array.
ValueArray value_array_init_of_size(int count);

// Write a value to a value array.
void value_array_write(ValueArray* array, Value value);

// Pop a value from a value array.
// Returns NIL_VAL if the array is empty.
Value value_array_pop(ValueArray* array);

// Remove a value at a specific index from a value array.
// Returns NIL_VAL if the index is out of bounds.
Value value_array_remove_at(ValueArray* array, int index);

// Free a value array.
void value_array_free(ValueArray* array);

// Prints a value to a file. Will look different from the values default print representation, but it will
// guarantee that the gc will not be called.
// Returns the number of characters printed.
int value_print_safe(FILE* file, Value value);

#endif
