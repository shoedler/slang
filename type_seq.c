#include <stdint.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool seq_get_prop(Value receiver, ObjString* name, Value* result);
static bool seq_get_subs(Value receiver, Value index, Value* result);
static bool seq_set_subs(Value receiver, Value index, Value value);
NATIVE_SET_PROP_NOT_SUPPORTED()

static Value seq_ctor(int argc, Value argv[]);
static Value seq_to_str(int argc, Value argv[]);
static Value seq_has(int argc, Value argv[]);
static Value seq_slice(int argc, Value argv[]);
static Value seq_push(int argc, Value argv[]);
static Value seq_pop(int argc, Value argv[]);
static Value seq_remove_at(int argc, Value argv[]);
static Value seq_index_of(int argc, Value argv[]);
static Value seq_first(int argc, Value argv[]);
static Value seq_last(int argc, Value argv[]);
static Value seq_each(int argc, Value argv[]);
static Value seq_map(int argc, Value argv[]);
static Value seq_filter(int argc, Value argv[]);
static Value seq_join(int argc, Value argv[]);
static Value seq_reverse(int argc, Value argv[]);
static Value seq_every(int argc, Value argv[]);
static Value seq_some(int argc, Value argv[]);
static Value seq_reduce(int argc, Value argv[]);
static Value seq_count(int argc, Value argv[]);
static Value seq_concat(int argc, Value argv[]);

void finalize_native_seq_class() {
  vm.seq_class->__get_prop = seq_get_prop;
  vm.seq_class->__set_prop = set_prop_not_supported;  // Not supported
  vm.seq_class->__get_subs = seq_get_subs;
  vm.seq_class->__set_subs = seq_set_subs;

  define_native(&vm.seq_class->methods, STR(SP_METHOD_CTOR), seq_ctor, 1);
  define_native(&vm.seq_class->methods, STR(SP_METHOD_TO_STR), seq_to_str, 0);
  define_native(&vm.seq_class->methods, STR(SP_METHOD_HAS), seq_has, 1);
  define_native(&vm.seq_class->methods, STR(SP_METHOD_SLICE), seq_slice, 2);
  define_native(&vm.seq_class->methods, "push", seq_push, -1);
  define_native(&vm.seq_class->methods, "pop", seq_pop, 0);
  define_native(&vm.seq_class->methods, "remove_at", seq_remove_at, 1);
  define_native(&vm.seq_class->methods, "index_of", seq_index_of, 1);
  define_native(&vm.seq_class->methods, "first", seq_first, 1);
  define_native(&vm.seq_class->methods, "last", seq_last, 1);
  define_native(&vm.seq_class->methods, "each", seq_each, 1);
  define_native(&vm.seq_class->methods, "map", seq_map, 1);
  define_native(&vm.seq_class->methods, "filter", seq_filter, 1);
  define_native(&vm.seq_class->methods, "join", seq_join, 1);
  define_native(&vm.seq_class->methods, "reverse", seq_reverse, 0);
  define_native(&vm.seq_class->methods, "every", seq_every, 1);
  define_native(&vm.seq_class->methods, "some", seq_some, 1);
  define_native(&vm.seq_class->methods, "reduce", seq_reduce, 2);
  define_native(&vm.seq_class->methods, "count", seq_count, 1);
  define_native(&vm.seq_class->methods, "concat", seq_concat, 1);
  finalize_new_class(vm.seq_class);
}

static bool seq_get_prop(Value receiver, ObjString* name, Value* result) {
  NATIVE_LISTLIKE_GET_PROP_BODY()
}

static bool seq_get_subs(Value receiver, Value index, Value* result) {
  NATIVE_LISTLIKE_GET_SUBS_BODY()
}

static bool seq_set_subs(Value receiver, Value index, Value value) {
  if (!is_int(index)) {
    runtime_error("Type %s does not support set-subscripting with %s. Expected " STR(TYPENAME_INT) ".",
                  receiver.type->name->chars, index.type->name->chars);
    return false;
  }

  long long idx = index.as.integer;
  ObjSeq* seq   = AS_SEQ(receiver);

  if (idx < 0 || idx >= seq->items.count) {
    runtime_error("Index out of bounds. Was %d, but this " STR(TYPENAME_SEQ) " has length %d.", idx, seq->items.count);
    return false;
  }

  seq->items.values[idx] = value;
  return true;
}

/**
 * TYPENAME_SEQ.SP_METHOD_CTOR(len: TYPENAME_INT) -> TYPENAME_SEQ
 * @brief Creates a new TYPENAME_NIL-initialized TYPENAME_SEQ of length 'len' with all items set to TYPENAME_NIL.
 *
 * TYPENAME_SEQ.SP_METHOD_CTOR(tuple: TYPENAME_TUPLE) -> TYPENAME_SEQ
 * @brief Creates a new TYPENAME_SEQ from a TYPENAME_TUPLE of values.
 */
