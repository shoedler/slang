#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_seq_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_SEQ, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, __ctor, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, to_str, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, len, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, push, -1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, pop, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, has, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, first, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, last, 1);
}

// Built-in seq constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ __ctor,
    /* Arguments   */ DOC_ARG("len", TYPENAME_NUMBER),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Creates a new " STR(TYPENAME_NIL) "-initialized " STR(TYPENAME_SEQ) " of length 'len'.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, __ctor) {
  BUILTIN_CHECK_ARG_AT(1, NUMBER)

  ValueArray items;
  init_value_array(&items);
  ObjSeq* seq = take_seq(&items);
  push(OBJ_VAL(seq));  // GC Protection

  int count = (int)AS_NUMBER(argv[1]);
  for (int i = 0; i < count; i++) {
    write_value_array(&seq->items, NIL_VAL);
  }

  pop();  // The seq
  return OBJ_VAL(seq);
}

// Built-in method to convert a sequence to a string
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ to_str,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a string representation of a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, to_str) {
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);

  strcpy(chars, VALUE_STR_SEQ_START);
  for (int i = 0; i < seq->items.count; i++) {
    // Use the default to-string method of the value to convert the item to a string
    push(seq->items.values[i]);  // Push the receiver (item at i) for to_str
    ObjString* item_str = AS_STRING(exec_method(copy_string("to_str", 6), 0));

    // Expand chars to fit the separator plus the next item
    size_t new_buf_size =
        strlen(chars) + item_str->length + (sizeof(VALUE_STR_SEQ_DELIM) - 1) +
        (sizeof(VALUE_STR_SEQ_END) - 1);  // Consider the closing bracket -  if we're done after this
                                          // iteration we won't need to expand and can just slap it on there
    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size = new_buf_size;
      chars    = realloc(chars, buf_size);
      if (chars == NULL) {
        return OBJ_VAL(copy_string("[???]", 5));
      }
    }

    // Append the string
    strcat(chars, item_str->chars);
    if (i < seq->items.count - 1) {
      strcat(chars, VALUE_STR_SEQ_DELIM);
    }
  }

  strcat(chars, VALUE_STR_SEQ_END);

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(
      chars,
      (int)strlen(chars));  // TODO (optimize): Use buf_size here, but
                            // we need to make sure that the string is
                            // null-terminated. Also, if it's < 64 chars long, we need to shorten the length.
  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in method to retrieve the length of a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ len,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */ "Returns the length of a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, len) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)

  int length = AS_SEQ(argv[0])->items.count;
  return NUMBER_VAL(length);
}

// Built-in method to push an arbitrary amount of values onto a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ push,
    /* Arguments   */ DOC_ARG("arg1", TYPENAME_OBJ) DOC_ARG_SEP DOC_ARG_REST,
    /* Return Type */ TYPENAME_NIL,
    /* Description */ "Pushes one or many values to a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, push) {
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  for (int i = 1; i <= argc; i++) {
    write_value_array(&seq->items, argv[i]);
  }
  return NIL_VAL;
}

// Built-in method to pop a value from a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ pop,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_OBJ,
    /* Description */
    "Pops and returns the last item of a " STR(TYPENAME_SEQ) ". Returns " STR(
        TYPENAME_NIL) " if it is empty.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, pop) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  return pop_value_array(&seq->items);
}

// Built-in method to check if a sequence contains a value
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ has,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE " if the " STR(TYPENAME_SEQ) " contains an item which equals 'value'.")
BUILTIN_METHOD_DOC_OVERLOAD(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ has,
    /* Arguments   */ DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE
    " if the " STR(TYPENAME_SEQ) " contains an item for which 'pred' evaulates to " VALUE_STR_TRUE ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, has) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return BOOL_VAL(false);
  }

  if (IS_CALLABLE(argv[1])) {
    // Function predicate
    for (int i = 0; i < seq->items.count; i++) {
      push(argv[1]);               // Push the function
      push(seq->items.values[i]);  // Push the item
      Value result = exec_fn(AS_OBJ(argv[1]), 1);
      // We could also check for truthiness here,
      if (IS_BOOL(result) && AS_BOOL(result)) {
        return BOOL_VAL(true);
      }
    }
  } else {
    // Value equality
    for (int i = 0; i < seq->items.count; i++) {
      if (values_equal(argv[1], seq->items.values[i])) {
        return BOOL_VAL(true);
      }
    }
  }

  return BOOL_VAL(false);
}

// Built-in method to retrieve the first item of a sequence
BUILTIN_METHOD_DOC(
  /* Receiver    */ TYPENAME_SEQ,
  /* Name        */ first, 
  /* Arguments   */ DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), 
  /* Return Type */ TYPENAME_OBJ, 
  /* Description */ 
    "Returns the first item of a " STR(
        TYPENAME_SEQ) " for which 'pred' evaluates to " VALUE_STR_TRUE
                      ". Returns " STR(TYPENAME_NIL) " if the " STR(
                          TYPENAME_SEQ) " is empty or no item satisfies the predicate.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, first) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  // Function predicate
  for (int i = 0; i < seq->items.count; i++) {
    push(argv[1]);               // Push the function
    push(seq->items.values[i]);  // Push the item
    Value result = exec_fn(AS_OBJ(argv[1]), 1);
    // We could also check for truthiness here,
    if (IS_BOOL(result) && AS_BOOL(result)) {
      return seq->items.values[i];
    }
  }

  return NIL_VAL;
}

// Built-in method to retrieve the last item of a sequence
BUILTIN_METHOD_DOC(  
  /* Receiver    */ TYPENAME_SEQ,
  /* Name        */ last, 
  /* Arguments   */ DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), 
  /* Return Type */ TYPENAME_OBJ, 
  /* Description */ 
    "Returns the last item of a " STR(
        TYPENAME_SEQ) " for which 'pred' evaluates to " VALUE_STR_TRUE
                      ". Returns " STR(TYPENAME_NIL) " if the " STR(
                          TYPENAME_SEQ) " is empty or no item satisfies the predicate.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, last) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  // Function predicate
  for (int i = seq->items.count - 1; i >= 0; i--) {
    push(argv[1]);               // Push the function
    push(seq->items.values[i]);  // Push the item
    Value result = exec_fn(AS_OBJ(argv[1]), 1);
    // We could also check for truthiness here,
    if (IS_BOOL(result) && AS_BOOL(result)) {
      return seq->items.values[i];
    }
  }

  return NIL_VAL;
}