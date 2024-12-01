#include <stdint.h>
#include <string.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static bool seq_get_prop(Value receiver, ObjString* name, Value* result);
static bool seq_get_subs(Value receiver, Value index, Value* result);
static bool seq_set_subs(Value receiver, Value index, Value value);

static Value seq_ctor(int argc, Value argv[]);
static Value seq_to_str(int argc, Value argv[]);
static Value seq_has(int argc, Value argv[]);
static Value seq_slice(int argc, Value argv[]);
static Value seq_push(int argc, Value argv[]);
static Value seq_pop(int argc, Value argv[]);
static Value seq_yank(int argc, Value argv[]);
static Value seq_pos(int argc, Value argv[]);
static Value seq_first(int argc, Value argv[]);
static Value seq_last(int argc, Value argv[]);
static Value seq_each(int argc, Value argv[]);
static Value seq_map(int argc, Value argv[]);
static Value seq_sift(int argc, Value argv[]);
static Value seq_join(int argc, Value argv[]);
static Value seq_flip(int argc, Value argv[]);
static Value seq_every(int argc, Value argv[]);
static Value seq_some(int argc, Value argv[]);
static Value seq_fold(int argc, Value argv[]);
static Value seq_count(int argc, Value argv[]);
static Value seq_concat(int argc, Value argv[]);

ObjClass* native_seq_class_partial_init() {
  ObjClass* seq_class = new_class(NULL, NULL);  // Names are null because hashtables are not yet initialized

  seq_class->__get_prop = seq_get_prop;
  seq_class->__set_prop = native_set_prop_not_supported;  // Not supported
  seq_class->__get_subs = seq_get_subs;
  seq_class->__set_subs = seq_set_subs;
  seq_class->__equals   = native_default_obj_equals;
  seq_class->__hash     = native_default_obj_hash;

  return seq_class;
}

void native_seq_class_finalize() {
  define_native(&vm.seq_class->methods, STR(SP_METHOD_CTOR), seq_ctor, 1);
  define_native(&vm.seq_class->methods, STR(SP_METHOD_TO_STR), seq_to_str, 0);
  define_native(&vm.seq_class->methods, STR(SP_METHOD_HAS), seq_has, 1);
  define_native(&vm.seq_class->methods, STR(SP_METHOD_SLICE), seq_slice, 2);
  define_native(&vm.seq_class->methods, "push", seq_push, -1);
  define_native(&vm.seq_class->methods, "pop", seq_pop, 0);
  define_native(&vm.seq_class->methods, "yank", seq_yank, 1);
  define_native(&vm.seq_class->methods, "pos", seq_pos, 1);
  define_native(&vm.seq_class->methods, "first", seq_first, 1);
  define_native(&vm.seq_class->methods, "last", seq_last, 1);
  define_native(&vm.seq_class->methods, "each", seq_each, 1);
  define_native(&vm.seq_class->methods, "map", seq_map, 1);
  define_native(&vm.seq_class->methods, "sift", seq_sift, 1);
  define_native(&vm.seq_class->methods, "join", seq_join, 1);
  define_native(&vm.seq_class->methods, "flip", seq_flip, 0);
  define_native(&vm.seq_class->methods, "every", seq_every, 1);
  define_native(&vm.seq_class->methods, "some", seq_some, 1);
  define_native(&vm.seq_class->methods, "fold", seq_fold, 2);
  define_native(&vm.seq_class->methods, "count", seq_count, 1);
  define_native(&vm.seq_class->methods, "concat", seq_concat, 1);
  finalize_new_class(vm.seq_class);
}

static bool seq_get_prop(Value receiver, ObjString* name, Value* result) {
#define NATIVE_LISTLIKE_GET_ARRAY(value) AS_SEQ(value)->items
  NATIVE_LISTLIKE_GET_PROP_BODY()
#undef NATIVE_LISTLIKE_GET_ARRAY
}

static bool seq_get_subs(Value receiver, Value index, Value* result) {
#define NATIVE_LISTLIKE_GET_ARRAY(value) AS_SEQ(value)->items
  NATIVE_LISTLIKE_GET_SUBS_BODY()
#undef NATIVE_LISTLIKE_GET_ARRAY
}

