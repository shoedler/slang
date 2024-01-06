#include <stdio.h>

#include "memory.h"
#include "object.h"
#include "value.h"

void init_value_array(ValueArray* array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void write_value_array(ValueArray* array, Value value) {
  if (array->capacity < array->count + 1) {
    int old_capacity = array->capacity;
    array->capacity = GROW_CAPACITY(old_capacity);
    array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void free_value_array(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  init_value_array(array);
}

void print_value(Value value) {
  switch (value.type) {
    case VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case VAL_NIL:
      printf("nil");
      break;
    case VAL_NUMBER: {
      double number = AS_NUMBER(value);
      if (number == (int)number) {
        printf("%d", (int)number);
      } else {
        printf("%f", number);
      }
      break;
    }
    case VAL_OBJ:
      print_object(value);
      break;
    case VAL_EMPTY_INTERNAL:
      printf("EMPTY_INTERNAL");
      break;
  }
}

bool values_equal(Value a, Value b) {
  // TODO (optimize): This is hot, maybe we can do better?
  if (a.type != b.type) {
    return false;
  }

  switch (a.type) {
    case VAL_BOOL:
      return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
      return true;
    case VAL_NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ: {
      if (IS_STRING(a) && IS_STRING(b)) {
        return AS_STRING(a) == AS_STRING(b);
      }
      if (IS_OBJ(a) && IS_OBJ(b)) {
        return true;
      }
      return false;
    }
    case VAL_EMPTY_INTERNAL:
      return true;
    default:
      INTERNAL_ERROR("Unhandled comparison type: %d", a.type);
      return false;
  }
}

// Hashes a double. Borrowed from Lua.
static uint32_t hash_doube(double value) {
  union BitCast {
    double source;
    uint32_t target[2];
  };

  union BitCast cast;
  cast.source = (value) + 1.0;
  return cast.target[0] + cast.target[1];
}

uint32_t hash_value(Value value) {
  // TODO (optimize): This is hot, maybe we can do better?
  switch (value.type) {
    case VAL_BOOL:
      return AS_BOOL(value) ? 4 : 3;
    case VAL_NIL:
      return 2;
    case VAL_NUMBER:
      return hash_doube(AS_NUMBER(value));
    case VAL_OBJ: {
      if (IS_STRING(value)) {
        ObjString* string = AS_STRING(value);
        return string->hash;
      } else {
        return (uint32_t)(intptr_t)AS_OBJ(value);  // Hash objects by pointer for now...
      }
    }
    case VAL_EMPTY_INTERNAL:
      return 0;
    default:
      INTERNAL_ERROR("Unhandled hash type: %d", value.type);
      return 0;
  }
}
