#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "common.h"
#include "vm.h"

static bool index_getter(Obj* self, Value index, Value* result);
static bool index_setter(Obj* self, Value index, Value value);

void register_builtin_seq_class() {
  BUILTIN_REGISTER_CLASS(TYPENAME_SEQ, TYPENAME_OBJ);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_CTOR, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_TO_STR, 0);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_HAS, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, SP_METHOD_SLICE, 1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, push, -1);
  BUILTIN_REGISTER_METHOD(TYPENAME_SEQ, pop, 0);
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

  BUILTIN_REGISTER_ACCESSOR(TYPENAME_SEQ, index_getter);
  BUILTIN_REGISTER_ACCESSOR(TYPENAME_SEQ, index_setter);

  BUILTIN_FINALIZE_CLASS(TYPENAME_SEQ);
}

// Internal OP_GET_INDEX handler
static bool index_getter(Obj* self, Value index, Value* result) {
  if (!IS_NUMBER(index)) {
    runtime_error(STR(TYPENAME_SEQ) " indices must be " STR(TYPENAME_NUMBER) "s, but got %s.", typeof(index)->name->chars);
    return false;
  }

  double i_raw = AS_NUMBER(index);
  long long i;
  if (!is_int(i_raw, &i)) {
    runtime_error("Index must be an integer, but got a float.");
    return false;
  }

  ObjSeq* seq = (ObjSeq*)self;
  if (i < 0 || i >= seq->items.count) {
    runtime_error("Index out of bounds. Was %lld, but this " STR(TYPENAME_SEQ) " has length %d.", i, seq->items.count);
    return false;
  }

  *result = seq->items.values[i];
  return true;
}

// Internal OP_SET_INDEX handler
static bool index_setter(Obj* self, Value index, Value value) {
  if (!IS_NUMBER(index)) {
    runtime_error(STR(TYPENAME_SEQ) " indices must be " STR(TYPENAME_NUMBER) "s, but got %s.", typeof(index)->name->chars);
    return false;
  }

  double i_raw = AS_NUMBER(index);
  long long i;
  if (!is_int(i_raw, &i)) {
    runtime_error("Index must be an integer, but got a float.");
    return false;
  }

  ObjSeq* seq = (ObjSeq*)self;

  if (i < 0 || i >= seq->items.count) {
    runtime_error("Index out of bounds. Was %d, but this " STR(TYPENAME_SEQ) " has length %d.", i, seq->items.count);
    return false;
  }

  seq->items.values[i] = value;
  return true;
}

// Built-in seq constructor
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ SP_METHOD_CTOR,
    /* Arguments   */ DOC_ARG("len", TYPENAME_NUMBER),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Creates a new " STR(TYPENAME_NIL) "-initialized " STR(TYPENAME_SEQ) " of length 'len'.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, SP_METHOD_CTOR) {
  BUILTIN_ARGC_EXACTLY(1)
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
    /* Name        */ SP_METHOD_TO_STR,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */ "Returns a " STR(TYPENAME_STRING) " representation of a " STR(TYPENAME_SEQ) ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, SP_METHOD_TO_STR) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq     = AS_SEQ(argv[0]);
  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);

  strcpy(chars, VALUE_STR_SEQ_START);
  for (int i = 0; i < seq->items.count; i++) {
    // Execute the to_str method on the item
    push(seq->items.values[i]);  // Push the receiver (item at i) for to_str
    ObjString* item_str = AS_STRING(exec_fn(typeof(seq->items.values[i])->__to_str, 0));
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;
    }

    // Expand chars to fit the separator plus the next item
    size_t new_buf_size = strlen(chars) + item_str->length + (STR_LEN(VALUE_STR_SEQ_DELIM)) +
                          (STR_LEN(VALUE_STR_SEQ_END));  // Consider the closing bracket -  if we're done after this
                                                         // iteration we won't need to expand and can just slap it on there

    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size = new_buf_size;
      chars    = realloc(chars, buf_size);
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
  ObjString* str_obj =
      copy_string(chars,
                  (int)strlen(chars));  // TODO (optimize): Use buf_size here, but
                                        // we need to make sure that the string is
                                        // null-terminated. Also, if it's < 64 chars long, we need to shorten the length.
  free(chars);
  return OBJ_VAL(str_obj);
}

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
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  return pop_value_array(&seq->items);
}