static Value seq_ctor(int argc, Value argv[]) {
  UNUSED(argc);
  if (is_int(argv[1])) {
    ValueArray items;
    init_value_array(&items);
    ObjSeq* seq = take_seq(&items);  // TODO (optimize): Use prealloc_value_array
    push(seq_value(seq));            // GC Protection

    int count = (int)argv[1].as.integer;
    for (int i = 0; i < count; i++) {
      write_value_array(&seq->items, nil_value());
    }

    pop();  // The seq
    return seq_value(seq);
  }

  if (is_tuple(argv[1])) {
    ObjTuple* tuple  = AS_TUPLE(argv[1]);
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

#define NATIVE_ENUMERABLE_GET_VALUE_ARRAY(value) items = AS_SEQ(value)->items
#define NATIVE_LISTLIKE_NEW_EMPTY() seq_value(new_seq())
#define NATIVE_LISTLIKE_TAKE_ARRAY(value_array) seq_value(take_seq(&value_array))
static Value seq_has(int argc, Value argv[]) {
  NATIVE_ENUMERABLE_HAS_BODY(vm.seq_class);
}
static Value seq_slice(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SLICE_BODY(vm.seq_class);
}
static Value seq_to_str(int argc, Value argv[]) {
  NATIVE_LISTLIKE_TO_STR_BODY(vm.seq_class, VALUE_STR_SEQ_START, VALUE_STR_SEQ_DELIM, VALUE_STR_SEQ_END);
}
static Value seq_index_of(int argc, Value argv[]) {
  NATIVE_LISTLIKE_INDEX_OF_BODY(vm.seq_class);
}
static Value seq_first(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FIRST_BODY(vm.seq_class);
}
static Value seq_last(int argc, Value argv[]) {
  NATIVE_LISTLIKE_LAST_BODY(vm.seq_class);
}
static Value seq_each(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EACH_BODY(vm.seq_class);
}
static Value seq_map(int argc, Value argv[]) {
  NATIVE_LISTLIKE_MAP_BODY(vm.seq_class);
}
static Value seq_filter(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FILTER_BODY(vm.seq_class);
}
static Value seq_join(int argc, Value argv[]) {
  NATIVE_LISTLIKE_JOIN_BODY(vm.seq_class);
}
static Value seq_reverse(int argc, Value argv[]) {
  NATIVE_LISTLIKE_REVERSE_BODY(vm.seq_class);
}
static Value seq_every(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EVERY_BODY(vm.seq_class);
}
static Value seq_some(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SOME_BODY(vm.seq_class);
}
static Value seq_reduce(int argc, Value argv[]) {
  NATIVE_LISTLIKE_REDUCE_BODY(vm.seq_class);
}
static Value seq_count(int argc, Value argv[]) {
  NATIVE_LISTLIKE_COUNT_BODY(vm.seq_class);
}
static Value seq_concat(int argc, Value argv[]) {
  NATIVE_LISTLIKE_CONCAT_BODY(vm.seq_class);
}
#undef NATIVE_ENUMERABLE_GET_VALUE_ARRAY
#undef NATIVE_LISTLIKE_NEW_EMPTY
#undef NATIVE_LISTLIKE_TAKE_ARRAY

/**
 * TYPENAME_SEQ.push(arg1: TYPENAME_VALUE, ...args: TYPENAME_VALUE) -> TYPENAME_NIL
 * @brief Pushes one or many values to a TYPENAME_SEQ.
 */
static Value seq_push(int argc, Value argv[]) {
  NATIVE_CHECK_RECEIVER(vm.seq_class)

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
static Value seq_pop(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.seq_class)

  ObjSeq* seq = AS_SEQ(argv[0]);
  return pop_value_array(&seq->items);  // Does bounds checking
}

/**
 * TYPENAME_SEQ.remove_at(index: TYPENAME_INT) -> TYPENAME_VALUE
 * @brief Removes and returns the item at 'index' from a TYPENAME_SEQ. Returns TYPENAME_NIL if 'index' is out of bounds.
 * Modifies the TYPENAME_SEQ.
 */
static Value seq_remove_at(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.seq_class)
  NATIVE_CHECK_ARG_AT(1, vm.int_class)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  long long index = argv[1].as.integer;

  if (index > INT32_MAX || index < INT32_MIN) {
    runtime_error("Index %lld surpasses the maximum value of %d.", index, INT32_MAX);
    return nil_value();
  }

  return remove_at_value_array(&seq->items, (int)index);  // Does bounds checking
}
