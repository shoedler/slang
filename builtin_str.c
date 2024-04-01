#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_str_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_STRING, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, __ctor);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, to_str);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, len);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, split);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, trim);
}

// Built-in string constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG_OPT("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Converts the first argument to a " STR(
        TYPENAME_STRING) ", if provided. Otherwise, returns a default " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, __ctor) {
  if (argc == 0) {
    return OBJ_VAL(copy_string("", 0));
  }

  BUILTIN_CHECK_ARGC_ONE()

  push(argv[1]);  // Push the receiver for to_str, which is the ctors' argument
  return exec_method(copy_string("to_str", 6), 0);  // Convert to string
}

// Built-in method to convert a string to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, to_str) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(STRING)

  return argv[0];
}

// Built-in method to retrieve the length of a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ len,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, len) {
  BUILTIN_CHECK_ARGC_ZERO()
  BUILTIN_CHECK_RECEIVER(STRING)

  int length = AS_STRING(argv[0])->length;
  return NUMBER_VAL(length);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ split,
    /* Arguments   */ DOC_ARG_OPT("sep", TYPENAME_STRING),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */  
 "Splits a " STR(TYPENAME_STRING) " into a " STR(TYPENAME_SEQ) 
 " of substrings, using 'sep' as the delimiter. If 'sep' is not provided, the " STR(TYPENAME_STRING) 
 " is split into individual characters.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, split) {
  BUILTIN_CHECK_RECEIVER(STRING)

  // Split into individual characters if no separator is provided
  if (argc == 0) {
  split_by_char:
    ObjString* str = AS_STRING(argv[0]);

    ObjSeq* seq = prealloc_seq(str->length);
    push(OBJ_VAL(seq));  // GC Protection
    for (int i = 0; i < str->length; i++) {
      seq->items.values[i] = OBJ_VAL(copy_string(str->chars + i, 1));
    }
    pop();  // The seq

    return OBJ_VAL(seq);
  }

  // Split by separator
  BUILTIN_CHECK_ARGC_ONE()
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* str = AS_STRING(argv[0]);
  ObjString* sep = AS_STRING(argv[1]);

  // If the separator is empty, split by character
  if (sep->length == 0) {
    goto split_by_char;
  }

  ObjSeq* seq = new_seq();
  push(OBJ_VAL(seq));  // GC Protection

  // Split the string by looking for the separator at each character
  int start = 0;
  for (int i = 0; i < str->length; i++) {
    if (strncmp(str->chars + i, sep->chars, sep->length) == 0) {
      Value item = OBJ_VAL(copy_string(str->chars + start, i - start));
      push(item);  // GC Protection
      write_value_array(&seq->items, item);
      pop();  // The item
      start = i + sep->length;
    }
  }

  // Add the last part of the string aswell - same behavior as Js.
  // TODO (optimize): Maybe remove this? "123".split("3") -> ["12", ""], but withour this it would be ["12"]
  Value item = OBJ_VAL(copy_string(str->chars + start, str->length - start));
  push(item);  // GC Protection
  write_value_array(&seq->items, item);
  pop();  // The item

  pop();  // The seq
  return OBJ_VAL(seq);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ trim,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Returns a new " STR(TYPENAME_STRING) " with leading and trailing whitespace removed.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, trim) {
  BUILTIN_CHECK_ARGC_ZERO()
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
  return OBJ_VAL(trimmed);
}
