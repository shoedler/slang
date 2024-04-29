#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void init_value_array(ValueArray* array) {
  array->values   = NULL;
  array->capacity = 0;
  array->count    = 0;
}

// Updates the capacity of the value array based on the provided count offset. It grows or shrinks the array's
// capacity only if the new count surpasses growth thresholds or falls below shrinkage criteria.
static void ensure_value_array_capacity(ValueArray* array, int count_offset) {
  int new_count = array->count + count_offset;
  if (SHOULD_GROW(new_count, array->capacity)) {
    // Grow
    int old_capacity = array->capacity;
    array->capacity  = GROW_CAPACITY(old_capacity);
    array->values    = RESIZE_ARRAY(Value, array->values, old_capacity, array->capacity);
  } else if (SHOULD_SHRINK(new_count, array->capacity)) {
    // Shrink
    int old_capacity = array->capacity;
    array->capacity  = SHRINK_CAPACITY(old_capacity);
    array->values    = RESIZE_ARRAY(Value, array->values, old_capacity, array->capacity);
  }
}

void write_value_array(ValueArray* array, Value value) {
  ensure_value_array_capacity(array, +1);

  array->values[array->count] = value;
  array->count++;
}

Value pop_value_array(ValueArray* array) {
  if (array->count == 0) {
    return NIL_VAL;
  }
  array->count--;
  Value value = array->values[array->count];
  ensure_value_array_capacity(array, -1);
  return value;
}

Value remove_at_value_array(ValueArray* array, int index) {
  if (index < 0 || index >= array->count) {
    return NIL_VAL;
  }
  Value value = array->values[index];
  array->count--;
  for (int i = index; i < array->count; i++) {
    array->values[i] = array->values[i + 1];
  }
  ensure_value_array_capacity(array, -1);
  return value;
}

void free_value_array(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  init_value_array(array);
}

bool is_int(double number, long long* integer) {
  double dint = rint(number);
  *integer    = (long long)dint;
  return number == dint;
}

int is_digit(char c) {
  return c >= '0' && c <= '9';
}

double string_to_double(char* str, int length) {
  double result              = 0.0;
  double decimal_place_value = 1.0;
  bool found_decimal_place   = false;  // This acts as a boolean flag to track if we've encountered a decimal point

  for (int i = 0; i < length; ++i) {
    if (is_digit(str[i])) {
      if (found_decimal_place) {
        // Process fraction
        decimal_place_value /= 10.0;
        result += (str[i] - '0') * decimal_place_value;
      } else {
        // Process whole number
        result = result * 10.0 + (str[i] - '0');
      }
    } else if (str[i] == '.' && !found_decimal_place) {
      found_decimal_place = true;  // Mark that we've found the decimal point
    }
    // Ignore all other characters
  }

  return result;
}

bool values_equal(Value a, Value b) {
  // TODO (optimize): This is hot, maybe we can do better?
  if (a.type != b.type) {
    return false;
  }

  switch (a.type) {
    case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL: return true;
    case VAL_INT: return AS_INT(a) == AS_INT(b);
    case VAL_FLOAT: return AS_FLOAT(a) == AS_FLOAT(b);
    case VAL_OBJ: {
      if (IS_STRING(a) && IS_STRING(b)) {
        return AS_STRING(a) == AS_STRING(b);  // Works, because strings are interned
      }
      if (IS_OBJ(a) && IS_OBJ(b)) {
        return AS_OBJ(a)->hash == AS_OBJ(b)->hash;
      }
      return false;
    }
    case VAL_EMPTY_INTERNAL: return true;
    case VAL_HANDLER: INTERNAL_ERROR("Cannot compare a value to a error handler. Vm has leaked a stack value.");
    default: INTERNAL_ERROR("Unhandled comparison type: %d", a.type); return false;
  }
}

// Hashes a double. Borrowed from Lua.
static uint32_t hash_double(double value) {
  union BitCast {
    double source;
    uint32_t target[2];
  };

  union BitCast cast;
  cast.source = (value) + 1.0;
  return cast.target[0] + cast.target[1];
}

// Hashes a long long.
static uint32_t hash_int(long long value) {
  return (uint32_t)value;
}

uint32_t hash_value(Value value) {
  // TODO (optimize): This is hot, maybe we can do better?
  switch (value.type) {
    case VAL_BOOL: return AS_BOOL(value) ? 4 : 3;
    case VAL_NIL: return 2;
    case VAL_INT: return hash_int(AS_INT(value));
    case VAL_FLOAT: return hash_double(AS_FLOAT(value));
    case VAL_OBJ: return AS_OBJ(value)->hash;
    case VAL_EMPTY_INTERNAL: return 0;
    default: INTERNAL_ERROR("Unhandled hash type: %d", value.type); return 0;
  }
}

