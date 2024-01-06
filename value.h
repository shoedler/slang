#ifndef value_h
#define value_h

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
  VAL_EMPTY_INTERNAL,
} ValueType;

// The single value construct used to represent all values in the language.
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
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

// Converts a C boolean into a value.
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})

// The singleton nil value.
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})

// Converts a C double into a value.
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

// Converts a C object pointer into a value.
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

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

// Free a value array.
void free_value_array(ValueArray* array);

// Print a value to stdout.
void print_value(Value value);

// Get the hashcode of a value, based on its type.
uint32_t hash_value(Value value);

#endif