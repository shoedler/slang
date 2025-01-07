#include <stdint.h>
#include <string.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool str_get_prop(Value receiver, ObjString* name, Value* result);
static bool str_get_subs(Value receiver, Value index, Value* result);
static bool str_eq(Value self, Value other);

static Value str_ctor(int argc, Value argv[]);
static Value str_to_str(int argc, Value argv[]);
static Value str_has(int argc, Value argv[]);
static Value str_slice(int argc, Value argv[]);
static Value str_add(int argc, Value argv[]);
static Value str_lt(int argc, Value argv[]);
static Value str_gt(int argc, Value argv[]);
static Value str_lteq(int argc, Value argv[]);
static Value str_gteq(int argc, Value argv[]);

static Value str_split(int argc, Value argv[]);
static Value str_trim(int argc, Value argv[]);
static Value str_ints(int argc, Value argv[]);
static Value str_ascii(int argc, Value argv[]);
static Value str_ascii_at(int argc, Value argv[]);
static Value str_chars(int argc, Value argv[]);
static Value str_reps(int argc, Value argv[]);
static Value str_from_ascii(int argc, Value argv[]);

ObjClass* native_str_class_partial_init() {
  ObjClass* str_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  str_class->__get_prop = str_get_prop;
  str_class->__set_prop = native_set_prop_not_supported;  // Not supported
  str_class->__get_subs = str_get_subs;
  str_class->__set_subs = native_set_subs_not_supported;  // Not supported
  str_class->__equals   = str_eq;
  str_class->__hash     = native_default_obj_hash;

  return str_class;
}

void native_str_class_finalize() {
  define_native(&vm.str_class->methods, STR(SP_METHOD_CTOR), str_ctor, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_TO_STR), str_to_str, 0);
  define_native(&vm.str_class->methods, STR(SP_METHOD_HAS), str_has, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_SLICE), str_slice, 2);
  define_native(&vm.str_class->methods, STR(SP_METHOD_ADD), str_add, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_SUB), native___sub_not_supported, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_MUL), native___mul_not_supported, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_DIV), native___div_not_supported, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_MOD), native___mod_not_supported, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_LT), str_lt, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_GT), str_gt, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_LTEQ), str_lteq, 1);
  define_native(&vm.str_class->methods, STR(SP_METHOD_GTEQ), str_gteq, 1);

  define_native(&vm.str_class->methods, "split", str_split, 1);
  define_native(&vm.str_class->methods, "trim", str_trim, 0);
  define_native(&vm.str_class->methods, "ints", str_ints, 0);
  define_native(&vm.str_class->methods, "ascii", str_ascii, 0);
  define_native(&vm.str_class->methods, "ascii_at", str_ascii_at, 1);
  define_native(&vm.str_class->methods, "chars", str_chars, 0);
  define_native(&vm.str_class->methods, "reps", str_reps, 1);

  define_native(&vm.str_class->static_methods, "from_ascii", str_from_ascii, 1);
  finalize_new_class(vm.str_class);
}

static bool str_eq(Value self, Value other) {
  return self.type == other.type && AS_STR(self) == AS_STR(other);  // Works because strings are interned
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
  vm_push(argv[1]);  // Push the receiver for to_str, which is the ctors' argument
  Value result = vm_exec_callable(fn_value(argv[1].type->__to_str), 0);  // Convert to string
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
    ValueArray items = value_array_init_of_size(str->length);
    ObjSeq* seq      = take_seq(&items);  // We can already take the seq, because seqs don't calculate the hash upon taking.
    vm_push(seq_value(seq));              // GC Protection
    for (int i = 0; i < str->length; i++) {
      seq->items.values[i] = str_value(copy_string(str->chars + i, 1));
      seq->items.count++;
    }

    return vm_pop();  // The seq
  }

  ObjSeq* seq = new_seq();
  vm_push(seq_value(seq));  // GC Protection

  // Split the string by looking for the separator at each character
  int start = 0;
  for (int i = 0; i < str->length; i++) {
    if (strncmp(str->chars + i, sep->chars, sep->length) == 0) {
      // Only create a new string if we have content between the separators
      if (i >= start) {
        Value item = str_value(copy_string(str->chars + start, i - start));
        vm_push(item);  // GC Protection
        value_array_write(&seq->items, item);
        vm_pop();  // The item
      }
      start = i + sep->length;
      i     = start - 1;  // -1 because the loop will increment it
    }
  }

  // Add the last part of the string aswell - same behavior as Js and Python
  Value item = str_value(copy_string(str->chars + start, str->length - start));
  vm_push(item);  // GC Protection
  value_array_write(&seq->items, item);
  vm_pop();  // The item

  vm_pop();  // The seq
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