// Built-in method to check if a sequence contains a value
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE " if the " STR(TYPENAME_SEQ) " contains an item which equals 'value'.")
BUILTIN_METHOD_DOC_OVERLOAD(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ SP_METHOD_HAS,
    /* Arguments   */ DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE " if the " STR(TYPENAME_SEQ) " contains an item for which 'pred' evaluates to " VALUE_STR_TRUE ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, SP_METHOD_HAS) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return BOOL_VAL(false);
  }

  int count = seq->items.count;  // We need to store this, because the sequence might change during the loop

  if (IS_CALLABLE(argv[1])) {
    // Function predicate
    for (int i = 0; i < count; i++) {
      // Execute the provided function on the item
      push(argv[1]);               // Push the function
      push(seq->items.values[i]);  // Push the item
      Value result = exec_fn(AS_OBJ(argv[1]), 1);
      if (vm.flags & VM_FLAG_HAS_ERROR) {
        return NIL_VAL;  // Propagate the error
      }

      // We don't use is_falsey here, because we want to check for a boolean value.
      if (IS_BOOL(result) && AS_BOOL(result)) {
        return BOOL_VAL(true);
      }
    }
  } else {
    // Value equality
    for (int i = 0; i < count; i++) {
      if (values_equal(argv[1], seq->items.values[i])) {
        return BOOL_VAL(true);
      }
    }
  }

  return BOOL_VAL(false);
}

// Builtin method to slice a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ SP_METHOD_SLICE,
    /* Arguments   */ DOC_ARG("start", TYPENAME_NUMBER) DOC_ARG_SEP DOC_ARG("end", TYPENAME_NUMBER | TYPENAME_NIL),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Returns a new " STR(TYPENAME_SEQ) " containing the items from 'start' to 'end' ('end' is exclusive)."
    " 'end' can be negative to count from the end of the " STR(TYPENAME_SEQ) ". If 'start' is greater than or equal to 'end', an empty "
    STR(TYPENAME_SEQ) " is returned. If 'end' is " STR(TYPENAME_NIL) ", all items from 'start' to the end of the " STR(
        TYPENAME_SEQ) " are included.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, SP_METHOD_SLICE) {
  BUILTIN_ARGC_EXACTLY(2)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT(1, NUMBER)
  if (IS_NIL(argv[2])) {
    argv[2] = NUMBER_VAL(AS_SEQ(argv[0])->items.count);
  }
  BUILTIN_CHECK_ARG_AT(2, NUMBER)

  ObjSeq* seq = AS_SEQ(argv[0]);
  int count   = seq->items.count;

  if (count == 0) {
    return OBJ_VAL(new_seq());
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
        "Slice indices out of bounds. Start resolved to %d and end to %d, but this " STR(TYPENAME_SEQ) " has length %d.", start,
        end, count);
    return NIL_VAL;
  }

  if (start >= end) {
    return OBJ_VAL(new_seq());
  }

  ObjSeq* sliced_seq = prealloc_seq(end - start);
  for (int i = start; i < end; i++) {
    sliced_seq->items.values[i - start] = seq->items.values[i];
  }

  return OBJ_VAL(sliced_seq);
}

// Built-in method to retrieve the first item of a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ first,
    /* Arguments   */ DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_OBJ,
    /* Description */
    "Returns the first item of a " STR(TYPENAME_SEQ) " for which 'pred' evaluates to " VALUE_STR_TRUE ". Returns " STR(
        TYPENAME_NIL) " if the " STR(TYPENAME_SEQ) " is empty or no item satisfies the predicate.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, first) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  int count = seq->items.count;  // We need to store this, because the sequence might change during the loop

  // Function predicate
  for (int i = 0; i < count; i++) {
    // Execute the provided function on the item
    push(argv[1]);               // Push the function
    push(seq->items.values[i]);  // Push the item
    Value result = exec_fn(AS_OBJ(argv[1]), 1);
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;  // Propagate the error
    }

    // We don't use is_falsey here, because we want to check for a boolean value.
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
    "Returns the last item of a " STR(TYPENAME_SEQ) " for which 'pred' evaluates to " VALUE_STR_TRUE ". Returns " STR(
        TYPENAME_NIL) " if the " STR(TYPENAME_SEQ) " is empty or no item satisfies the predicate.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, last) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NIL_VAL;
  }

  int count = seq->items.count;  // We need to store this, because the sequence might change during the loop

  // Function predicate
  for (int i = count - 1; i >= 0; i--) {
    // Execute the provided function on the item
    push(argv[1]);               // Push the function
    push(seq->items.values[i]);  // Push the item
    Value result = exec_fn(AS_OBJ(argv[1]), 1);
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      return NIL_VAL;  // Propagate the error
    }

    // We don't use is_falsey here, because we want to check for a boolean value.
    if (IS_BOOL(result) && AS_BOOL(result)) {
      return seq->items.values[i];
    }
  }

  return NIL_VAL;
}

