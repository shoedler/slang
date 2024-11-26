#include <string.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
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

ObjClass* partial_init_native_tuple_class() {
  ObjClass* tuple_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  tuple_class->__get_prop = tuple_get_prop;
  tuple_class->__set_prop = native_set_prop_not_supported;  // Not supported
  tuple_class->__get_subs = tuple_get_subs;
  tuple_class->__set_subs = native_set_subs_not_supported;  // Not supported
  tuple_class->__equals   = native_default_obj_equals;
  tuple_class->__hash     = native_default_obj_hash;

  return tuple_class;
}

void finalize_native_tuple_class() {
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
#define NATIVE_LISTLIKE_GET_ARRAY(value) AS_TUPLE(value)->items
  NATIVE_LISTLIKE_GET_PROP_BODY()
#undef NATIVE_LISTLIKE_GET_ARRAY
}

static bool tuple_get_subs(Value receiver, Value index, Value* result) {
#define NATIVE_LISTLIKE_GET_ARRAY(value) AS_TUPLE(value)->items
  NATIVE_LISTLIKE_GET_SUBS_BODY()
#undef NATIVE_LISTLIKE_GET_ARRAY
}

/**
 * TYPENAME_TUPLE.SP_METHOD_CTOR(seq: TYPENAME_SEQ) -> TYPENAME_TUPLE
 * @brief Creates a new TYPENAME_TUPLE from a TYPENAME_SEQ of values.
 */
static Value tuple_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_ARG_AT(1, vm.seq_class)

  ObjSeq* seq      = AS_SEQ(argv[1]);
  ValueArray items = init_value_array_of_size(seq->items.count);

  // We can use memcpy here because the items array is already preallocated
  memcpy(items.values, seq->items.values, seq->items.count * sizeof(Value));
  items.count = seq->items.count;

  ObjTuple* tuple = take_tuple(&items);
  return tuple_value(tuple);
}

#define NATIVE_LISTLIKE_GET_ARRAY(value) AS_TUPLE(value)->items
#define NATIVE_LISTLIKE_NEW_EMPTY() tuple_value(new_tuple())
#define NATIVE_LISTLIKE_TAKE_ARRAY(value_array) tuple_value(take_tuple(&value_array))
static Value tuple_has(int argc, Value argv[]) {
  NATIVE_LISTLIKE_HAS_BODY(vm.tuple_class);
}
static Value tuple_slice(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SLICE_BODY(vm.tuple_class);
}
static Value tuple_to_str(int argc, Value argv[]) {
  NATIVE_LISTLIKE_TO_STR_BODY(vm.tuple_class, VALUE_STR_TUPLE_START, VALUE_STR_TUPLE_DELIM, VALUE_STR_TUPLE_END);
}
static Value tuple_index_of(int argc, Value argv[]) {
  NATIVE_LISTLIKE_INDEX_OF_BODY(vm.tuple_class);
}
static Value tuple_first(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FIRST_BODY(vm.tuple_class);
}
static Value tuple_last(int argc, Value argv[]) {
  NATIVE_LISTLIKE_LAST_BODY(vm.tuple_class);
}
static Value tuple_each(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EACH_BODY(vm.tuple_class);
}
static Value tuple_map(int argc, Value argv[]) {
  NATIVE_LISTLIKE_MAP_BODY(vm.tuple_class);
}
static Value tuple_filter(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FILTER_BODY(vm.tuple_class);
}
static Value tuple_join(int argc, Value argv[]) {
  NATIVE_LISTLIKE_JOIN_BODY(vm.tuple_class);
}
static Value tuple_reverse(int argc, Value argv[]) {
  NATIVE_LISTLIKE_REVERSE_BODY(vm.tuple_class);
}
static Value tuple_every(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EVERY_BODY(vm.tuple_class);
}
static Value tuple_some(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SOME_BODY(vm.tuple_class);
}
static Value tuple_reduce(int argc, Value argv[]) {
  NATIVE_LISTLIKE_REDUCE_BODY(vm.tuple_class);
}
static Value tuple_count(int argc, Value argv[]) {
  NATIVE_LISTLIKE_COUNT_BODY(vm.tuple_class);
}
static Value tuple_concat(int argc, Value argv[]) {
  NATIVE_LISTLIKE_CONCAT_BODY(vm.tuple_class);
}
#undef NATIVE_LISTLIKE_GET_ARRAY
#undef NATIVE_LISTLIKE_NEW_EMPTY
#undef NATIVE_LISTLIKE_TAKE_ARRAY