#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void register_builtin_tuple_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_TUPLE, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, SP_METHOD_SLICE, 2);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, index_of, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, first, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, last, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, each, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, map, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, filter, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, join, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, reverse, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, every, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, some, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, reduce, 2);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, count, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_TUPLE, concat, 1);
  BUILTIN_FINALIZE_CLASS(TYPENAME_TUPLE);
}

// Built-in tuple constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_TUPLE,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("seq", TYPENAME_SEQ),
    /* Return Type */ TYPENAME_TUPLE,
    /* Description */
    "Creates a new " STR(TYPENAME_TUPLE) " from a " STR(TYPENAME_SEQ) " of values.");
BUILTIN_METHOD_IMPL(TYPENAME_TUPLE, SP_METHOD_CTOR) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_ARG_AT(1, SEQ)

  ObjSeq* seq = AS_SEQ(argv[1]);

  ValueArray items = prealloc_value_array(seq->items.count);

  // We can use memcpy here because the items array is already preallocated
  memcpy(items.values, seq->items.values, seq->items.count * sizeof(Value));

  ObjTuple* tuple = take_tuple(&items);
  return OBJ_VAL(tuple);
}

#define BUILTIN_ENUMERABLE_GET_VALUE_ARRAY(value) items = AS_TUPLE(value)->items
#define BUILTIN_LISTLIKE_NEW_EMPTY() new_tuple()
#define BUILTIN_LISTLIKE_TAKE_ARRAY(value_array) take_tuple(&value_array)
BUILTIN_ENUMERABLE_HAS(TUPLE, "an item")
BUILTIN_LISTLIKE_SLICE(TUPLE)
BUILTIN_LISTLIKE_TO_STR(TUPLE, VALUE_STR_TUPLE_START, VALUE_STR_TUPLE_DELIM, VALUE_STR_TUPLE_END)
BUILTIN_LISTLIKE_INDEX_OF(TUPLE)
BUILTIN_LISTLIKE_FIRST(TUPLE)
BUILTIN_LISTLIKE_LAST(TUPLE)
BUILTIN_LISTLIKE_EACH(TUPLE)
BUILTIN_LISTLIKE_MAP(TUPLE)
BUILTIN_LISTLIKE_FILTER(TUPLE)
BUILTIN_LISTLIKE_JOIN(TUPLE)
BUILTIN_LISTLIKE_REVERSE(TUPLE)
BUILTIN_LISTLIKE_EVERY(TUPLE)
BUILTIN_LISTLIKE_SOME(TUPLE)
BUILTIN_LISTLIKE_REDUCE(TUPLE)
BUILTIN_LISTLIKE_COUNT(TUPLE)
BUILTIN_LISTLIKE_CONCAT(TUPLE)
#undef BUILTIN_ENUMERABLE_GET_VALUE_ARRAY
#undef BUILTIN_LISTLIKE_NEW_EMPTY
#undef BUILTIN_LISTLIKE_TAKE_ARRAY