// Built-in method to loop over a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ each,
    /* Arguments   */ DOC_ARG("fn", TYPENAME_FUNCTION),
    /* Return Type */ TYPENAME_NIL,
    /* Description */
    "Executes 'fn' for each item in the " STR(
        TYPENAME_SEQ) ". 'fn' should take one or two arguments: the item and the index of the item."
        " The latter is optional.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, each) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq  = AS_SEQ(argv[0]);
  int fn_arity = get_arity(AS_OBJ(argv[1]));
  int count    = seq->items.count;  // We need to store this, because the sequence might change during the loop

  // Loops are duplicated to avoid the overhead of checking the arity on each iteration
  switch (fn_arity) {
    case 1: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0: Push the item
        exec_fn(AS_OBJ(argv[1]), 1);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }
      }
      break;
    }
    case 2: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        push(NUMBER_VAL(i));         // arg1 (2): Push the index
        exec_fn(AS_OBJ(argv[1]), 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }
      }
      break;
    }
    default: {
      runtime_error("Function passed to \"" STR(each) "\" must take 1 or 2 arguments, but got %d.", fn_arity);
      return NIL_VAL;
    }
  }

  return NIL_VAL;
}

// Built-in method to map a sequence to a new sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ map,
    /* Arguments   */ DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_OBJ),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Maps each item in the " STR(TYPENAME_SEQ) " to a new value by executing 'fn' on it. Returns a new "
    STR(TYPENAME_SEQ) " with the mapped values. 'fn' should take one or two arguments: the item and the index of the "
    "item. The latter is optional.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, map) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq        = AS_SEQ(argv[0]);
  int fn_arity       = get_arity(AS_OBJ(argv[1]));
  int count          = seq->items.count;  // We need to store this, because the sequence might change during the loop
  ObjSeq* mapped_seq = prealloc_seq(count);

  push(OBJ_VAL(mapped_seq));  // GC Protection

  // Loops are duplicated to avoid the overhead of checking the arity on each iteration
  switch (fn_arity) {
    case 1: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        Value mapped = exec_fn(AS_OBJ(argv[1]), 1);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // Store the mapped value
        mapped_seq->items.values[i] = mapped;
      }
      break;
    }
    case 2: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        push(NUMBER_VAL(i));         // arg1 (2): Push the index
        Value mapped = exec_fn(AS_OBJ(argv[1]), 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // Store the mapped value
        mapped_seq->items.values[i] = mapped;
      }
      break;
    }
    default: {
      runtime_error("Function passed to \"" STR(map) "\" must take 1 or 2 arguments, but got %d.", fn_arity);
      return NIL_VAL;
    }
  }

  return pop();  // The seq
}