/**
 * TYPENAME_STRING.SP_METHOD_ADD(other: TYPENAME_VALUE) -> TYPENAME_STRING
 * @brief Concatenates two TYPENAME_STRINGs, if [other] isn't a TYPENAME_STRING, it's SP_METHOD_TO_STR is called.
 */
static Value str_add(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)

  Value other = argv[1];

  if (!is_str(other)) {
    vm_push(other);  // Receiver
    other = vm_exec_callable(fn_value(other.type->__to_str), 0);
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {
      return nil_value();
    }
  }

  vm_push(argv[0]);  // GC Protection
  vm_push(other);    // GC Protection
  vm_concatenate();
  return vm_pop();  // The new string
}

#define STR_COMPARATOR(name, op, length_op)                                       \
  static Value name(int argc, Value argv[]) {                                     \
    UNUSED(argc);                                                                 \
    NATIVE_CHECK_RECEIVER(vm.str_class)                                           \
    NATIVE_CHECK_ARG_AT(1, vm.str_class)                                          \
                                                                                  \
    ObjString* left  = AS_STR(argv[0]);                                           \
    ObjString* right = AS_STR(argv[1]);                                           \
                                                                                  \
    int min_length = left->length < right->length ? left->length : right->length; \
    int cmp        = strncmp(left->chars, right->chars, min_length);              \
                                                                                  \
    if (cmp == 0) {                                                               \
      return bool_value(left->length length_op right->length);                    \
    }                                                                             \
                                                                                  \
    return bool_value(cmp op 0);                                                  \
  }

/**
 * TYPENAME_STRING.SP_METHOD_LT(other: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Compares two TYPENAME_STRINGs lexicographically.
 */
STR_COMPARATOR(str_lt, <, <)
/**
 * TYPENAME_STRING.SP_METHOD_GT(other: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Compares two TYPENAME_STRINGs lexicographically.
 */
STR_COMPARATOR(str_gt, >, >)
/**
 * TYPENAME_STRING.SP_METHOD_LTEQ(other: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Compares two TYPENAME_STRINGs lexicographically.
 */
STR_COMPARATOR(str_lteq, <, <=)
/**
 * TYPENAME_STRING.SP_METHOD_GTEQ(other: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Compares two TYPENAME_STRINGs lexicographically.
 */
STR_COMPARATOR(str_gteq, >, >=)
#undef STR_COMPARATOR

static bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

/**
 * TYPENAME_STRING.ints() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ containing the integers found in the TYPENAME_STRING.
 * Everything except a digit and a unary minus is considered a separator. Leading and trailing separators are ignored.
 * E.g. "12a34" -> [12, 34], "1.2" -> [1, 2], "hello1234 " -> [1234], "32-23" -> [32, -23]
 */
