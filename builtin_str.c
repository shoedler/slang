#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_str_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_STRING, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_LEN, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, split, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, trim, 0);
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
  BUILTIN_ARGC_EXACTLY(1);
  // Execute the to_str method on the argument
  push(argv[1]);  // Push the receiver for to_str, which is the ctors' argument
  Value result = exec_fn((Obj*)vm.special_field_names[SPECIAL_METHOD_TO_STR], 0);  // Convert to string
  if (vm.flags & VM_FLAG_HAS_ERROR) {
    return NIL_VAL;
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
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(STRING)

  return argv[0];
}

// Built-in method to retrieve the length of a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_LEN,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_STRING) ".");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_LEN) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(STRING)

  int length = AS_STRING(argv[0])->length;
  return NUMBER_VAL(length);
}

BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ split,
    /* Arguments   */ DOC_ARG("sep", TYPENAME_STRING),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Splits a " STR(TYPENAME_STRING) " into a " STR(TYPENAME_SEQ) " of substrings, using 'sep' as the delimiter.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, split) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(STRING)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjString* str = AS_STRING(argv[0]);
  ObjString* sep = AS_STRING(argv[1]);

  // If the separator is empty, split by character
  if (sep->length == 0) {
    ObjSeq* seq = prealloc_seq(str->length);
    push(OBJ_VAL(seq));  // GC Protection
    for (int i = 0; i < str->length; i++) {
      seq->items.values[i] = OBJ_VAL(copy_string(str->chars + i, 1));
    }

    return pop();  // The seq
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
  // TODO (optimize): Maybe remove this? "123".split("3") -> ["12", ""], but without this it would be ["12"]
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
  BUILTIN_ARGC_EXACTLY(0)
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