// Built-in method to filter a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ filter,
    /* Arguments   */ DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Filters the items of a " STR(TYPENAME_SEQ) " by executing 'fn' on each item. Returns a new " STR(
        TYPENAME_SEQ) " with the items for which 'fn' evaluates to " VALUE_STR_TRUE
                      ". 'fn' should take one or two arguments: the item and the index of the item. The "
                      "latter is "
                      "optional.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, filter) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq          = AS_SEQ(argv[0]);
  int fn_arity         = get_arity(AS_OBJ(argv[1]));
  int count            = seq->items.count;  // We need to store this, because the sequence might change during the loop
  ObjSeq* filtered_seq = new_seq();

  push(OBJ_VAL(filtered_seq));  // GC Protection

  // Loops are duplicated to avoid the overhead of checking the arity on each iteration
  switch (fn_arity) {
    case 1: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        Value result = exec_fn(AS_OBJ(argv[1]), 1);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // We don't use is_falsey here, because we want to check for a boolean value.
        if (IS_BOOL(result) && AS_BOOL(result)) {
          write_value_array(&filtered_seq->items, seq->items.values[i]);
        }
      }
      break;
    }
    case 2: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        push(NUMBER_VAL(i));         // arg1 (2): Push the index
        Value result = exec_fn(AS_OBJ(argv[1]), 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // We don't use is_falsey here, because we want to check for a boolean value.
        if (IS_BOOL(result) && AS_BOOL(result)) {
          write_value_array(&filtered_seq->items, seq->items.values[i]);
        }
      }
      break;
    }
    default: {
      runtime_error("Function passed to \"" STR(filter) "\" must take 1 or 2 arguments, but got %d.", fn_arity);
      return NIL_VAL;
    }
  }

  return pop();  // The seq
}

// Built-in method to join a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ join,
    /* Arguments   */ DOC_ARG("sep", TYPENAME_STRING),
    /* Return Type */ TYPENAME_STRING,
    /* Description */
    "Joins the items of a " STR(TYPENAME_SEQ) " into a single " STR(TYPENAME_STRING) ", separated by 'sep'.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, join) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT(1, STRING)

  ObjSeq* seq    = AS_SEQ(argv[0]);
  ObjString* sep = AS_STRING(argv[1]);

  size_t buf_size = 64;  // Start with a reasonable size
  char* chars     = malloc(buf_size);
  chars[0]        = '\0';  // Start with an empty string, so we can use strcat

  for (int i = 0; i < seq->items.count; i++) {
    // Maybe this is faster (checking if the item is a string)  - unsure though
    ObjString* item_str = NULL;
    if (!IS_STRING(seq->items.values[i])) {
      // Execute the to_str method on the item
      push(seq->items.values[i]);  // Push the receiver (item at i) for to_str, or
      item_str = AS_STRING(exec_fn(typeof(seq->items.values[i])->__to_str, 0));
      if (vm.flags & VM_FLAG_HAS_ERROR) {
        return NIL_VAL;
      }
    } else {
      item_str = AS_STRING(seq->items.values[i]);
    }

    // Expand chars to fit the separator plus the next item
    size_t new_buf_size = strlen(chars) + item_str->length + sep->length;  // Consider the separator

    // Expand if necessary
    if (new_buf_size > buf_size) {
      buf_size = new_buf_size;
      chars    = realloc(chars, buf_size);
    }

    // Append the string
    strcat(chars, item_str->chars);
    if (i < seq->items.count - 1) {
      strcat(chars, sep->chars);
    }
  }

  // Intuitively, you'd expect to use take_string here, but we don't know where malloc
  // allocates the memory - we don't want this block in our own memory pool.
  ObjString* str_obj = copy_string(chars, (int)strlen(chars));
  free(chars);
  return OBJ_VAL(str_obj);
}

// Built-in method to reverse a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ reverse,
    /* Arguments   */ "",
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Reverses the items of a " STR(TYPENAME_SEQ) ". Returns a new " STR(TYPENAME_SEQ) " with the items in reverse order.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, reverse) {
  BUILTIN_ARGC_EXACTLY(0)
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq          = AS_SEQ(argv[0]);
  ObjSeq* reversed_seq = prealloc_seq(seq->items.count);

  push(OBJ_VAL(reversed_seq));  // GC Protection

  for (int i = seq->items.count - 1; i >= 0; i--) {
    reversed_seq->items.values[seq->items.count - 1 - i] = seq->items.values[i];
  }

  return pop();  // The seq
}

