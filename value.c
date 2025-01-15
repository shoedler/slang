#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void value_array_init(ValueArray* array) {
  array->values   = NULL;
  array->capacity = 0;
  array->count    = 0;
}

ValueArray value_array_init_of_size(int count) {  // TODO (refactor): Make this reentrant.
  ValueArray items;
  value_array_init(&items);

  int capacity   = GROW_CAPACITY(count);
  items.values   = RESIZE_ARRAY(Value, items.values, 0, capacity);
  items.capacity = capacity;
  items.count    = 0;

  return items;
}

void value_array_write(ValueArray* array, Value value) {
  if (SHOULD_GROW(array->count + 1, array->capacity)) {
    int old_capacity = array->capacity;
    array->capacity  = GROW_CAPACITY(old_capacity);
    array->values    = RESIZE_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

Value value_array_pop(ValueArray* array) {
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

Value value_array_remove_at(ValueArray* array, int index) {
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

void value_array_free(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  value_array_init(array);
}

int fprint_string_escaped(FILE* file, const char* str, int max_len, bool quote) {
  int written = 0;
  int count   = 0;

  if (quote) {
    written += fprintf(file, "\"");
  }

  for (const char* p = str; *p; p++) {
    if (count++ >= max_len) {
      written += fprintf(file, "...");
      break;
    }

    if (*p == '\n') {
      written += fprintf(file, "\\n");
    } else if (*p == '\t') {
      written += fprintf(file, "\\t");
    } else if (*p == '\r') {
      written += fprintf(file, "\\r");
    } else if (*p == '\v') {
      written += fprintf(file, "\\v");
    } else if (*p == '\b') {
      written += fprintf(file, "\\b");
    } else if (*p == '\f') {
      written += fprintf(file, "\\f");
    } else if (*p == '\a') {
      written += fprintf(file, "\\a");
    } else if (*p == '\\') {
      written += fprintf(file, "\\\\");
    } else if (*p == '\"') {
      written += fprintf(file, "\\\"");
    } else if (*p == '\'') {
      written += fprintf(file, "\\\'");
    } else {
      written += fprintf(file, "%c", *p);
    }
  }

  if (quote) {
    written += fprintf(file, "\"");
  }

  return written;
}

int value_print_safe(FILE* file, Value value) {
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
    return fprint_string_escaped(file, AS_STR(value)->chars, AS_STR(value)->length, false);
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
      written += value_print_safe(file, items.values[i]);
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

      written += value_print_safe(file, entry->key);
      written += fprintf(file, VALUE_STR_OBJECT_SEPARATOR);
      written += value_print_safe(file, entry->value);

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

int value_array_sort_compare_wrapper_native(Value a, Value b, Value cmp_fn) {
  UNUSED(cmp_fn);
  vm_push(a);
  vm_push(b);
  Value result = vm_exec_callable(fn_value(a.type->__lt), 1);
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    return 0;
  }

  if (is_bool(result)) {
    return result.as.boolean ? -1 : 1;
  }

  vm_error("Method \"%s." STR(SP_METHOD_LT) "\" must return a " STR(TYPENAME_BOOL) ". Got %s.", a.type->name->chars,
           result.type->name->chars);
  return 0;
}

int value_array_sort_compare_wrapper_custom(Value a, Value b, Value cmp_fn) {
  vm_push(cmp_fn);
  vm_push(a);
  vm_push(b);
  Value result = vm_exec_callable(cmp_fn, 2);
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    return 0;
  }

  if (is_int(result)) {
    return result.as.integer;
  }

  vm_error("Comparison " STR(TYPENAME_FUNCTION) " must return an " STR(TYPENAME_INT) ". Got %s.", result.type->name->chars);
  return 0;
}

void value_array_insertion_sort(ValueArray* array, SortCompareWrapperFn cmp_fn_wrapper, Value cmp_fn) {
  for (int i = 1; i < array->count; i++) {
    Value current = array->values[i];
    int j         = i;

    while (j > 0) {
      int cmp = cmp_fn_wrapper(array->values[j - 1], current, cmp_fn);
      if (cmp <= 0) {
        break;
      }

      array->values[j] = array->values[j - 1];
      j--;
    }

    if (j != i) {
      array->values[j] = current;
    }
  }
}

// Select pivot using median-of-three
static int select_pivot_index(ValueArray* array, int low, int high, SortCompareWrapperFn cmp_fn_wrapper, Value cmp_fn) {
  int mid = low + (high - low) / 2;

  // Sort low, mid, high elements
  if (cmp_fn_wrapper(array->values[low], array->values[mid], cmp_fn) > 0) {
    Value temp         = array->values[low];
    array->values[low] = array->values[mid];
    array->values[mid] = temp;
  }

  if (cmp_fn_wrapper(array->values[mid], array->values[high], cmp_fn) > 0) {
    Value temp          = array->values[mid];
    array->values[mid]  = array->values[high];
    array->values[high] = temp;

    if (cmp_fn_wrapper(array->values[low], array->values[mid], cmp_fn) > 0) {
      temp               = array->values[low];
      array->values[low] = array->values[mid];
      array->values[mid] = temp;
    }
  }

  return mid;
}

// Partition array around pivot
static int partition(ValueArray* array, int low, int high, SortCompareWrapperFn cmp_fn_wrapper, Value cmp_fn) {
  int pivot_idx = select_pivot_index(array, low, high, cmp_fn_wrapper, cmp_fn);
  Value pivot   = array->values[pivot_idx];

  // Move pivot to end
  array->values[pivot_idx] = array->values[high];
  array->values[high]      = pivot;

  int store = low;

  for (int i = low; i < high; i++) {
    if (cmp_fn_wrapper(array->values[i], pivot, cmp_fn) < 0) {
      // cmp_fn_wrapper return 0 on error, so no need to check error here
      if (i != store) {
        Value temp           = array->values[i];
        array->values[i]     = array->values[store];
        array->values[store] = temp;
      }
      store++;
    }
  }

  // Restore pivot
  array->values[high]  = array->values[store];
  array->values[store] = pivot;

  return store;
}

void value_array_quicksort(ValueArray* array, int low, int high, SortCompareWrapperFn cmp_fn_wrapper, Value cmp_fn) {
  while (high - low > 50) {  // Use quicksort for larger segments
    int p = partition(array, low, high, cmp_fn_wrapper, cmp_fn);

    // Recurse on smaller partition, iterate on larger partition
    if (p - low < high - p) {
      value_array_quicksort(array, low, p - 1, cmp_fn_wrapper, cmp_fn);
      low = p + 1;
    } else {
      value_array_quicksort(array, p + 1, high, cmp_fn_wrapper, cmp_fn);
      high = p - 1;
    }
  }

  // Use insertion sort for small segments
  if (high > low) {
    ValueArray subarray = {.values = array->values + low, .count = high - low + 1, .capacity = high - low + 1};
    value_array_insertion_sort(&subarray, cmp_fn_wrapper, cmp_fn);
  }
}