static bool seq_set_subs(Value receiver, Value index, Value value) {
  if (!is_int(index)) {
    vm_error("Type %s does not support set-subscripting with %s. Expected " STR(TYPENAME_INT) ".", receiver.type->name->chars,
             index.type->name->chars);
    return false;
  }

  long long idx = index.as.integer;
  ObjSeq* seq   = AS_SEQ(receiver);

  if (idx < 0 || idx >= seq->items.count) {
    vm_error("Index out of bounds. Was %d, but this " STR(TYPENAME_SEQ) " has length %d.", idx, seq->items.count);
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
    value_array_init(&items);
    ObjSeq* seq = take_seq(&items);  // TODO (optimize): Use value_array_init_of_size
    vm_push(seq_value(seq));         // GC Protection

    int count = (int)argv[1].as.integer;
    for (int i = 0; i < count; i++) {
      value_array_write(&seq->items, nil_value());
    }

    vm_pop();  // The seq
    return seq_value(seq);
  }

  if (is_tuple(argv[1])) {
    ObjTuple* tuple  = AS_TUPLE(argv[1]);
    ValueArray items = value_array_init_of_size(tuple->items.count);

    // We can use memcpy here because the items array is already preallocated
    memcpy(items.values, tuple->items.values, tuple->items.count * sizeof(Value));
    items.count = tuple->items.count;

    ObjSeq* seq = take_seq(&items);
    return seq_value(seq);
  }

  // TODO: Make a macro for this error message
  vm_error("Expected argument 0 of type " STR(TYPENAME_INT) " or " STR(TYPENAME_TUPLE) " but got %s.", argv[1].type->name->chars);
  return nil_value();
}

#define NATIVE_LISTLIKE_GET_ARRAY(value) AS_SEQ(value)->items
#define NATIVE_LISTLIKE_NEW_EMPTY() seq_value(new_seq())
#define NATIVE_LISTLIKE_TAKE_ARRAY(value_array) seq_value(take_seq(&value_array))
static Value seq_has(int argc, Value argv[]) {
  NATIVE_LISTLIKE_HAS_BODY(vm.seq_class);
}
static Value seq_slice(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SLICE_BODY(vm.seq_class);
}
static Value seq_to_str(int argc, Value argv[]) {
  NATIVE_LISTLIKE_TO_STR_BODY(vm.seq_class, VALUE_STR_SEQ_START, VALUE_STR_SEQ_DELIM, VALUE_STR_SEQ_END);
}
static Value seq_pos(int argc, Value argv[]) {
  NATIVE_LISTLIKE_POS_BODY(vm.seq_class);
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
static Value seq_sift(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SIFT_BODY(vm.seq_class);
}
static Value seq_join(int argc, Value argv[]) {
  NATIVE_LISTLIKE_JOIN_BODY(vm.seq_class);
}
static Value seq_flip(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FLIP_BODY(vm.seq_class);
}
static Value seq_every(int argc, Value argv[]) {
  NATIVE_LISTLIKE_EVERY_BODY(vm.seq_class);
}
static Value seq_some(int argc, Value argv[]) {
  NATIVE_LISTLIKE_SOME_BODY(vm.seq_class);
}
static Value seq_fold(int argc, Value argv[]) {
  NATIVE_LISTLIKE_FOLD_BODY(vm.seq_class);
}
static Value seq_count(int argc, Value argv[]) {
  NATIVE_LISTLIKE_COUNT_BODY(vm.seq_class);
}
static Value seq_concat(int argc, Value argv[]) {
  NATIVE_LISTLIKE_CONCAT_BODY(vm.seq_class);
}
#undef NATIVE_LISTLIKE_GET_ARRAY
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
    value_array_write(&seq->items, argv[i]);
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
  return value_array_pop(&seq->items);  // Does bounds checking
}

/**
 * TYPENAME_SEQ.yank(index: TYPENAME_INT) -> TYPENAME_VALUE
 * @brief Removes and returns the item at 'index' from a TYPENAME_SEQ. Returns TYPENAME_NIL if 'index' is out of bounds.
 * Modifies the TYPENAME_SEQ.
 */
static Value seq_yank(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_RECEIVER(vm.seq_class)
  NATIVE_CHECK_ARG_AT(1, vm.int_class)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  long long index = argv[1].as.integer;

  if (index > INT32_MAX || index < INT32_MIN) {
    vm_error("Index %lld surpasses the maximum value of %d.", index, INT32_MAX);
    return nil_value();
  }

  return value_array_remove_at(&seq->items, (int)index);  // Does bounds checking
}
