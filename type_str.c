#include <stdint.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool str_get_prop(Value receiver, ObjString* name, Value* result);
NATIVE_SET_PROP_NOT_SUPPORTED()
static bool str_get_subs(Value receiver, Value index, Value* result);
NATIVE_SET_SUBS_NOT_SUPPORTED()

static Value str_ctor(int argc, Value argv[]);
static Value str_to_str(int argc, Value argv[]);
static Value str_has(int argc, Value argv[]);
static Value str_slice(int argc, Value argv[]);
static Value str_split(int argc, Value argv[]);
static Value str_trim(int argc, Value argv[]);

void finalize_native_str_class() {
  vm.str_class->__get_prop = str_get_prop;
  vm.str_class->__set_prop = set_prop_not_supported;  // Not supported
  vm.str_class->__get_subs = str_get_subs;
  vm.str_class->__set_subs = set_subs_not_supported;  // Not supported

  define_native(&vm.str_class->methods, STR(SP_METHOD_CTOR), str_ctor, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_TO_STR), str_to_str, 0);
  define_native(&vm.str_class->methods, STR(SP_METHOD_HAS), str_has, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_SLICE), str_slice, 2);
  define_native(&vm.str_class->methods, "split", str_split, 1);
  define_native(&vm.str_class->methods, "trim", str_trim, 0);
  finalize_new_class(vm.str_class);
}

static bool str_get_prop(Value receiver, ObjString* name, Value* result) {
  if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {
    ObjString* str = AS_STR(receiver);
    *result        = int_value(str->length);
    return true;
  }
  NATIVE_DEFAULT_GET_PROP_BODY(vm.str_class)
}

static bool str_get_subs(Value receiver, Value index, Value* result) {
  ObjString* string = AS_STR(receiver);

  if (!is_int(index)) {
    false;
  }

  long long idx = index.as.integer;
  if (idx >= string->length) {
    *result = nil_value();
    return true;
  }

  // Negative index
  if (idx < 0) {
    idx += string->length;
  }
  if (idx < 0) {
    *result = nil_value();
    return true;
  }

  ObjString* char_str = copy_string(string->chars + idx, 1);
  *result             = str_value(char_str);
  return true;
}

/**
 * TYPENAME_STRING.SP_METHOD_CTOR(value: TYPENAME_OBJ) -> TYPENAME_STRING
 * @brief Converts the first argument to a TYPENAME_STRING.
 */
static Value str_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  // Execute the to_str method on the argument
  push(argv[1]);  // Push the receiver for to_str, which is the ctors' argument
  Value result = exec_callable(fn_value(argv[1].type->__to_str), 0);  // Convert to string
  if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
    return nil_value();
  }

  return result;
}

/**
 * TYPENAME_STRING.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of a TYPENAME_STRING.
 */
static Value str_to_str(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)

  return argv[0];
}

/**
 * TYPENAME_STRING.split(sep: TYPENAME_STRING) -> TYPENAME_SEQ
 * @brief Splits a TYPENAME_STRING into a TYPENAME_SEQ of substrings, using 'sep' as the delimiter.
 */
static Value str_split(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)
  NATIVE_CHECK_ARG_AT(1, vm.str_class)

  ObjString* str = AS_STR(argv[0]);
  ObjString* sep = AS_STR(argv[1]);

  // If the separator is empty, split by character
  if (sep->length == 0) {
    ValueArray items = prealloc_value_array(str->length);
    ObjSeq* seq      = take_seq(&items);  // We can already take the seq, because seqs don't calculate the hash upon taking.
    push(seq_value(seq));                 // GC Protection
    for (int i = 0; i < str->length; i++) {
      seq->items.values[i] = str_value(copy_string(str->chars + i, 1));
      seq->items.count++;
    }

    return pop();  // The seq
  }

  ObjSeq* seq = new_seq();
  push(seq_value(seq));  // GC Protection

  // Split the string by looking for the separator at each character
  int start = 0;
  for (int i = 0; i < str->length; i++) {
    if (strncmp(str->chars + i, sep->chars, sep->length) == 0) {
      Value item = str_value(copy_string(str->chars + start, i - start));
      push(item);  // GC Protection
      write_value_array(&seq->items, item);
      pop();  // The item
      start = i + sep->length;
    }
  }

  // Add the last part of the string aswell - same behavior as Js.
  // TODO (optimize): Maybe remove this? "123".split("3") -> ["12", ""], but without this it would be ["12"]
  Value item = str_value(copy_string(str->chars + start, str->length - start));
  push(item);  // GC Protection
  write_value_array(&seq->items, item);
  pop();  // The item

  pop();  // The seq
  return seq_value(seq);
}