static Value str_ints(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class);

  ObjString* str = AS_STR(argv[0]);
  ObjSeq* seq    = new_seq();
  vm_push(seq_value(seq));  // GC Protection

  int i = 0;
  while (i < str->length) {
    // Skip non-digit and non-minus characters
    while (i < str->length && !is_digit(str->chars[i]) && str->chars[i] != '-') {
      i++;
    }

    if (i == str->length) {
      break;
    }

    // Check for unary minus
    bool is_negative = false;
    if (str->chars[i] == '-') {
      is_negative = true;
      i++;
    }

    // Collect digits
    char* num = str->chars + i;
    while (i < str->length && is_digit(str->chars[i])) {
      i++;
    }

    // Null-terminate the number string temporarily
    char c        = str->chars[i];
    str->chars[i] = '\0';
    int value     = atoi(num);
    if (is_negative) {
      value = -value;
    }
    str->chars[i] = c;

    value_array_write(&seq->items, int_value(value));
  }

  vm_pop();  // The seq
  return seq_value(seq);
}

/**
 * TYPENAME_STRING.ascii() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ containing the ASCII values of the characters in the TYPENAME_STRING.
 */
static Value str_ascii(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class);

  ObjString* str   = AS_STR(argv[0]);
  ValueArray items = value_array_init_of_size(str->length);
  ObjSeq* seq      = take_seq(&items);  // We can already take the seq, because seqs don't calculate the hash upon taking.

  for (int i = 0; i < str->length; i++) {
    seq->items.values[i] = int_value((int)str->chars[i]);
  }
  seq->items.count = str->length;

  return seq_value(seq);
}

/**
 * TYPENAME_STRING.ascii_at(index: TYPENAME_INT) -> TYPENAME_INT
 * @brief Returns the ASCII value of the character at the given index in the TYPENAME_STRING. If the index is out of bounds, -1 is
 * returned.
 */
static Value str_ascii_at(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class);
  NATIVE_CHECK_ARG_AT(1, vm.int_class);

  ObjString* str = AS_STR(argv[0]);
  int index      = (int)argv[1].as.integer;

  if (index < 0 || index >= str->length) {
    return int_value(-1);
  }

  return int_value((int)str->chars[index]);
}

/**
 * TYPENAME_STRING.chars() -> TYPENAME_SEQ
 * @brief Returns a TYPENAME_SEQ containing individual characters of the TYPENAME_STRING.
 */
static Value str_chars(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class);

  ObjString* str   = AS_STR(argv[0]);
  ValueArray items = value_array_init_of_size(str->length);
  ObjSeq* seq      = take_seq(&items);  // We can already take the seq, because seqs don't calculate the hash upon taking.

  vm_push(seq_value(seq));  // GC Protection
  for (int i = 0; i < str->length; i++) {
    seq->items.values[i] = str_value(copy_string(str->chars + i, 1));
    seq->items.count++;
  }

  return vm_pop();  // The seq
}

/**
 * TYPENAME_STRING.reps(count: TYPENAME_INT) -> TYPENAME_STRING
 * @brief Returns a new TYPENAME_STRING containing 'count' repetitions of the original TYPENAME_STRING.
 */
static Value str_reps(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.str_class)
  NATIVE_CHECK_ARG_AT(1, vm.int_class)

  ObjString* str = AS_STR(argv[0]);
  int count      = (int)argv[1].as.integer;

  if (count <= 0) {
    return str_value(copy_string("", 0));
  }

  int new_length = str->length * count;
  char* new_str  = malloc(new_length + 1);
  for (int i = 0; i < count; i++) {
    memcpy(new_str + i * str->length, str->chars, str->length);
  }
  new_str[new_length] = '\0';

  return str_value(copy_string(new_str, new_length));
}

/**
 * static TYPENAME_STRING.from_ascii(code: TYPENAME_INT) -> TYPENAME_STRING
 * @brief Returns a new TYPENAME_STRING containing the character represented by the ASCII value 'code'.
 */
static Value str_from_ascii(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_ARG_AT(1, vm.int_class)

  int code = (int)argv[1].as.integer;
  if (code < 0 || code > 255) {
    vm_error("Invalid ASCII code %d.", code);
    return nil_value();
  }

  char c = (char)code;
  return str_value(copy_string(&c, 1));
}