int print_value_safe(FILE* f, Value value) {
  // TODO (refactor): Use nested switch-case (value.type) and for VAL_OBJ, switch-case (OBJ_TYPE(value))
  if (!IS_OBJ(value)) {
    switch (value.type) {
      case VAL_BOOL: return fprintf(f, AS_BOOL(value) ? VALUE_STR_TRUE : VALUE_STR_FALSE);
      case VAL_NIL: return fprintf(f, VALUE_STR_NIL);
      case VAL_HANDLER: return fprintf(f, VALUE_STRFMT_HANDLER, AS_HANDLER(value));
      case VAL_INT: return fprintf(f, VALUE_STR_INT, AS_INT(value));
      case VAL_FLOAT: return fprintf(f, VALUE_STR_FLOAT, AS_FLOAT(value));
      case VAL_EMPTY_INTERNAL: return fprintf(f, VALUE_STR_EMPTY_INTERNAL);
    }

    return fprintf(f, "<unknown value type %d>", value.type);
  }

  switch (OBJ_TYPE(value)) {
    case OBJ_STRING: return fprintf(f, "%s", AS_CSTRING(value));
    case OBJ_FUNCTION: return fprintf(f, VALUE_STRFMT_FUNCTION, AS_FUNCTION(value)->name->chars);
    case OBJ_CLOSURE: return fprintf(f, VALUE_STRFMT_FUNCTION, AS_CLOSURE(value)->function->name->chars);
    case OBJ_CLASS: return fprintf(f, VALUE_STRFMT_CLASS, AS_CLASS(value)->name->chars);
    case OBJ_NATIVE: return fprintf(f, VALUE_STR_NATIVE);
    case OBJ_BOUND_METHOD: {
      ObjBoundMethod* bound = AS_BOUND_METHOD(value);
      if (bound->method == NULL) {
        return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(corrupt)");
      }

      if (bound->method->type == OBJ_CLOSURE) {
        if (((ObjClosure*)bound->method)->function->name != NULL) {
          return fprintf(f, VALUE_STRFMT_BOUND_METHOD, ((ObjClosure*)bound->method)->function->name->chars);
        } else {
          return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(corrupt closure)");
        }
      } else if (bound->method->type == OBJ_NATIVE) {
        return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(native)");
      } else {
        return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(unknown)");
      }
    }
    case OBJ_TUPLE:
    case OBJ_SEQ: {
      const char* start;
      const char* delim;
      const char* end;
      ValueArray* items;

      if (IS_SEQ(value)) {
        start = VALUE_STR_SEQ_START;
        delim = VALUE_STR_SEQ_DELIM;
        end   = VALUE_STR_SEQ_END;
        items = &AS_SEQ(value)->items;
      } else {
        start = VALUE_STR_TUPLE_START;
        delim = VALUE_STR_TUPLE_DELIM;
        end   = VALUE_STR_TUPLE_END;
        items = &AS_TUPLE(value)->items;
      }

      int written = fprintf(f, start);
      for (int i = 0; i < items->count; i++) {
        written += print_value_safe(f, items->values[i]);
        if (i < items->count - 1) {
          written += fprintf(f, delim);
        }
      }
      written += fprintf(f, end);
      return written;
    }
    case OBJ_OBJECT: {
      ObjObject* object = AS_OBJECT(value);
      if (OBJECT_IS_INSTANCE(object)) {
        return fprintf(f, VALUE_STRFTM_INSTANCE, AS_OBJECT(value)->klass->name->chars);
      }

      int written   = fprintf(f, VALUE_STR_OBJECT_START);
      int processed = 0;
      for (int i = 0; i < object->fields.capacity; i++) {
        if (IS_EMPTY_INTERNAL(object->fields.entries[i].key)) {
          continue;
        }
        Entry* entry = &object->fields.entries[i];

        written += print_value_safe(f, entry->key);
        written += fprintf(f, VALUE_STR_OBJECT_SEPARATOR);
        written += print_value_safe(f, entry->value);

        if (processed < object->fields.count - 1) {
          written += fprintf(f, VALUE_STR_OBJECT_DELIM);
        }
        processed++;
      }
      written += fprintf(f, VALUE_STR_OBJECT_END);
      return written;
    }
    default: return fprintf(f, VALUE_STRFMT_OBJ, "Unknown", (void*)AS_OBJ(value));
  }
}
