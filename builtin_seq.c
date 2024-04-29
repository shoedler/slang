#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_seq_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_SEQ, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_SLICE, 2);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, push, -1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, pop, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, remove_at, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, index_of, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, first, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, last, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, each, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, map, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, filter, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, join, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, reverse, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, every, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, some, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, reduce, 2);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, count, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, concat, 1);

  BUILTIN_FINALIZE_CLASS(TYPENAME_SEQ);
}

// Built-in seq constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("len", TYPENAME_INT),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Creates a new " STR(TYPENAME_NIL) "-initialized " STR(TYPENAME_SEQ) " of length 'len'.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, SP_METHOD_CTOR) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, INT)

  ValueArray items;
  init_value_array(&items);
  ObjSeq* seq = take_seq(&items);
  push(OBJ_VAL(seq));  // GC Protection

  int count = (int)AS_INT(argv[1]);
  for (int i = 0; i < count; i++) {
    write_value_array(&seq->items, NIL_VAL);
  }

  pop();  // The seq
  return OBJ_VAL(seq);
}

#define BUILTIN_ENUMERABLE_GET_VALUE_ARRAY(value) items = AS_SEQ(value)->items
#define BUILTIN_LISTLIKE_NEW_EMPTY() new_seq()
#define BUILTIN_LISTLIKE_TAKE_ARRAY(value_array) take_seq(&value_array)
BUILTIN_ENUMERABLE_HAS(SEQ, "an item")
BUILTIN_LISTLIKE_SLICE(SEQ)
BUILTIN_LISTLIKE_TO_STR(SEQ, VALUE_STR_SEQ_START, VALUE_STR_SEQ_DELIM, VALUE_STR_SEQ_END)
BUILTIN_LISTLIKE_INDEX_OF(SEQ)
BUILTIN_LISTLIKE_FIRST(SEQ)
BUILTIN_LISTLIKE_LAST(SEQ)
BUILTIN_LISTLIKE_EACH(SEQ)
BUILTIN_LISTLIKE_MAP(SEQ)
BUILTIN_LISTLIKE_FILTER(SEQ)
BUILTIN_LISTLIKE_JOIN(SEQ)
BUILTIN_LISTLIKE_REVERSE(SEQ)
BUILTIN_LISTLIKE_EVERY(SEQ)
BUILTIN_LISTLIKE_SOME(SEQ)
BUILTIN_LISTLIKE_REDUCE(SEQ)
BUILTIN_LISTLIKE_COUNT(SEQ)
BUILTIN_LISTLIKE_CONCAT(SEQ)
#undef BUILTIN_ENUMERABLE_GET_VALUE_ARRAY
#undef BUILTIN_LISTLIKE_NEW_EMPTY
#undef BUILTIN_LISTLIKE_TAKE_ARRAY

// Built-in method to push an arbitrary amount of values onto a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ push,
    /* Arguments   */ DOC_ARG("arg1", TYPENAME_OBJ) DOC_ARG_SEP DOC_ARG_REST,
    /* Return Type */ TYPENAME_NIL,
    /* Description */ "Pushes one or many values to a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, push) {
  BUILTIN_ARGC_AT_LEAST(1)
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
    "Pops and returns the last item of a " STR(TYPENAME_SEQ) ". Returns " STR(TYPENAME_NIL) " if it is empty.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, pop) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  return pop_value_array(&seq->items);  // Does bounds checking
}

// Built-in method to remove a value from a sequence at a given index
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ remove_at,
    /* Arguments   */ DOC_ARG("index", TYPENAME_INT),
    /* Return Type */ TYPENAME_OBJ,
    /* Description */
    "Removes and returns the item at 'index' from a " STR(TYPENAME_SEQ) ". Returns " STR(
        TYPENAME_NIL) " if 'index' is out of bounds.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, remove_at) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT(1, INT)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  long long index = AS_INT(argv[1]);

  if (index > INT32_MAX || index < INT32_MIN) {
    runtime_error("Index %lld surpasses the maximum value of %d.", index, INT32_MAX);
    return NIL_VAL;
  }

  return remove_at_value_array(&seq->items, (int)index);  // Does bounds checking
}