#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void finalize_builtin_str_class() {
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_SLICE, 2);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, split, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, trim, 0);

  BUILTIN_FINALIZE_CLASS(TYPENAME_STRING);
}

// Built-in string constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Converts the first argument to a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_CTOR) {
  UNUSED(argc);
  // Execute the to_str method on the argument
  push(argv[1]);  // Push the receiver for to_str, which is the ctors' argument
  Value result = exec_callable(fn_value(argv[1].type->__to_str), 0);  // Convert to string
  if (vm.flags & VM_FLAG_HAS_ERROR) {
    return nil_value();
  }

  return result;
}

// Built-in method to convert a string to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_TO_STR) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(STRING)

  return argv[0];
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ split,
    /* Arguments   */ DOC_ARG("sep", TYPENAME_STRING),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Splits a " STR(TYPENAME_STRING) " into a " STR(TYPENAME_SEQ) " of substrings, using 'sep' as the delimiter.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, split) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(STRING)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* str = AS_STRING(argv[0]);
  ObjString* sep = AS_STRING(argv[1]);

  // If the separator is empty, split by character
  if (sep->length == 0) {
    ValueArray items = prealloc_value_array(str->length);
    ObjSeq* seq      = take_seq(&items);  // We can already take the seq, because seqs don't calculate the hash upon taking.
    push(seq_value(seq));                 // GC Protection
    for (int i = 0; i < str->length; i++) {
      seq->items.values[i] = str_value(copy_string(str->chars + i, 1));
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

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ trim,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a new " STR(TYPENAME_STRING) " with leading and trailing whitespace removed.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, trim) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(STRING)

  ObjString* str = AS_STRING(argv[0]);
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

// Built-in method to check if a value has a property
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("subs", TYPENAME_STRING),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Returns true if the " STR(TYPENAME_STRING) " contains the substring 'subs'.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_HAS) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(STRING)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  // Should align with prop_getter
  ObjString* str    = AS_STRING(argv[0]);
  ObjString* substr = AS_STRING(argv[1]);

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

// Builtin method to slice a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_SLICE,
    /* Arguments   */ DOC_ARG("start", TYPENAME_INT) DOC_ARG_SEP DOC_ARG("end", TYPENAME_INT | TYPENAME_NIL),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Returns a new " STR(TYPENAME_STRING) " containing the items from 'start' to 'end' ('end' is exclusive)."
    " 'end' can be negative to count from the end of the " STR(TYPENAME_STRING) ". If 'start' is greater than or equal to 'end', an empty "
    STR(TYPENAME_STRING) " is returned. If 'end' is " STR(TYPENAME_NIL) ", all items from 'start' to the end of the " STR(
        TYPENAME_STRING) " are included.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_SLICE) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(STRING)
  BUILTIN_CHECK_ARG_AT(1, INT)
  if (IS_NIL(argv[2])) {
    argv[2] = int_value(AS_STRING(argv[0])->length);
  }
  BUILTIN_CHECK_ARG_AT(2, INT)

  ObjString* str = AS_STRING(argv[0]);
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