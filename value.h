#ifndef value_h
#define value_h

#include <stdio.h>
#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjSeq ObjSeq;

// The type of a value.
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
  VAL_HANDLER,
  VAL_EMPTY_INTERNAL,
} ValueType;

#define TYPENAME_OBJ Obj
#define TYPENAME_NUMBER Num
#define TYPENAME_STRING Str
#define TYPENAME_BOOL Bool
#define TYPENAME_NIL Nil
#define TYPENAME_SEQ Seq
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
#define VALUE_STR_FLOAT "%f"
#define VALUE_STR_TRUE "true"
#define VALUE_STR_FALSE "false"
#define VALUE_STR_NIL "nil"
#define VALUE_STR_EMPTY_INTERNAL "EMPTY_INTERNAL"

#define VALUE_STRFMT_FUNCTION "<Fn %s>"
#define VALUE_STRFMT_FUNCTION_LEN (sizeof(VALUE_STRFMT_FUNCTION) - 2)
#define VALUE_STRFMT_CLASS "<Class %s>"
#define VALUE_STRFMT_CLASS_LEN (sizeof(VALUE_STRFMT_CLASS) - 2)
#define VALUE_STRFTM_INSTANCE "<Instance of %s>"
#define VALUE_STRFTM_INSTANCE_LEN (sizeof(VALUE_STRFTM_INSTANCE) - 2)
#define VALUE_STRFMT_BOUND_METHOD "<BoundMethod %s>"
#define VALUE_STRFMT_BOUND_METHOD_LEN (sizeof(VALUE_STRFMT_BOUND_METHOD) - 2)
#define VALUE_STRFMT_HANDLER "<Try -> %hu>"  // The handler is a 16-bit unsigned integer
#define VALUE_STRFMT_OBJ "<%s at %p>"
#define VALUE_STRFMT_OBJ_LEN (sizeof(VALUE_STRFMT_OBJ) - 4)
#define VALUE_STR_NATIVE "<Native Fn>"
#define VALUE_STR_SEQ_START "["
#define VALUE_STR_SEQ_DELIM ", "
#define VALUE_STR_SEQ_END "]"
#define VALUE_STR_OBJECT_START "{"
#define VALUE_STR_OBJECT_END "}"
#define VALUE_STR_OBJECT_SEPARATOR ": "
#define VALUE_STR_OBJECT_DELIM ", "
#define VALUE_STR_UPVALUE "<Upvalue>"

#define INSTANCENAME_BUILTIN "__builtin"

// The single value construct used to represent all values in the language.
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    uint16_t handler;
    Obj* obj;
  } as;
} Value;

// Determines whether a value is of type boolean.
#define IS_BOOL(value) ((value).type == VAL_BOOL)

// Determines whether a value is of type nil.
#define IS_NIL(value) ((value).type == VAL_NIL)

// Determines whether a value is of type number.
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

// Determines whether a value is of type object.
#define IS_OBJ(value) ((value).type == VAL_OBJ)

// Determines whether a value is of type (error) handler.
#define IS_HANDLER(value) ((value).type == VAL_HANDLER)

// Determines whether a value is of type empty.
// Users will never see this type, it is used internally to represent empty buckets in the
// hashtable.
#define IS_EMPTY_INTERNAL(value) ((value).type == VAL_EMPTY_INTERNAL)

// Unpacks a value into a C boolean.
// Value must be of type bool.
#define AS_BOOL(value) ((value).as.boolean)

// Unpacks a value into a C double.
// Value must be of type number.
#define AS_NUMBER(value) ((value).as.number)

// Unpacks a value into a C object pointer.
// Value must be of type object.
#define AS_OBJ(value) ((value).as.obj)

// Unpacks a value into a C long long.
// Value must be of type (error) handler.
#define AS_HANDLER(value) ((value).as.handler)

// Converts a C boolean into a value.
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})

// The singleton nil value.
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})

// Converts a C double into a value.
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

// Converts a C object pointer into a value.
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

// Converts a C long long into a value.
#define HANDLER_VAL(value) ((Value){VAL_HANDLER, {.handler = value}})

// The singleton empty value.
// Users will never see this value, it is used internally to represent empty buckets in the
// hashtable.
#define EMPTY_INTERNAL_VAL ((Value){VAL_EMPTY_INTERNAL, {.number = 0}})

// Dynamic array of values. This represents the constant pool of a chunk.
// See https://docs.oracle.com/javase/specs/jvms/se7/html/jvms-4.html#jvms-4.4
typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

// Determines whether two values are equal.
// Values are not eqal if their types differ.
bool values_equal(Value a, Value b);

// Initialize a value array.
void init_value_array(ValueArray* array);

// Write a value to a value array.
void write_value_array(ValueArray* array, Value value);

// Pop a value from a value array.
// Returns NIL_VAL if the array is empty.
Value pop_value_array(ValueArray* array);

// Remove a value at a specific index from a value array.
// Returns NIL_VAL if the index is out of bounds.
Value remove_at_value_array(ValueArray* array, int index);

// Free a value array.
void free_value_array(ValueArray* array);

// Returns true if a floating point number is an integer, false otherwise.
// Assignes the resulting integer value to the long long pointer.
bool is_int(double number, long long* integer);

// Get the hashcode of a value, based on its type.
uint32_t hash_value(Value value);

// Converts a string to a double.
// Result for "[ 1, 2, 3, 4]": 1234.000000
// Result for "12.34.56": 12.345600
// Returns 0.0 if the string is not a valid number.
double string_to_double(char* str, int length);

// Prints a value to a file. Will look different from the values default print representation, but it will
// guarantee that the gc will not be called.
// Returns the number of characters printed.
int print_value_safe(FILE* file, Value value);

#endif