/**
 * TYPENAME_STRING.trim() -> TYPENAME_STRING
 * @brief Returns a new TYPENAME_STRING with leading and trailing whitespace (' ', \\f, \\n, \\r, \\t, \\v) removed.
 */
static Value str_trim(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)

  ObjString* str = AS_STR(argv[0]);
  int start      = 0;
  int end        = str->length - 1;

  static bool whitespace_lookup[256] = {
      [' '] = true, ['\f'] = true, ['\n'] = true, ['\r'] = true, ['\t'] = true, ['\v'] = true,
  };

  while (start < str->length && whitespace_lookup[(uint8_t)str->chars[start]]) {
    start++;
  }

  while (end > start && whitespace_lookup[(uint8_t)str->chars[end]]) {
    end--;
  }

  ObjString* trimmed = copy_string(str->chars + start, end - start + 1);
  return str_value(trimmed);
}

/**
 * TYPENAME_STRING.SP_METHOD_HAS(subs: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Returns true if the TYPENAME_STRING contains the substring 'subs'.
 */
static Value str_has(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)
  NATIVE_CHECK_ARG_AT(1, vm.str_class)

  // Should align with prop_getter
  ObjString* str    = AS_STR(argv[0]);
  ObjString* substr = AS_STR(argv[1]);

  if (substr->length == 0) {
    return bool_value(true);
  }
  if (substr->length > str->length) {
    return bool_value(false);
  }

  for (int i = 0; i < str->length - substr->length + 1; i++) {
    if (strncmp(str->chars + i, substr->chars, substr->length) == 0) {
      return bool_value(true);
    }
  }

  return bool_value(false);
}

/**
 * TYPENAME_STRING.SP_METHOD_SLICE(start: TYPENAME_INT, end: TYPENAME_INT | TYPENAME_NIL) -> TYPENAME_STRING
 * @brief Returns a new TYPENAME_STRING containing the items from 'start' to 'end' ('end' is exclusive).
 * 'end' can be negative to count from the end of the TYPENAME_STRING. If 'start' is greater than or equal to 'end', an empty
 * TYPENAME_STRING is returned. If 'end' is TYPENAME_NIL, all items from 'start' to the end of the TYPENAME_STRING are included.
 */
static Value str_slice(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)
  NATIVE_CHECK_ARG_AT(1, vm.int_class)
  if (is_nil(argv[2])) {
    argv[2] = int_value(AS_STR(argv[0])->length);
  }
  NATIVE_CHECK_ARG_AT(2, vm.int_class)

  ObjString* str = AS_STR(argv[0]);
  int count      = str->length;

  if (count == 0) {
    return str_value(copy_string("", 0));
  }

  int start = (int)argv[1].as.integer;
  int end   = (int)argv[2].as.integer;

  // Handle negative indices
  if (start < 0) {
    start = count + start;
  }
  if (end < 0) {
    end = count + end;
  }

  // Clamp out-of-bounds indices
  if (start < 0) {
    start = 0;
  }
  if (end > count) {
    end = count;
  }

  // Handle invalid or 0 length ranges
  if (start >= end) {
    return str_value(copy_string("", 0));
  }

  char* start_ptr       = str->chars + start;
  int length            = end - start;
  ObjString* sliced_str = copy_string(start_ptr, length);
  return str_value(sliced_str);
}