// Built-in method to check if all items in a sequence satisfy a predicate
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ every,
    /* Arguments   */ DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE
    " if 'fn' evaluates to " VALUE_STR_TRUE
    " for every item in the " STR(TYPENAME_SEQ) ". 'fn' should take one or two arguments: the item and the index of the "
    "item. The latter is optional. Returns " VALUE_STR_FALSE " if the " STR(TYPENAME_SEQ) " is empty.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, every) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq  = AS_SEQ(argv[0]);
  int fn_arity = get_arity(AS_OBJ(argv[1]));
  int count    = seq->items.count;  // We need to store this, because the sequence might change during the loop

  // Loops are duplicated to avoid the overhead of checking the arity on each iteration
  switch (fn_arity) {
    case 1: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        Value result = exec_fn(AS_OBJ(argv[1]), 1);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // We don't use is_falsey here, because we want to check for a boolean value.
        if (!IS_BOOL(result) || !AS_BOOL(result)) {
          return BOOL_VAL(false);
        }
      }
      break;
    }
    case 2: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        push(NUMBER_VAL(i));         // arg1 (2): Push the index
        Value result = exec_fn(AS_OBJ(argv[1]), 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // We don't use is_falsey here, because we want to check for a boolean value.
        if (!IS_BOOL(result) || !AS_BOOL(result)) {
          return BOOL_VAL(false);
        }
      }
      break;
    }
    default: {
      runtime_error("Function passed to \"" STR(every) "\" must take 1 or 2 arguments, but got %d.", fn_arity);
      return NIL_VAL;
    }
  }
  if (fn_arity > 1) {
  } else {
  }

  return BOOL_VAL(true);
}

// Built-in method to check if any item in a sequence satisfies a predicate
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ some,
    /* Arguments   */ DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE
    " if 'fn' evaluates to " VALUE_STR_TRUE
    " for at least one item in the " STR(TYPENAME_SEQ) ". 'fn' should take one or two arguments: the item and the index of the "
    "item. The latter is optional. Returns " VALUE_STR_FALSE " if the " STR(TYPENAME_SEQ) " is empty.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, some) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)

  ObjSeq* seq  = AS_SEQ(argv[0]);
  int fn_arity = get_arity(AS_OBJ(argv[1]));
  int count    = seq->items.count;  // We need to store this, because the sequence might change during the loop

  // Loops are duplicated to avoid the overhead of checking the arity on each iteration
  switch (fn_arity) {
    case 1: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        Value result = exec_fn(AS_OBJ(argv[1]), 1);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // We don't use is_falsey here, because we want to check for a boolean value.
        if (IS_BOOL(result) && AS_BOOL(result)) {
          return BOOL_VAL(true);
        }
      }
      break;
    }
    case 2: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[1]);               // Push the function
        push(seq->items.values[i]);  // arg0 (1): Push the item
        push(NUMBER_VAL(i));         // arg1 (2): Push the index
        Value result = exec_fn(AS_OBJ(argv[1]), 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }

        // We don't use is_falsey here, because we want to check for a boolean value.
        if (IS_BOOL(result) && AS_BOOL(result)) {
          return BOOL_VAL(true);
        }
      }
      break;
    }
    default: {
      runtime_error("Function passed to \"" STR(some) "\" must take 1 or 2 arguments, but got %d.", fn_arity);
      return NIL_VAL;
    }
  }

  return BOOL_VAL(false);
}

// Built-in method to reduce a sequence to a single value
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ reduce,
    /* Arguments   */ DOC_ARG("initial", TYPENAME_OBJ) DOC_ARG_SEP DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_OBJ),
    /* Return Type */ TYPENAME_OBJ,
    /* Description */
    "Reduces the items of a " STR(TYPENAME_SEQ) " to a single value by executing 'fn' on each item. 'fn' should take two or three "
    "arguments: the accumulator, the item and the index. The latter is optional. The initial value of the accumulator is 'initial'.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, reduce) {
  BUILTIN_ARGC_EXACTLY(2)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT_IS_CALLABLE(2)

  ObjSeq* seq       = AS_SEQ(argv[0]);
  Value accumulator = argv[1];
  int fn_arity      = get_arity(AS_OBJ(argv[2]));
  int count         = seq->items.count;  // We need to store this, because the sequence might change during the loop

  // Loops are duplicated to avoid the overhead of checking the arity on each iteration
  switch (fn_arity) {
    case 2: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[2]);               // Push the function
        push(accumulator);           // arg0 (1): Push the accumulator
        push(seq->items.values[i]);  // arg1 (2): Push the item
        accumulator = exec_fn(AS_OBJ(argv[2]), 2);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }
      }
      break;
    }
    case 3: {
      for (int i = 0; i < count; i++) {
        // Execute the provided function on the item
        push(argv[2]);               // Push the function
        push(accumulator);           // arg0 (1): Push the accumulator
        push(seq->items.values[i]);  // arg1 (2): Push the item
        push(NUMBER_VAL(i));         // arg2 (3): Push the index
        accumulator = exec_fn(AS_OBJ(argv[2]), 3);
        if (vm.flags & VM_FLAG_HAS_ERROR) {
          return NIL_VAL;  // Propagate the error
        }
      }
      break;
    }
    default: {
      runtime_error("Function passed to \"" STR(reduce) "\" must take 2 or 3 arguments, but got %d.", fn_arity);
      return NIL_VAL;
    }
  }

  return accumulator;
}

