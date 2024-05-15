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
    return nil_value();
  }
  array->count--;
  Value value = array->values[array->count];
  ensure_value_array_capacity(array, -1);
  return value;
}

Value remove_at_value_array(ValueArray* array, int index) {
  if (index < 0 || index >= array->count) {
    return nil_value();
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

  if (is_nil(a)) {
    return true;
  }
  if (is_empty_internal(a)) {
    return true;
  }

  if (is_bool(a)) {
    return a.as.boolean == b.as.boolean;
  }
  if (is_int(a)) {
    return a.as.integer == b.as.integer;
  }
  if (IS_FLOAT(a)) {
    return a.as.float_ == b.as.float_;
  }
  if (is_str(a)) {
    return AS_STR(a) == AS_STR(b);  // Works, because strings are interned
  }
  if (is_handler(a)) {
    INTERNAL_ERROR("Cannot compare a value to a error handler. Vm has leaked a stack value.");
    return false;
  }

  // We trust that all non-object types are handled above.
  return a.as.obj->hash == b.as.obj->hash;
}

// Hashes a double. Borrowed from Lua.
static uint64_t hash_double(double value) {
  union BitCast {
    double source;
    uint64_t target;
  };

  union BitCast cast;
  cast.source = (value) + 1.0;
  return cast.target;
}

// Hashes a long long.
static uint64_t hash_int(long long value) {
  // Bc of 2's complement, directly casting to uint64_t should ensure unique hash values.
  return (uint64_t)value;
}

uint64_t hash_value(Value value) {
  if (is_empty_internal(value)) {
    return 0;
  }

  if (is_nil(value)) {
    return 2;
  }
  if (is_bool(value)) {
    return value.as.boolean ? 977 : 479;  // Prime numbers. Selected based on trial and error.
  }
  if (is_int(value)) {
    return hash_int(value.as.integer);
  }
  if (IS_FLOAT(value)) {
    return hash_double(value.as.float_);
  }

  // We trust that all non-object types are handled above.
  return value.as.obj->hash;
}

int print_value_safe(FILE* f, Value value) {
  if (is_bool(value)) {
    return fprintf(f, value.as.boolean ? VALUE_STR_TRUE : VALUE_STR_FALSE);
  }
  if (is_nil(value)) {
    return fprintf(f, VALUE_STR_NIL);
  }
  if (is_handler(value)) {
    return fprintf(f, VALUE_STRFMT_HANDLER, value.as.handler);
  }
  if (is_int(value)) {
    return fprintf(f, VALUE_STR_INT, value.as.integer);
  }
  if (IS_FLOAT(value)) {
    return fprintf(f, VALUE_STR_FLOAT, value.as.float_);
  }
  if (is_empty_internal(value)) {
    return fprintf(f, VALUE_STR_EMPTY_INTERNAL);
  }

  if (is_str(value)) {
    return fprintf(f, "%s", AS_CSTRING(value));
  }
  if (is_closure(value)) {
    return fprintf(f, VALUE_STRFMT_FUNCTION, AS_CLOSURE(value)->function->name->chars);
  }
  if (is_function(value)) {
    return fprintf(f, VALUE_STRFMT_FUNCTION, AS_FUNCTION(value)->name->chars);
  }
  if (is_class(value)) {
    return fprintf(f, VALUE_STRFMT_CLASS, AS_CLASS(value)->name->chars);
  }
  if (is_native(value)) {
    return fprintf(f, VALUE_STRFMT_NATIVE, AS_NATIVE(value)->name->chars);
  }
  if (is_bound_method(value)) {
    ObjBoundMethod* bound = AS_BOUND_METHOD(value);
    if (bound->method == NULL) {
      return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(corrupt)");
    }

    if (bound->method->type == OBJ_GC_CLOSURE) {
      if (((ObjClosure*)bound->method)->function->name != NULL) {
        return fprintf(f, VALUE_STRFMT_BOUND_METHOD, ((ObjClosure*)bound->method)->function->name->chars);
      } else {
        return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(corrupt closure)");
      }
    } else if (bound->method->type == OBJ_GC_NATIVE) {
      return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(native)");
    } else {
      return fprintf(f, VALUE_STRFMT_BOUND_METHOD, "(unknown)");
    }
  }

  if (is_tuple(value) || is_seq(value)) {
    const char* start;
    const char* delim;
    const char* end;

    if (is_seq(value)) {
      start = VALUE_STR_SEQ_START;
      delim = VALUE_STR_SEQ_DELIM;
      end   = VALUE_STR_SEQ_END;
    } else {
      start = VALUE_STR_TUPLE_START;
      delim = VALUE_STR_TUPLE_DELIM;
      end   = VALUE_STR_TUPLE_END;
    }

    ValueArray items = LISTLIKE_GET_VALUEARRAY(value);

    int written = fprintf(f, start);
    for (int i = 0; i < items.count; i++) {
      written += print_value_safe(f, items.values[i]);
      if (i < items.count - 1) {
        written += fprintf(f, delim);
      }
    }
    written += fprintf(f, end);
    return written;
  }

  if (is_obj(value)) {
    ObjObject* object = AS_OBJECT(value);

    int written   = fprintf(f, VALUE_STR_OBJECT_START);
    int processed = 0;
    for (int i = 0; i < object->fields.capacity; i++) {
      if (is_empty_internal(object->fields.entries[i].key)) {
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

  // Everything else is an instance.
  return fprintf(f, VALUE_STRFTM_INSTANCE, value.type->name->chars);
}
