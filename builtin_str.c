#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool prop_getter(Obj* self, ObjString* name, Value* result);
static bool prop_setter(Obj* self, ObjString* name, Value value);
static bool index_getter(Obj* self, Value index, Value* result);
static bool index_setter(Obj* self, Value index, Value value);

void register_builtin_str_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_STRING, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, SP_METHOD_SLICE, 2);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, split, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_STRING, trim, 0);

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_STRING, prop_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_STRING, prop_setter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_STRING, index_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_STRING, index_setter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_STRING);
}

// Internal OP_GET_PROPERTY handler
static bool prop_getter(Obj* self, ObjString* name, Value* result) {
  if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {
    *result = NUMBER_VAL((double)((ObjString*)self)->length);
    return true;
  }
  if (bind_method(vm.__builtin_Str_class, name, result)) {
    return true;
  }

  return false;
}

// Internal OP_SET_PROPERTY handler
static bool prop_setter(Obj* self, ObjString* name, Value value) {
  UNUSED(self);
  UNUSED(name);
  UNUSED(value);
  return false;
}

// Internal OP_GET_INDEX handler
static bool index_getter(Obj* self, Value index, Value* result) {
  if (!IS_NUMBER(index)) {
    runtime_error(STR(TYPENAME_STRING) " indices must be " STR(TYPENAME_NUMBER) "s, but got %s.", typeof(index)->name->chars);
    return false;
  }

  double i_raw = AS_NUMBER(index);
  long long i;
  if (!is_int(i_raw, &i)) {
    *result = NIL_VAL;
    return true;
  }

  ObjString* string = (ObjString*)self;
  if (i < 0 || i >= string->length) {
    runtime_error("Index out of bounds. Was %lld, but this " STR(TYPENAME_STRING) " has length %d.", i, string->length);
    return false;
  }

  ObjString* char_str = copy_string(string->chars + i, 1);
  *result             = OBJ_VAL(char_str);
  return true;
}

// Internal OP_SET_INDEX handler
static bool index_setter(Obj* self, Value index, Value value) {
  UNUSED(self);
  UNUSED(index);
  UNUSED(value);
  runtime_error("Cannot set index on a " STR(TYPENAME_STRING) ".");
  return false;
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
  push(argv[1]);                                         // Push the receiver for to_str, which is the ctors' argument
  Value result = exec_fn(typeof(argv[1])->__to_str, 0);  // Convert to string
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

// Built-in method to check if a value has a property
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("subs", TYPENAME_STRING),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Returns true if the " STR(TYPENAME_STRING) " contains the substring 'subs'.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_HAS) {
  BUILTIN_CHECK_RECEIVER(STRING)
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  // Should align with prop_getter
  ObjString* str    = AS_STRING(argv[0]);
  ObjString* substr = AS_STRING(argv[1]);

  if (substr->length == 0) {
    return BOOL_VAL(true);
  }
  if (substr->length > str->length) {
    return BOOL_VAL(false);
  }

  for (int i = 0; i < str->length - substr->length + 1; i++) {
    if (strncmp(str->chars + i, substr->chars, substr->length) == 0) {
      return BOOL_VAL(true);
    }
  }

  return BOOL_VAL(false);
}

// Builtin method to slice a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_STRING,
    /* Name        */ SP_METHOD_SLICE,
    /* Arguments   */ DOC_ARG("start", TYPENAME_NUMBER) DOC_ARG_SEP DOC_ARG("end", TYPENAME_NUMBER | TYPENAME_NIL),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Returns a new " STR(TYPENAME_STRING) " containing the items from 'start' to 'end' ('end' is exclusive)."
    " 'end' can be negative to count from the end of the " STR(TYPENAME_STRING) ". If 'start' is greater than or equal to 'end', an empty "
    STR(TYPENAME_STRING) " is returned. If 'end' is " STR(TYPENAME_NIL) ", all items from 'start' to the end of the " STR(
        TYPENAME_STRING) " are included.");
BUILTIN_METHOD_IMPL(TYPENAME_STRING, SP_METHOD_SLICE) {
  BUILTIN_ARGC_EXACTLY(2)
  BUILTIN_CHECK_RECEIVER(STRING)
  BUILTIN_CHECK_ARG_AT(1, NUMBER)
  if (IS_NIL(argv[2])) {
    argv[2] = NUMBER_VAL(AS_STRING(argv[0])->length);
  }
  BUILTIN_CHECK_ARG_AT(2, NUMBER)

  ObjString* str = AS_STRING(argv[0]);
  int count      = str->length;

  if (count == 0) {
    return OBJ_VAL(copy_string("", 0));
  }

  double start_raw = AS_NUMBER(argv[1]);
  double end_raw   = AS_NUMBER(argv[2]);

  long long start;
  long long end;

  if (!is_int(start_raw, &start) || !is_int(end_raw, &end)) {
    runtime_error("Indices must be integers, but got floats.");
    return NIL_VAL;
  }

  if (start < 0) {
    start = count + start;
  }
  if (end < 0) {
    end = count + end;
  }

  if (start < 0 || start >= count || end < 0 || end > count) {
    runtime_error(
        "Slice indices out of bounds. Start resolved to %d and end to %d, but this " STR(TYPENAME_STRING) " has length %d.",
        start, end, count);
    return NIL_VAL;
  }

  if (start >= end) {
    return OBJ_VAL(copy_string("", 0));
  }

  char* start_ptr       = str->chars + start;
  int length            = end - start;
  ObjString* sliced_str = copy_string(start_ptr, length);
  return OBJ_VAL(sliced_str);
}