// Built-in method to count the occurrences of a value in a sequence
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ count,
    /* Arguments   */ DOC_ARG("value", TYPENAME_OBJ),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Returns the number of occurrences of 'value' in the " STR(TYPENAME_SEQ) ".")
BUILTIN_METHOD_DOC_OVERLOAD(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ count,
    /* Arguments   */ DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL),
    /* Return Type */ TYPENAME_NUMBER,
    /* Description */
    "Returns the number of items in the " STR(TYPENAME_SEQ) " for which 'pred' evaluates to " VALUE_STR_TRUE ".");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, count) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)

  ObjSeq* seq = AS_SEQ(argv[0]);
  if (seq->items.count == 0) {
    return NUMBER_VAL(0);
  }

  int count       = seq->items.count;  // We need to store this, because the sequence might change during the loop
  int occurrences = 0;

  if (IS_CALLABLE(argv[1])) {
    int fn_arity = get_arity(AS_OBJ(argv[1]));
    if (fn_arity != 1) {
      runtime_error("Function passed to \"" STR(count) "\" must take 1 argument, but got %d.", fn_arity);
      return NIL_VAL;
    }

    // Function predicate
    for (int i = 0; i < count; i++) {
      // Execute the provided function on the item
      push(argv[1]);               // Push the function
      push(seq->items.values[i]);  // Push the item
      Value result = exec_fn(AS_OBJ(argv[1]), 1);
      if (vm.flags & VM_FLAG_HAS_ERROR) {
        return NIL_VAL;  // Propagate the error
      }

      // We don't use is_falsey here, because we want to check for a boolean value.
      if (IS_BOOL(result) && AS_BOOL(result)) {
        occurrences++;
      }
    }
  } else {
    // Value equality
    for (int i = 0; i < count; i++) {
      if (values_equal(argv[1], seq->items.values[i])) {
        occurrences++;
      }
    }
  }

  return NUMBER_VAL(occurrences);
}

// Built-in method to concatenate two sequences
BUILTIN_METHOD_DOC(
    /* Receiver    */ TYPENAME_SEQ,
    /* Name        */ concat,
    /* Arguments   */ DOC_ARG("seq", TYPENAME_SEQ),
    /* Return Type */ TYPENAME_SEQ,
    /* Description */
    "Concatenates two " STR(TYPENAME_SEQ) "s. Returns a new " STR(TYPENAME_SEQ) " with the items of the receiver followed by the "
    "items of 'seq'.");
BUILTIN_METHOD_IMPL(TYPENAME_SEQ, concat) {
  BUILTIN_ARGC_EXACTLY(1)
  BUILTIN_CHECK_RECEIVER(SEQ)
  BUILTIN_CHECK_ARG_AT(1, SEQ)

  ObjSeq* seq1         = AS_SEQ(argv[0]);
  ObjSeq* seq2         = AS_SEQ(argv[1]);
  ObjSeq* concated_seq = prealloc_seq(seq1->items.count + seq2->items.count);

  for (int i = 0; i < seq1->items.count; i++) {
    concated_seq->items.values[i] = seq1->items.values[i];
  }

  for (int i = 0; i < seq2->items.count; i++) {
    concated_seq->items.values[seq1->items.count + i] = seq2->items.values[i];
  }

  return OBJ_VAL(concated_seq);
}