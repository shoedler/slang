#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

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
  bool found_decimal_place =
      false;  // This acts as a boolean flag to track if we've encountered a decimal point

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
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ: {
      if (IS_STRING(a) && IS_STRING(b)) {
        return AS_STRING(a) == AS_STRING(b);  // Works, because strings are interned
      }
      if (IS_OBJ(a) && IS_OBJ(b)) {
        return AS_OBJ(a) == AS_OBJ(b);
      }
      return false;
    }
    case VAL_EMPTY_INTERNAL: return true;
    case VAL_HANDLER:
      INTERNAL_ERROR("Cannot compare a value to a error handler. Vm has leaked a stack value.");
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

uint32_t hash_value(Value value) {
  // TODO (optimize): This is hot, maybe we can do better?
  switch (value.type) {
    case VAL_BOOL: return AS_BOOL(value) ? 4 : 3;
    case VAL_NIL: return 2;
    case VAL_NUMBER: return hash_double(AS_NUMBER(value));
    case VAL_OBJ: {
      if (IS_STRING(value)) {
        ObjString* string = AS_STRING(value);
        return string->hash;
      } else {
        return (uint32_t)(intptr_t)AS_OBJ(value);  // Hash objects by pointer for now...
      }
    }
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
      case VAL_NUMBER: {
        long long integer;
        if (is_int(AS_NUMBER(value), &integer)) {
          return fprintf(f, VALUE_STR_INT, integer);
        } else {
          return fprintf(f, VALUE_STR_FLOAT, AS_NUMBER(value));
        }
      }
      case VAL_EMPTY_INTERNAL: return fprintf(f, VALUE_STR_EMPTY_INTERNAL);
    }

    return fprintf(f, "<unknown value type %d>", value.type);
  }

  switch (OBJ_TYPE(value)) {
    case OBJ_STRING: return fprintf(f, "%s", AS_CSTRING(value));
    case OBJ_FUNCTION: return fprintf(f, VALUE_STRFMT_FUNCTION, AS_FUNCTION(value)->name->chars);
    case OBJ_CLOSURE: return fprintf(f, VALUE_STRFMT_FUNCTION, AS_CLOSURE(value)->function->name->chars);
    case OBJ_CLASS: return fprintf(f, VALUE_STRFMT_CLASS, AS_CLASS(value)->name->chars);
    case OBJ_INSTANCE: return fprintf(f, VALUE_STRFTM_INSTANCE, AS_INSTANCE(value)->klass->name->chars);
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
    case OBJ_SEQ: {
      ObjSeq* seq = AS_SEQ(value);
      int written = fprintf(f, VALUE_STR_SEQ_START);
      for (int i = 0; i < seq->items.count; i++) {
        written += print_value_safe(f, seq->items.values[i]);
        if (i < seq->items.count - 1) {
          written += fprintf(f, VALUE_STR_SEQ_DELIM);
        }
      }
      written += fprintf(f, VALUE_STR_SEQ_END);
      return written;
    }
    case OBJ_MAP: {
      ObjMap* map   = AS_MAP(value);
      int written   = fprintf(f, VALUE_STR_MAP_START);
      int processed = 0;
      for (int i = 0; i < map->entries.capacity; i++) {
        if (IS_EMPTY_INTERNAL(map->entries.entries[i].key)) {
          continue;
        }
        Entry* entry = &map->entries.entries[i];

        written += print_value_safe(f, entry->key);
        written += fprintf(f, VALUE_STR_MAP_SEPARATOR);
        written += print_value_safe(f, entry->value);

        if (processed < map->entries.count - 1) {
          written += fprintf(f, VALUE_STR_MAP_DELIM);
        }
        processed++;
      }
      written += fprintf(f, VALUE_STR_MAP_END);
      return written;
    }
    default: return fprintf(f, VALUE_STRFMT_OBJ, "Unknown", (void*)AS_OBJ(value));
  }
}
