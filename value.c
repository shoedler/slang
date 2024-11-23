#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void init_value_array(ValueArray* array) {
  array->values   = NULL;
  array->capacity = 0;
  array->count    = 0;
}

ValueArray init_value_array_of_size(int count) {  // TODO (refactor): Make this reentrant.
  ValueArray items;
  init_value_array(&items);

  int capacity   = GROW_CAPACITY(count);
  items.values   = RESIZE_ARRAY(Value, items.values, 0, capacity);
  items.capacity = capacity;
  items.count    = 0;

  return items;
}

void write_value_array(ValueArray* array, Value value) {
  if (SHOULD_GROW(array->count + 1, array->capacity)) {
    int old_capacity = array->capacity;
    array->capacity  = GROW_CAPACITY(old_capacity);
    array->values    = RESIZE_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

Value pop_value_array(ValueArray* array) {
  if (array->count == 0) {
    return nil_value();
  }

  array->count--;
  Value value = array->values[array->count];

  if (SHOULD_SHRINK(array->count - 1, array->capacity)) {
    int old_capacity = array->capacity;
    array->capacity  = SHRINK_CAPACITY(old_capacity);
    array->values    = RESIZE_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

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

  if (SHOULD_SHRINK(array->count - 1, array->capacity)) {
    int old_capacity = array->capacity;
    array->capacity  = SHRINK_CAPACITY(old_capacity);
    array->values    = RESIZE_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

  return value;
}

void free_value_array(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  init_value_array(array);
}

int is_digit(char chr) {
  return chr >= '0' && chr <= '9';
}

double string_to_double(char* str, int length) {
  double result              = 0.0;
  double decimal_place_value = 1.0;
  bool found_decimal_place   = false;

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

bool values_equal(Value left, Value right) {
  // TODO (optimize): This is hot, maybe we can do better?
  if (left.type != right.type) {
    return false;
  }

  // All non-object types are handled "manually". Obj are compared by their hash.
  if (is_nil(left)) {
    return true;
  }
  if (is_empty_internal(left)) {
    return true;
  }
  if (is_bool(left)) {
    return left.as.boolean == right.as.boolean;
  }
  if (is_int(left)) {
    return left.as.integer == right.as.integer;
  }
  if (is_float(left)) {
    return left.as.float_ == right.as.float_;
  }
  if (is_str(left)) {
    return AS_STR(left) == AS_STR(right);  // Works, because strings are interned
  }
  if (is_handler(left)) {
    INTERNAL_ERROR("Cannot compare a value to a error handler. Vm has leaked a stack value.");
    return false;
  }

  // We trust that all non-object types are handled above.
  return left.as.obj->hash == right.as.obj->hash;
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

uint64_t hash_value(Value value) {
  // All non-object types are handled "manually". Obj are hashed by their hash.
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
    return (uint64_t)value.as.integer;  // Bc of 2's complement, directly casting to uint64_t should ensure unique hash values.
  }
  if (is_float(value)) {
    return hash_double(value.as.float_);
  }

  // We trust that all non-object types are handled above.
  return value.as.obj->hash;
}

int print_value_safe(FILE* file, Value value) {
  if (is_bool(value)) {
    return fprintf(file, value.as.boolean ? VALUE_STR_TRUE : VALUE_STR_FALSE);
  }
  if (is_nil(value)) {
    return fprintf(file, VALUE_STR_NIL);
  }
  if (is_handler(value)) {
    return fprintf(file, VALUE_STRFMT_HANDLER, value.as.handler);
  }
  if (is_int(value)) {
    return fprintf(file, VALUE_STR_INT, value.as.integer);
  }
  if (is_float(value)) {
    return fprintf(file, VALUE_STR_FLOAT, value.as.float_);
  }
  if (is_empty_internal(value)) {
    return fprintf(file, VALUE_STR_EMPTY_INTERNAL);
  }

  if (is_str(value)) {
    return fprintf(file, "%s", AS_CSTRING(value));
  }
  if (is_closure(value)) {
    return fprintf(file, VALUE_STRFMT_FUNCTION, AS_CLOSURE(value)->function->name->chars);
  }
  if (is_function(value)) {
    return fprintf(file, VALUE_STRFMT_FUNCTION, AS_FUNCTION(value)->name->chars);
  }
  if (is_class(value)) {
    return fprintf(file, VALUE_STRFMT_CLASS, AS_CLASS(value)->name->chars);
  }
  if (is_native(value)) {
    return fprintf(file, VALUE_STRFMT_NATIVE, AS_NATIVE(value)->name->chars);
  }
  if (is_bound_method(value)) {
    ObjBoundMethod* bound = AS_BOUND_METHOD(value);
    if (bound->method == NULL) {
      return fprintf(file, VALUE_STRFMT_BOUND_METHOD, "(corrupt)");
    }

    if (bound->method->type == OBJ_GC_CLOSURE) {
      if (((ObjClosure*)bound->method)->function->name != NULL) {
        return fprintf(file, VALUE_STRFMT_BOUND_METHOD, ((ObjClosure*)bound->method)->function->name->chars);
      }
      return fprintf(file, VALUE_STRFMT_BOUND_METHOD, "(corrupt closure)");
    }

    if (bound->method->type == OBJ_GC_NATIVE) {
      return fprintf(file, VALUE_STRFMT_BOUND_METHOD, "(native)");
    }

    return fprintf(file, VALUE_STRFMT_BOUND_METHOD, "(unknown)");
  }

  if (is_tuple(value) || is_seq(value)) {
    const char* start;
    const char* delim;
    const char* end;
    ValueArray items;

    if (is_seq(value)) {
      start = VALUE_STR_SEQ_START;
      delim = VALUE_STR_SEQ_DELIM;
      end   = VALUE_STR_SEQ_END;
      items = AS_SEQ(value)->items;
    } else {
      start = VALUE_STR_TUPLE_START;
      delim = VALUE_STR_TUPLE_DELIM;
      end   = VALUE_STR_TUPLE_END;
      items = AS_TUPLE(value)->items;
    }

    int written = fprintf(file, start);
    for (int i = 0; i < items.count; i++) {
      written += print_value_safe(file, items.values[i]);
      if (i < items.count - 1) {
        written += fprintf(file, delim);
      }
    }
    written += fprintf(file, end);
    return written;
  }

  if (is_obj(value)) {
    ObjObject* object = AS_OBJECT(value);

    int written   = fprintf(file, VALUE_STR_OBJECT_START);
    int processed = 0;
    for (int i = 0; i < object->fields.capacity; i++) {
      if (is_empty_internal(object->fields.entries[i].key)) {
        continue;
      }
      Entry* entry = &object->fields.entries[i];

      written += print_value_safe(file, entry->key);
      written += fprintf(file, VALUE_STR_OBJECT_SEPARATOR);
      written += print_value_safe(file, entry->value);

      if (processed < object->fields.count - 1) {
        written += fprintf(file, VALUE_STR_OBJECT_DELIM);
      }
      processed++;
    }
    written += fprintf(file, VALUE_STR_OBJECT_END);
    return written;
  }

  // Everything else is an instance.
  return fprintf(file, VALUE_STRFTM_INSTANCE, value.type->name->chars);
}
