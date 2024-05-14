#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool tuple_get_prop(Value receiver, ObjString* name, Value* result);
static bool tuple_get_subs(Value receiver, Value index, Value* result);

static Value tuple_ctor(int argc, Value argv[]);
static Value tuple_to_str(int argc, Value argv[]);
static Value tuple_has(int argc, Value argv[]);
static Value tuple_slice(int argc, Value argv[]);
static Value tuple_index_of(int argc, Value argv[]);
static Value tuple_first(int argc, Value argv[]);
static Value tuple_last(int argc, Value argv[]);
static Value tuple_each(int argc, Value argv[]);
static Value tuple_map(int argc, Value argv[]);
static Value tuple_filter(int argc, Value argv[]);
static Value tuple_join(int argc, Value argv[]);
static Value tuple_reverse(int argc, Value argv[]);
static Value tuple_every(int argc, Value argv[]);
static Value tuple_some(int argc, Value argv[]);
static Value tuple_reduce(int argc, Value argv[]);
static Value tuple_count(int argc, Value argv[]);
static Value tuple_concat(int argc, Value argv[]);

void finalize_native_tuple_class() {
  vm.tuple_class->get_property  = tuple_get_prop;
  vm.tuple_class->get_subscript = tuple_get_subs;

  define_native(&vm.tuple_class->methods, STR(SP_METHOD_CTOR), tuple_ctor, 1);
  define_native(&vm.tuple_class->methods, STR(SP_METHOD_TO_STR), tuple_to_str, 0);
  define_native(&vm.tuple_class->methods, STR(SP_METHOD_HAS), tuple_has, 1);
  define_native(&vm.tuple_class->methods, STR(SP_METHOD_SLICE), tuple_slice, 2);
  define_native(&vm.tuple_class->methods, "index_of", tuple_index_of, 1);
  define_native(&vm.tuple_class->methods, "first", tuple_first, 1);
  define_native(&vm.tuple_class->methods, "last", tuple_last, 1);
  define_native(&vm.tuple_class->methods, "each", tuple_each, 1);
  define_native(&vm.tuple_class->methods, "map", tuple_map, 1);
  define_native(&vm.tuple_class->methods, "filter", tuple_filter, 1);
  define_native(&vm.tuple_class->methods, "join", tuple_join, 1);
  define_native(&vm.tuple_class->methods, "reverse", tuple_reverse, 0);
  define_native(&vm.tuple_class->methods, "every", tuple_every, 1);
  define_native(&vm.tuple_class->methods, "some", tuple_some, 1);
  define_native(&vm.tuple_class->methods, "reduce", tuple_reduce, 2);
  define_native(&vm.tuple_class->methods, "count", tuple_count, 1);
  define_native(&vm.tuple_class->methods, "concat", tuple_concat, 1);
  finalize_new_class(vm.tuple_class);
}

static bool tuple_get_prop(Value receiver, ObjString* name, Value* result) {
  NATIVE_LISTLIKE_GET_PROP_BODY()
}

static bool tuple_get_subs(Value receiver, Value index, Value* result) {
  NATIVE_LISTLIKE_GET_SUBS_BODY()
}

/**
 * TYPENAME_TUPLE.SP_METHOD_CTOR(seq: TYPENAME_SEQ) -> TYPENAME_TUPLE
 * @brief Creates a new TYPENAME_TUPLE from a TYPENAME_SEQ of values.
 */
static Value tuple_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_ARG_AT(1, SEQ)

  ObjSeq* seq = AS_SEQ(argv[1]);

  ValueArray items = prealloc_value_array(seq->items.count);

  // We can use memcpy here because the items array is already preallocated
  memcpy(items.values, seq->items.values, seq->items.count * sizeof(Value));

  ObjTuple* tuple = take_tuple(&items);
  return tuple_value(tuple);
}

#define NATIVE_ENUMERABLE_GET_VALUE_ARRAY(value) items = AS_TUPLE(value)->items
#define NATIVE_LISTLIKE_NEW_EMPTY() tuple_value(new_tuple())
#define NATIVE_LISTLIKE_TAKE_ARRAY(value_array) tuple_value(take_tuple(&value_array))
static Value tuple_has(int argc, Value argv[]) {
  NATIVE_ENUMERABLE_HAS_BODY(TUPLE);
}
static Value tuple_slice(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SLICE_BODY(TUPLE);
}
static Value tuple_to_str(int argc, Value argv[]) {
  NATIVE_LISTLIKE_TO_STR_BODY(TUPLE, VALUE_STR_TUPLE_START, VALUE_STR_TUPLE_DELIM, VALUE_STR_TUPLE_END);
}
static Value tuple_index_of(int argc, Value argv[]) {
  NATIVE_LISTLIKE_INDEX_OF_BODY(TUPLE);
}
static Value tuple_first(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FIRST_BODY(TUPLE);
}
static Value tuple_last(int argc, Value argv[]) {
  NATIVE_LISTLIKE_LAST_BODY(TUPLE);
}
static Value tuple_each(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EACH_BODY(TUPLE);
}
static Value tuple_map(int argc, Value argv[]) {
  NATIVE_LISTLIKE_MAP_BODY(TUPLE);
}
static Value tuple_filter(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FILTER_BODY(TUPLE);
}
static Value tuple_join(int argc, Value argv[]) {
  NATIVE_LISTLIKE_JOIN_BODY(TUPLE);
}
static Value tuple_reverse(int argc, Value argv[]) {
  NATIVE_LISTLIKE_REVERSE_BODY(TUPLE);
}
static Value tuple_every(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EVERY_BODY(TUPLE);
}
static Value tuple_some(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SOME_BODY(TUPLE);
}
static Value tuple_reduce(int argc, Value argv[]) {
  NATIVE_LISTLIKE_REDUCE_BODY(TUPLE);
}
static Value tuple_count(int argc, Value argv[]) {
  NATIVE_LISTLIKE_COUNT_BODY(TUPLE);
}
static Value tuple_concat(int argc, Value argv[]) {
  NATIVE_LISTLIKE_CONCAT_BODY(TUPLE);
}
#undef NATIVE_ENUMERABLE_GET_VALUE_ARRAY
#undef NATIVE_LISTLIKE_NEW_EMPTY
#undef NATIVE_LISTLIKE_TAKE_ARRAY