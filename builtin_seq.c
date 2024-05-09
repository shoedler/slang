#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

void finalize_builtin_seq_class() {
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

/**
 * TYPENAME_SEQ.SP_METHOD_CTOR(len: TYPENAME_INT) -> TYPENAME_SEQ
 * @brief Creates a new TYPENAME_NIL-initialized TYPENAME_SEQ of length 'len'.
 *
 * TYPENAME_SEQ.SP_METHOD_CTOR(tuple: TYPENAME_TUPLE) -> TYPENAME_SEQ
 * @brief Creates a new TYPENAME_SEQ from a TYPENAME_TUPLE of values.
 */
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, SP_METHOD_CTOR) {
  UNUSED(argc);
  if (IS_INT(argv[1])) {
    ValueArray items;
    init_value_array(&items);
    ObjSeq* seq = take_seq(&items);
    push(seq_value(seq));  // GC Protection

    int count = (int)argv[1].as.integer;
    for (int i = 0; i < count; i++) {
      write_value_array(&seq->items, nil_value());
    }

    pop();  // The seq
    return seq_value(seq);
  }

  if (IS_TUPLE(argv[1])) {
    ObjTuple* tuple = AS_TUPLE(argv[1]);

    ValueArray items = prealloc_value_array(tuple->items.count);

    // We can use memcpy here because the items array is already preallocated
    memcpy(items.values, tuple->items.values, tuple->items.count * sizeof(Value));

    ObjSeq* seq = take_seq(&items);
    return seq_value(seq);
  }

  // TODO: Make a macro for this error message
  runtime_error("Expected argument 0 of type " STR(TYPENAME_INT) " or " STR(TYPENAME_TUPLE) " but got %s.",
                argv[1].type->name->chars);
  return nil_value();
}

#define BUILTIN_ENUMERABLE_GET_VALUE_ARRAY(value) items = AS_SEQ(value)->items
#define BUILTIN_LISTLIKE_NEW_EMPTY() seq_value(new_seq())
#define BUILTIN_LISTLIKE_TAKE_ARRAY(value_array) seq_value(take_seq(&value_array))
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

/**
 * TYPENAME_SEQ.push(arg1: TYPENAME_VALUE, ...args: TYPENAME_VALUE) -> TYPENAME_NIL
 * @brief Pushes one or many values to a TYPENAME_SEQ.
 */
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, push) {
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  for (int i = 1; i <= argc; i++) {
    write_value_array(&seq->items, argv[i]);
  }
  return nil_value();
}

/**
 * TYPENAME_SEQ.pop() -> TYPENAME_VALUE
 * @brief Pops and returns the last item of a TYPENAME_SEQ. Returns TYPENAME_NIL if it is empty.
 */
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, pop) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  return pop_value_array(&seq->items);  // Does bounds checking
}

/**
 * TYPENAME_SEQ.remove_at(index: TYPENAME_INT) -> TYPENAME_VALUE
 * @brief Removes and returns the item at 'index' from a TYPENAME_SEQ. Returns TYPENAME_NIL if 'index' is out of bounds.
 * Modifies the TYPENAME_SEQ.
 */
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, remove_at) {
  UNUSED(argc);
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT(1, INT)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  long long index = argv[1].as.integer;

  if (index > INT32_MAX || index < INT32_MIN) {
    runtime_error("Index %lld surpasses the maximum value of %d.", index, INT32_MAX);
    return nil_value();
  }

  return remove_at_value_array(&seq->items, (int)index);  // Does bounds checking
}