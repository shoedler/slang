#ifndef builtin_h
#define builtin_h

#include "builtin_util.h"
#include "value.h"
#include "vm.h"

// Register built-in functions
extern void register_builtin_functions();

// Registers the built-in object class and its methods.
extern void register_builtin_obj_class();

// Registers the built-in nil class and its methods.
extern void register_builtin_nil_class();

// Registers the built-in bool class and its methods.
extern void register_builtin_bool_class();

// Registers the built-in number class and its methods.
extern void register_builtin_num_class();

// Registers the built-in int class and its methods.
extern void register_builtin_int_class();

// Registers the built-in float class and its methods.
extern void register_builtin_float_class();

// Registers the built-in string class and its methods.
extern void register_builtin_str_class();

// Registers the built-in seq class and its methods.
extern void register_builtin_seq_class();

// Registers the built-in tuple class and its methods.
extern void register_builtin_tuple_class();

// Registers the built-in fn class and its methods.
extern void register_builtin_fn_class();

// Registers the built-in class class and its methods.
extern void register_builtin_class_class();

// Registers the built-in file module
extern void register_builtin_file_module();

// Registers the built-in perf module
extern void register_builtin_perf_module();

// Registers the built-in debug module
extern void register_builtin_debug_module();

//
// Built-in function-, method- and class-declarations
//

// Built-in functions
BUILTIN_DECLARE_FN(clock);
BUILTIN_DECLARE_FN(log);
BUILTIN_DECLARE_FN(typeof);
BUILTIN_DECLARE_FN(cwd);

// Obj built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, SP_METHOD_TO_STR)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, SP_METHOD_HAS)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, hash)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, entries)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, values)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, keys)

// Fn built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_FUNCTION, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_FUNCTION, SP_METHOD_TO_STR)
BUILTIN_DECLARE_METHOD(TYPENAME_FUNCTION, SP_METHOD_HAS)
BUILTIN_DECLARE_METHOD(TYPENAME_FUNCTION, bind);

// Class built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_CLASS, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_CLASS, SP_METHOD_TO_STR)
BUILTIN_DECLARE_METHOD(TYPENAME_CLASS, SP_METHOD_HAS)

// Bool built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_BOOL, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_BOOL, SP_METHOD_TO_STR)

// Nil built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_NIL, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_NIL, SP_METHOD_TO_STR)

// Number built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_NUM, SP_METHOD_CTOR)
// Int built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_INT, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_INT, SP_METHOD_TO_STR)
// Float built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_FLOAT, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_FLOAT, SP_METHOD_TO_STR)

// String built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, SP_METHOD_TO_STR)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, SP_METHOD_HAS)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, SP_METHOD_SLICE)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, split)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, trim)

// Seq built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, SP_METHOD_TO_STR)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, SP_METHOD_HAS)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, SP_METHOD_SLICE)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, push)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, pop)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, remove_at)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, index_of)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, first)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, last)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, each)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, map)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, filter)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, join)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, reverse)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, every)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, some)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, reduce)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, count)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, concat)

// Tuple built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, SP_METHOD_TO_STR)
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, SP_METHOD_HAS)
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, SP_METHOD_SLICE);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, index_of);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, first);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, last);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, each);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, map);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, filter);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, join);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, reverse);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, every);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, some);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, reduce);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, count);
BUILTIN_DECLARE_METHOD(TYPENAME_TUPLE, concat);
#endif

// Implementation for the 'has' method for objects that have a value array. BUILTIN_ENUMERABLE_GET_VALUE_ARRAY must be defined
// before this.
#define BUILTIN_ENUMERABLE_HAS(type, doc_return_kind)                                                                           \
  BUILTIN_METHOD_DOC(TYPENAME_##type, SP_METHOD_HAS, DOC_ARG("value", TYPENAME_OBJ), TYPENAME_BOOL,                             \
                     "Returns " VALUE_STR_TRUE " if the " STR(TYPENAME_##type) " contains " doc_return_kind                     \
                                                                               " which equals 'value'.")                        \
  BUILTIN_METHOD_DOC_OVERLOAD(TYPENAME_##type, SP_METHOD_HAS, DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), TYPENAME_BOOL, \
                              "Returns " VALUE_STR_TRUE                                                                         \
                              " if the " STR(TYPENAME_##type) " contains " doc_return_kind                                      \
                                                              " for which 'pred' evaluates to " VALUE_STR_TRUE ".");            \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, SP_METHOD_HAS) {                                                                         \
    BUILTIN_ARGC_EXACTLY(1)                                                                                                     \
    BUILTIN_CHECK_RECEIVER(##type)                                                                                              \
                                                                                                                                \
    ValueArray items;                                                                                                           \
    BUILTIN_ENUMERABLE_GET_VALUE_ARRAY(argv[0]);                                                                                \
    if (items.count == 0) {                                                                                                     \
      return BOOL_VAL(false);                                                                                                   \
    }                                                                                                                           \
                                                                                                                                \
    int count = items.count; /* We need to store this. Might change during the loop */                                          \
                                                                                                                                \
    if (IS_CALLABLE(argv[1])) {                                                                                                 \
      /* Function predicate */                                                                                                  \
      for (int i = 0; i < count; i++) {                                                                                         \
        /* Execute the provided function on the item */                                                                         \
        push(argv[1]);         /* Push the function */                                                                          \
        push(items.values[i]); /* Push the item */                                                                              \
        Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                                       \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                                     \
          return NIL_VAL; /* Propagate the error */                                                                             \
        }                                                                                                                       \
        /* We don't use is_falsey here, because we want a boolean value. */                                                     \
        if (IS_BOOL(result) && AS_BOOL(result)) {                                                                               \
          return BOOL_VAL(true);                                                                                                \
        }                                                                                                                       \
      }                                                                                                                         \
    } else {                                                                                                                    \
      /* Value equality */                                                                                                      \
      for (int i = 0; i < count; i++) {                                                                                         \
        if (values_equal(argv[1], items.values[i])) {                                                                           \
          return BOOL_VAL(true);                                                                                                \
        }                                                                                                                       \
      }                                                                                                                         \
    }                                                                                                                           \
                                                                                                                                \
    return BOOL_VAL(false);                                                                                                     \
  }

// Implementation for the 'to_str' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_TO_STR(type, start_chars, delim_chars, end_chars)                                              \
  BUILTIN_METHOD_DOC(TYPENAME_##type, SP_METHOD_TO_STR, "", TYPENAME_##type,                                            \
                     "Returns a " STR(TYPENAME_STRING) " representation of a " STR(TYPENAME_##type) ".");               \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, SP_METHOD_TO_STR) {                                                              \
    BUILTIN_ARGC_EXACTLY(0)                                                                                             \
    BUILTIN_CHECK_RECEIVER(type)                                                                                        \
                                                                                                                        \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                \
    size_t buf_size  = 64; /* Start with a reasonable size */                                                           \
    char* chars      = malloc(buf_size);                                                                                \
                                                                                                                        \
    strcpy(chars, start_chars);                                                                                         \
    for (int i = 0; i < items.count; i++) {                                                                             \
      /* Execute the to_str method on the item */                                                                       \
      push(items.values[i]); /* Push the receiver (item at i) for to_str */                                             \
      ObjString* item_str = AS_STRING(exec_callable(typeof(items.values[i])->__to_str, 0));                             \
      if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                               \
        return NIL_VAL;                                                                                                 \
      }                                                                                                                 \
                                                                                                                        \
      /* Expand chars to fit the separator plus the next item */                                                        \
      size_t new_buf_size = strlen(chars) + item_str->length + (STR_LEN(delim_chars)) +                                 \
                            (STR_LEN(end_chars)); /* Consider the closing bracket -  if we're done after this */        \
                                                  /* iteration we won't need to expand and can just slap it on there */ \
                                                                                                                        \
      /* Expand if necessary */                                                                                         \
      if (new_buf_size > buf_size) {                                                                                    \
        buf_size        = new_buf_size;                                                                                 \
        size_t old_size = strlen(chars);                                                                                \
        chars           = realloc(chars, buf_size);                                                                     \
        chars[old_size] = '\0'; /* Ensure null-termination at the end of the old string */                              \
      }                                                                                                                 \
                                                                                                                        \
      /* Append the string */                                                                                           \
      strcat(chars, item_str->chars);                                                                                   \
      if (i < items.count - 1) {                                                                                        \
        strcat(chars, delim_chars);                                                                                     \
      }                                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    strcat(chars, end_chars);                                                                                           \
                                                                                                                        \
    /* Intuitively, you'd expect to use take_string here, but we don't know where malloc */                             \
    /* allocates the memory - we don't want this block in our own memory pool. */                                       \
    ObjString* str_obj = copy_string(chars, (int)strlen(chars)); /* TODO (optimize): Use buf_size here, but */          \
                                                                 /* we need to make sure that the string is */          \
    /* null-terminated. Also, if it's < 64 chars long, we need to shorten the length. */                                \
    free(chars);                                                                                                        \
    return OBJ_VAL(str_obj);                                                                                            \
  }

// Implementation for the 'slice' method for listlike objects (Objects that have a 'ValueArray items' field). Requires these
// macros to be defined before this:
// - BUILTIN_LISTLIKE_TAKE_ARRAY(value): to create a new listlike object from a ValueArray.
// - BUILTIN_LISTLIKE_NEW_EMPTY(): to create a new empty listlike object.
#define BUILTIN_LISTLIKE_SLICE(type)                                                                                                                               \
  BUILTIN_METHOD_DOC( TYPENAME_##type, SP_METHOD_SLICE, DOC_ARG("start", TYPENAME_INT) DOC_ARG_SEP DOC_ARG("end", TYPENAME_INT | TYPENAME_NIL), TYPENAME_##type,\
    "Returns a new " STR(TYPENAME_##type) " containing the items from 'start' to 'end' ('end' is exclusive)."\
    " 'end' can be negative to count from the end of the " STR(TYPENAME_##type) ". If 'start' is greater than or equal to 'end', an empty "\
    STR(TYPENAME_##type) " is returned. If 'end' is " STR(TYPENAME_NIL) ", all items from 'start' to the end of the " STR(\
        TYPENAME_##type) " are included."); \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, SP_METHOD_SLICE) {                                                                                                          \
    BUILTIN_ARGC_EXACTLY(2)                                                                                                                                        \
    BUILTIN_CHECK_RECEIVER(type)                                                                                                                                   \
    BUILTIN_CHECK_ARG_AT(1, INT)                                                                                                                                   \
    if (IS_NIL(argv[2])) {                                                                                                                                         \
      argv[2] = INT_VAL(AS_##type(argv[0])->items.count);                                                                                                          \
    }                                                                                                                                                              \
    BUILTIN_CHECK_ARG_AT(2, INT)                                                                                                                                   \
                                                                                                                                                                   \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                                                           \
    int count        = items.count;                                                                                                                                \
                                                                                                                                                                   \
    if (count == 0) {                                                                                                                                              \
      return OBJ_VAL(BUILTIN_LISTLIKE_NEW_EMPTY());                                                                                                                \
    }                                                                                                                                                              \
                                                                                                                                                                   \
    int start = (int)AS_INT(argv[1]);                                                                                                                              \
    int end   = (int)AS_INT(argv[2]);                                                                                                                              \
                                                                                                                                                                   \
    /* Handle negative indices */                                                                                                                                  \
    if (start < 0) {                                                                                                                                               \
      start = count + start;                                                                                                                                       \
    }                                                                                                                                                              \
    if (end < 0) {                                                                                                                                                 \
      end = count + end;                                                                                                                                           \
    }                                                                                                                                                              \
                                                                                                                                                                   \
    /* Clamp out-of-bounds indices */                                                                                                                              \
    if (start < 0) {                                                                                                                                               \
      start = 0;                                                                                                                                                   \
    }                                                                                                                                                              \
    if (end > count) {                                                                                                                                             \
      end = count;                                                                                                                                                 \
    }                                                                                                                                                              \
                                                                                                                                                                   \
    /* Handle invalid or 0 length ranges */                                                                                                                        \
    if (start >= end) {                                                                                                                                            \
      return OBJ_VAL(BUILTIN_LISTLIKE_NEW_EMPTY());                                                                                                                \
    }                                                                                                                                                              \
    ValueArray sliced = prealloc_value_array(end - start);                                                                                                         \
    for (int i = start; i < end; i++) {                                                                                                                            \
      sliced.values[i - start] = items.values[i];                                                                                                                  \
    }                                                                                                                                                              \
                                                                                                                                                                   \
    /* No need for GC protection - taking an array will not trigger a GC. */                                                                                       \
    return OBJ_VAL(BUILTIN_LISTLIKE_TAKE_ARRAY(sliced));                                                                                                           \
  }

// Implementation for the 'index_of' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_INDEX_OF(type)                                                                                        \
  BUILTIN_METHOD_DOC(TYPENAME_##type, index_of, DOC_ARG("value", TYPENAME_OBJ), TYPENAME_INT,                                  \
                     "Returns the index of first item which equals 'value' in a " STR(                                         \
                         TYPENAME_##type) ". Returns " VALUE_STR_NIL " if 'value' is not found.")                              \
  BUILTIN_METHOD_DOC_OVERLOAD(TYPENAME_##type, index_of, DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), TYPENAME_INT,      \
                              "Returns the index of the first item in a " STR(                                                 \
                                  TYPENAME_##type) " for which 'pred' evaluates to " VALUE_STR_TRUE ". Returns " VALUE_STR_NIL \
                                                   " if no item satisfies the predicate.");                                    \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, index_of) {                                                                             \
    BUILTIN_ARGC_EXACTLY(1)                                                                                                    \
    BUILTIN_CHECK_RECEIVER(type)                                                                                               \
                                                                                                                               \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                       \
    if (items.count == 0) {                                                                                                    \
      return NIL_VAL;                                                                                                          \
    }                                                                                                                          \
                                                                                                                               \
    int count = items.count; /* We need to store this, because the listlike might change during the loop */                    \
                                                                                                                               \
    if (IS_CALLABLE(argv[1])) {                                                                                                \
      /* Function predicate */                                                                                                 \
      for (int i = 0; i < count; i++) {                                                                                        \
        /* Execute the provided function on the item */                                                                        \
        push(argv[1]);         /* Push the function */                                                                         \
        push(items.values[i]); /* Push the item */                                                                             \
        Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                                      \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                                    \
          return NIL_VAL; /* Propagate the error */                                                                            \
        }                                                                                                                      \
                                                                                                                               \
        /* We don't use is_falsey here, because we want to check for a boolean value. */                                       \
        if (IS_BOOL(result) && AS_BOOL(result)) {                                                                              \
          return INT_VAL(i);                                                                                                   \
        }                                                                                                                      \
      }                                                                                                                        \
    } else {                                                                                                                   \
      /* Value equality */                                                                                                     \
      for (int i = 0; i < count; i++) {                                                                                        \
        if (values_equal(argv[1], items.values[i])) {                                                                          \
          return INT_VAL(i);                                                                                                   \
        }                                                                                                                      \
      }                                                                                                                        \
    }                                                                                                                          \
                                                                                                                               \
    return NIL_VAL;                                                                                                            \
  }

// Implementation for the 'first' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_FIRST(type)                                                                                         \
  BUILTIN_METHOD_DOC(                                                                                                        \
      TYPENAME_##type, first, DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), TYPENAME_OBJ,                               \
      "Returns the first item of a " STR(TYPENAME_##type) " for which 'pred' evaluates to " VALUE_STR_TRUE ". Returns " STR( \
          TYPENAME_NIL) " if the " STR(TYPENAME_##type) " is empty or no item satisfies the predicate.");                    \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, first) {                                                                              \
    BUILTIN_ARGC_EXACTLY(1)                                                                                                  \
    BUILTIN_CHECK_RECEIVER(type)                                                                                             \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                                      \
                                                                                                                             \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                     \
    if (items.count == 0) {                                                                                                  \
      return NIL_VAL;                                                                                                        \
    }                                                                                                                        \
                                                                                                                             \
    int count = items.count; /* We need to store this, because the listlike might change during the loop */                  \
                                                                                                                             \
    /* Function predicate */                                                                                                 \
    for (int i = 0; i < count; i++) {                                                                                        \
      /* Execute the provided function on the item */                                                                        \
      push(argv[1]);         /* Push the function */                                                                         \
      push(items.values[i]); /* Push the item */                                                                             \
      Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                                      \
      if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                                    \
        return NIL_VAL; /* Propagate the error */                                                                            \
      }                                                                                                                      \
                                                                                                                             \
      /* We don't use is_falsey here, because we want to check for a boolean value. */                                       \
      if (IS_BOOL(result) && AS_BOOL(result)) {                                                                              \
        return items.values[i];                                                                                              \
      }                                                                                                                      \
    }                                                                                                                        \
                                                                                                                             \
    return NIL_VAL;                                                                                                          \
  }

// Implementation for the 'last' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_LAST(type)                                                                                         \
  BUILTIN_METHOD_DOC(                                                                                                       \
      TYPENAME_##type, last, DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), TYPENAME_OBJ,                               \
                                                                                                                            \
      "Returns the last item of a " STR(TYPENAME_##type) " for which 'pred' evaluates to " VALUE_STR_TRUE ". Returns " STR( \
          TYPENAME_NIL) " if the " STR(TYPENAME_##type) " is empty or no item satisfies the predicate.");                   \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, last) {                                                                              \
    BUILTIN_ARGC_EXACTLY(1)                                                                                                 \
    BUILTIN_CHECK_RECEIVER(type)                                                                                            \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                                     \
                                                                                                                            \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                    \
    if (items.count == 0) {                                                                                                 \
      return NIL_VAL;                                                                                                       \
    }                                                                                                                       \
                                                                                                                            \
    int count = items.count; /* We need to store this, because the listlike might change during the loop */                 \
                                                                                                                            \
    /* Function predicate */                                                                                                \
    for (int i = count - 1; i >= 0; i--) {                                                                                  \
      /* Execute the provided function on the item */                                                                       \
      push(argv[1]);         /* Push the function */                                                                        \
      push(items.values[i]); /* Push the item */                                                                            \
      Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                                     \
      if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                                   \
        return NIL_VAL; /* Propagate the error */                                                                           \
      }                                                                                                                     \
                                                                                                                            \
      /* We don't use is_falsey here, because we want to check for a boolean value. */                                      \
      if (IS_BOOL(result) && AS_BOOL(result)) {                                                                             \
        return items.values[i];                                                                                             \
      }                                                                                                                     \
    }                                                                                                                       \
                                                                                                                            \
    return NIL_VAL;                                                                                                         \
  }

// Implementation for the 'each' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_EACH(type)                                                                                 \
  BUILTIN_METHOD_DOC(TYPENAME_##type,each,DOC_ARG("fn", TYPENAME_FUNCTION), TYPENAME_NIL,                        \
    "Executes 'fn' for each item in the " STR(TYPENAME_##type) ". 'fn' should take one or two arguments: "       \
    "the item and the index of the item. The latter is optional."); \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, each) {                                                                      \
    BUILTIN_ARGC_EXACTLY(1)                                                                                         \
    BUILTIN_CHECK_RECEIVER(type)                                                                                    \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                             \
                                                                                                                    \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                            \
    int fn_arity     = callable_get_arity(AS_OBJ(argv[1]));                                                         \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */  \
                                                                                                                    \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                        \
    switch (fn_arity) {                                                                                             \
      case 1: {                                                                                                     \
        for (int i = 0; i < count; i++) {                                                                           \
          /* Execute the provided function on the item */                                                           \
          push(argv[1]);         /* Push the function */                                                            \
          push(items.values[i]); /* arg0: Push the item */                                                          \
          exec_callable(AS_OBJ(argv[1]), 1);                                                                        \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return NIL_VAL; /* Propagate the error */                                                               \
          }                                                                                                         \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      case 2: {                                                                                                     \
        for (int i = 0; i < count; i++) {                                                                           \
          /* Execute the provided function on the item */                                                           \
          push(argv[1]);         /* Push the function */                                                            \
          push(items.values[i]); /* arg0 (1): Push the item */                                                      \
          push(INT_VAL(i));      /* arg1 (2): Push the index */                                                     \
          exec_callable(AS_OBJ(argv[1]), 2);                                                                        \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return NIL_VAL; /* Propagate the error */                                                               \
          }                                                                                                         \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      default: {                                                                                                    \
        runtime_error("Function passed to \"" STR(each) "\" must take 1 or 2 arguments, but got %d.", fn_arity);    \
        return NIL_VAL;                                                                                             \
      }                                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    return NIL_VAL;                                                                                                 \
  }

// Implementation for the 'map' method for listlike objects (Objects that have a 'ValueArray items' field). Requires these macros
// to be defined before this:
// - BUILTIN_LISTLIKE_TAKE_ARRAY(value): to create a new listlike object from a ValueArray.
#define BUILTIN_LISTLIKE_MAP(type)                                                                                  \
  BUILTIN_METHOD_DOC(\
     TYPENAME_##type,\
     map,\
     DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_OBJ),\
     TYPENAME_##type,\
    "Maps each item in the " STR(TYPENAME_##type) " to a new value by executing 'fn' on it. Returns a new "\
    STR(TYPENAME_##type) " with the mapped values. 'fn' should take one or two arguments: the item and the index of the "\
    "item. The latter is optional.");                                                                                             \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, map) {                                                                       \
    BUILTIN_ARGC_EXACTLY(1)                                                                                         \
    BUILTIN_CHECK_RECEIVER(type)                                                                                    \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                             \
                                                                                                                    \
    ValueArray items  = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity      = callable_get_arity(AS_OBJ(argv[1]));                                                        \
    int count         = items.count; /* We need to store this, because the listlike might change during the loop */ \
    ValueArray mapped = prealloc_value_array(count);                                                                \
                                                                                                                    \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                        \
    switch (fn_arity) {                                                                                             \
      case 1: {                                                                                                     \
        for (int i = 0; i < count; i++) {                                                                           \
          /* Execute the provided function on the item */                                                           \
          push(argv[1]);         /* Push the function */                                                            \
          push(items.values[i]); /* arg0 (1): Push the item */                                                      \
          mapped.values[i] = exec_callable(AS_OBJ(argv[1]), 1);                                                     \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return NIL_VAL; /* Propagate the error */                                                               \
          }                                                                                                         \
          push(mapped.values[i]); /* GC Protection */                                                               \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      case 2: {                                                                                                     \
        for (int i = 0; i < count; i++) {                                                                           \
          /* Execute the provided function on the item */                                                           \
          push(argv[1]);         /* Push the function */                                                            \
          push(items.values[i]); /* arg0 (1): Push the item */                                                      \
          push(INT_VAL(i));      /* arg1 (2): Push the index */                                                     \
          mapped.values[i] = exec_callable(AS_OBJ(argv[1]), 2);                                                     \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return NIL_VAL; /* Propagate the error */                                                               \
          }                                                                                                         \
          push(mapped.values[i]); /* GC Protection */                                                               \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      default: {                                                                                                    \
        runtime_error("Function passed to \"" STR(map) "\" must take 1 or 2 arguments, but got %d.", fn_arity);     \
        return NIL_VAL;                                                                                             \
      }                                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    /* Take at the end so that tuples calc their hash correctly */                                                  \
    push(OBJ_VAL(BUILTIN_LISTLIKE_TAKE_ARRAY(mapped)));                                                             \
    Value result = pop();                                                                                           \
                                                                                                                    \
    /* Remove the values which were pushed for GC Protection */                                                     \
    vm.stack_top -= count;                                                                                          \
    return result;                                                                                                  \
  }

// Implementation for the 'filter' method for listlike objects (Objects that have a 'ValueArray items' field). Requires these
// macros to be defined before this:
// - BUILTIN_LISTLIKE_TAKE_ARRAY(value): to create a new listlike object from a ValueArray.
#define BUILTIN_LISTLIKE_FILTER(type)                                                                                         \
  BUILTIN_METHOD_DOC(TYPENAME_##type, filter, DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_BOOL), TYPENAME_##type,               \
                                                                                                                              \
                     "Filters the items of a " STR(TYPENAME_##type) " by executing 'fn' on each item. Returns a new " STR(    \
                         TYPENAME_##type) " with the items for which 'fn' evaluates to " VALUE_STR_TRUE                       \
                                          ". 'fn' should take one or two arguments: the item and the index of the item. The " \
                                          "latter is "                                                                        \
                                          "optional.");                                                                       \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, filter) {                                                                              \
    BUILTIN_ARGC_EXACTLY(1)                                                                                                   \
    BUILTIN_CHECK_RECEIVER(type)                                                                                              \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                                       \
                                                                                                                              \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                      \
    int fn_arity     = callable_get_arity(AS_OBJ(argv[1]));                                                                   \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */            \
                                                                                                                              \
    ValueArray filtered_items;                                                                                                \
    init_value_array(&filtered_items);                                                                                        \
    int filtered_count = 0; /* Need to track this so we can clean the stack from the pushed values for GC protection */       \
                                                                                                                              \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                                  \
    switch (fn_arity) {                                                                                                       \
      case 1: {                                                                                                               \
        for (int i = 0; i < count; i++) {                                                                                     \
          /* Execute the provided function on the item */                                                                     \
          push(argv[1]);         /* Push the function */                                                                      \
          push(items.values[i]); /* arg0 (1): Push the item */                                                                \
          Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                                   \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                                 \
            return NIL_VAL; /* Propagate the error */                                                                         \
          }                                                                                                                   \
                                                                                                                              \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                                    \
          if (IS_BOOL(result) && AS_BOOL(result)) {                                                                           \
            push(result); /* GC Protection */                                                                                 \
            filtered_count++;                                                                                                 \
            write_value_array(&filtered_items, items.values[i]);                                                              \
          }                                                                                                                   \
        }                                                                                                                     \
        break;                                                                                                                \
      }                                                                                                                       \
      case 2: {                                                                                                               \
        for (int i = 0; i < count; i++) {                                                                                     \
          /* Execute the provided function on the item */                                                                     \
          push(argv[1]);         /* Push the function */                                                                      \
          push(items.values[i]); /* arg0 (1): Push the item */                                                                \
          push(INT_VAL(i));      /* arg1 (2): Push the index */                                                               \
          Value result = exec_callable(AS_OBJ(argv[1]), 2);                                                                   \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                                 \
            return NIL_VAL; /* Propagate the error */                                                                         \
          }                                                                                                                   \
                                                                                                                              \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                                    \
          if (IS_BOOL(result) && AS_BOOL(result)) {                                                                           \
            push(result); /* GC Protection */                                                                                 \
            filtered_count++;                                                                                                 \
            write_value_array(&filtered_items, items.values[i]);                                                              \
          }                                                                                                                   \
        }                                                                                                                     \
        break;                                                                                                                \
      }                                                                                                                       \
      default: {                                                                                                              \
        runtime_error("Function passed to \"" STR(filter) "\" must take 1 or 2 arguments, but got %d.", fn_arity);            \
        return NIL_VAL;                                                                                                       \
      }                                                                                                                       \
    }                                                                                                                         \
                                                                                                                              \
    /* Take at the end so that tuples calc their hash correctly */                                                            \
    push(OBJ_VAL(BUILTIN_LISTLIKE_TAKE_ARRAY(filtered_items)));                                                               \
    Value result = pop();                                                                                                     \
                                                                                                                              \
    /* Remove the values which were pushed for GC Protection */                                                               \
    vm.stack_top -= filtered_count;                                                                                           \
    return result;                                                                                                            \
  }

// Implementation for the 'join' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_JOIN(type)                                                                                 \
  BUILTIN_METHOD_DOC(                                                                                               \
      TYPENAME_##type, join, DOC_ARG("sep", TYPENAME_STRING), TYPENAME_STRING,                                      \
      "Joins the items of a " STR(TYPENAME_##type) " into a single " STR(TYPENAME_STRING) ", separated by 'sep'."); \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, join) {                                                                      \
    BUILTIN_ARGC_EXACTLY(1)                                                                                         \
    BUILTIN_CHECK_RECEIVER(type)                                                                                    \
    BUILTIN_CHECK_ARG_AT(1, STRING)                                                                                 \
                                                                                                                    \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                            \
    ObjString* sep   = AS_STRING(argv[1]);                                                                          \
                                                                                                                    \
    size_t buf_size = 64; /* Start with a reasonable size */                                                        \
    char* chars     = malloc(buf_size);                                                                             \
    chars[0]        = '\0'; /* Start with an empty string, so we can use strcat */                                  \
                                                                                                                    \
    for (int i = 0; i < items.count; i++) {                                                                         \
      /* Maybe this is faster (checking if the item is a string)  - unsure though */                                \
      ObjString* item_str = NULL;                                                                                   \
      if (!IS_STRING(items.values[i])) {                                                                            \
        /* Execute the to_str method on the item */                                                                 \
        push(items.values[i]); /* Push the receiver (item at i) for to_str, or */                                   \
        item_str = AS_STRING(exec_callable(typeof(items.values[i])->__to_str, 0));                                  \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                         \
          return NIL_VAL;                                                                                           \
        }                                                                                                           \
      } else {                                                                                                      \
        item_str = AS_STRING(items.values[i]);                                                                      \
      }                                                                                                             \
                                                                                                                    \
      /* Expand chars to fit the separator plus the next item */                                                    \
      size_t new_buf_size = strlen(chars) + item_str->length + sep->length; /* Consider the separator */            \
                                                                                                                    \
      /* Expand if necessary */                                                                                     \
      if (new_buf_size > buf_size) {                                                                                \
        buf_size        = new_buf_size;                                                                             \
        size_t old_size = strlen(chars);                                                                            \
        chars           = realloc(chars, buf_size);                                                                 \
        chars[old_size] = '\0'; /* Ensure null-termination at the end of the old string */                          \
      }                                                                                                             \
                                                                                                                    \
      /* Append the string */                                                                                       \
      strcat(chars, item_str->chars);                                                                               \
      if (i < items.count - 1) {                                                                                    \
        strcat(chars, sep->chars);                                                                                  \
      }                                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    /* Intuitively, you'd expect to use take_string here, but we don't know where malloc */                         \
    /* allocates the memory - we don't want this block in our own memory pool. */                                   \
    ObjString* str_obj = copy_string(chars, (int)strlen(chars));                                                    \
    free(chars);                                                                                                    \
    return OBJ_VAL(str_obj);                                                                                        \
  }

// Implementation for the 'reverse' method for listlike objects (Objects that have a 'ValueArray items' field). Requires these
// macros to be defined before this:
// - BUILTIN_LISTLIKE_TAKE_ARRAY(value): to create a new listlike object from a ValueArray.
#define BUILTIN_LISTLIKE_REVERSE(type)                                                       \
  BUILTIN_METHOD_DOC(TYPENAME_##type, reverse, "", TYPENAME_##type,                          \
                     "Reverses the items of a " STR(TYPENAME_##type) ". Returns a new " STR( \
                         TYPENAME_##type) " with the items in reverse order.");              \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, reverse) {                                            \
    BUILTIN_ARGC_EXACTLY(0)                                                                  \
    BUILTIN_CHECK_RECEIVER(type)                                                             \
                                                                                             \
    ValueArray items    = LISTLIKE_GET_VALUEARRAY(argv[0]);                                  \
    ValueArray reversed = prealloc_value_array(items.count);                                 \
                                                                                             \
    /* No GC protection needed */                                                            \
    for (int i = items.count - 1; i >= 0; i--) {                                             \
      reversed.values[items.count - 1 - i] = items.values[i];                                \
    }                                                                                        \
                                                                                             \
    /* No need for GC protection - taking an array will not trigger a GC. */                 \
    return OBJ_VAL(BUILTIN_LISTLIKE_TAKE_ARRAY(reversed));                                   \
  }

// Implementation for the 'every' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_EVERY(type)                                                                               \
  BUILTIN_METHOD_DOC(\
     TYPENAME_##type,\
     every,\
     DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_BOOL),\
     TYPENAME_BOOL,\
    "Returns " VALUE_STR_TRUE\
    " if 'fn' evaluates to " VALUE_STR_TRUE\
    " for every item in the " STR(TYPENAME_##type) ". 'fn' should take one or two arguments: the item and the index of the "\
    "item. The latter is optional. Returns " VALUE_STR_FALSE " if the " STR(TYPENAME_##type) " is empty.");                                                                                            \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, every) {                                                                    \
    BUILTIN_ARGC_EXACTLY(1)                                                                                        \
    BUILTIN_CHECK_RECEIVER(type)                                                                                   \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                            \
                                                                                                                   \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity     = callable_get_arity(AS_OBJ(argv[1]));                                                        \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                   \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
    switch (fn_arity) {                                                                                            \
      case 1: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                        \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return NIL_VAL; /* Propagate the error */                                                              \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (!IS_BOOL(result) || !AS_BOOL(result)) {                                                              \
            return BOOL_VAL(false);                                                                                \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      case 2: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          push(INT_VAL(i));      /* arg1 (2): Push the index */                                                    \
          Value result = exec_callable(AS_OBJ(argv[1]), 2);                                                        \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return NIL_VAL; /* Propagate the error */                                                              \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (!IS_BOOL(result) || !AS_BOOL(result)) {                                                              \
            return BOOL_VAL(false);                                                                                \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      default: {                                                                                                   \
        runtime_error("Function passed to \"" STR(every) "\" must take 1 or 2 arguments, but got %d.", fn_arity);  \
        return NIL_VAL;                                                                                            \
      }                                                                                                            \
    }                                                                                                              \
    if (fn_arity > 1) {                                                                                            \
    } else {                                                                                                       \
    }                                                                                                              \
                                                                                                                   \
    return BOOL_VAL(true);                                                                                         \
  }

// Implementation for the 'some' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_SOME(type)                                                                                \
  BUILTIN_METHOD_DOC(\
     TYPENAME_##type,\
     some,\
     DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_BOOL),\
     TYPENAME_BOOL,\
  \
    "Returns " VALUE_STR_TRUE\
    " if 'fn' evaluates to " VALUE_STR_TRUE\
    " for at least one item in the " STR(TYPENAME_##type) ". 'fn' should take one or two arguments: the item and the index of the "\
    "item. The latter is optional. Returns " VALUE_STR_FALSE " if the " STR(TYPENAME_##type) " is empty.");                                                                                            \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, some) {                                                                     \
    BUILTIN_ARGC_EXACTLY(1)                                                                                        \
    BUILTIN_CHECK_RECEIVER(type)                                                                                   \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                            \
                                                                                                                   \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity     = callable_get_arity(AS_OBJ(argv[1]));                                                        \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                   \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
    switch (fn_arity) {                                                                                            \
      case 1: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                        \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return NIL_VAL; /* Propagate the error */                                                              \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (IS_BOOL(result) && AS_BOOL(result)) {                                                                \
            return BOOL_VAL(true);                                                                                 \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      case 2: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          push(INT_VAL(i));      /* arg1 (2): Push the index */                                                    \
          Value result = exec_callable(AS_OBJ(argv[1]), 2);                                                        \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return NIL_VAL; /* Propagate the error */                                                              \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (IS_BOOL(result) && AS_BOOL(result)) {                                                                \
            return BOOL_VAL(true);                                                                                 \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      default: {                                                                                                   \
        runtime_error("Function passed to \"" STR(some) "\" must take 1 or 2 arguments, but got %d.", fn_arity);   \
        return NIL_VAL;                                                                                            \
      }                                                                                                            \
    }                                                                                                              \
                                                                                                                   \
    return BOOL_VAL(false);                                                                                        \
  }

// Implementation for the 'reduce' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_REDUCE(type)                                                                               \
  BUILTIN_METHOD_DOC(\
     TYPENAME_##type,\
     reduce,\
     DOC_ARG("initial", TYPENAME_OBJ) DOC_ARG_SEP DOC_ARG("fn", TYPENAME_FUNCTION->TYPENAME_OBJ),\
     TYPENAME_OBJ,\
    "Reduces the items of a " STR(TYPENAME_##type) " to a single value by executing 'fn' on each item."\
    " 'fn' should take two or three arguments: the accumulator, the item and the index. "\
    "The latter is optional. The initial value of the accumulator is 'initial'.");                                                                                             \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, reduce) {                                                                    \
    BUILTIN_ARGC_EXACTLY(2)                                                                                         \
    BUILTIN_CHECK_RECEIVER(type)                                                                                    \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(2)                                                                             \
                                                                                                                    \
    ValueArray items  = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    Value accumulator = argv[1];                                                                                    \
    int fn_arity      = callable_get_arity(AS_OBJ(argv[2]));                                                        \
    int count         = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                    \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                        \
    switch (fn_arity) {                                                                                             \
      case 2: {                                                                                                     \
        for (int i = 0; i < count; i++) {                                                                           \
          /* Execute the provided function on the item */                                                           \
          push(argv[2]);         /* Push the function */                                                            \
          push(accumulator);     /* arg0 (1): Push the accumulator */                                               \
          push(items.values[i]); /* arg1 (2): Push the item */                                                      \
          accumulator = exec_callable(AS_OBJ(argv[2]), 2);                                                          \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return NIL_VAL; /* Propagate the error */                                                               \
          }                                                                                                         \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      case 3: {                                                                                                     \
        for (int i = 0; i < count; i++) {                                                                           \
          /* Execute the provided function on the item */                                                           \
          push(argv[2]);         /* Push the function */                                                            \
          push(accumulator);     /* arg0 (1): Push the accumulator */                                               \
          push(items.values[i]); /* arg1 (2): Push the item */                                                      \
          push(INT_VAL(i));      /* arg2 (3): Push the index */                                                     \
          accumulator = exec_callable(AS_OBJ(argv[2]), 3);                                                          \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return NIL_VAL; /* Propagate the error */                                                               \
          }                                                                                                         \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      default: {                                                                                                    \
        runtime_error("Function passed to \"" STR(reduce) "\" must take 2 or 3 arguments, but got %d.", fn_arity);  \
        return NIL_VAL;                                                                                             \
      }                                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    return accumulator;                                                                                             \
  }

// Implementation for the 'count' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_COUNT(type)                                                                                    \
  BUILTIN_METHOD_DOC(TYPENAME_##type, count, DOC_ARG("value", TYPENAME_OBJ), TYPENAME_INT,                              \
                     "Returns the number of occurrences of 'value' in the " STR(TYPENAME_##type) ".")                   \
  BUILTIN_METHOD_DOC_OVERLOAD(                                                                                          \
      TYPENAME_##type, count, DOC_ARG("pred", TYPENAME_FUNCTION->TYPENAME_BOOL), TYPENAME_INT,                          \
      "Returns the number of items in the " STR(TYPENAME_##type) " for which 'pred' evaluates to " VALUE_STR_TRUE "."); \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, count) {                                                                         \
    BUILTIN_ARGC_EXACTLY(1)                                                                                             \
    BUILTIN_CHECK_RECEIVER(type)                                                                                        \
                                                                                                                        \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                \
    if (items.count == 0) {                                                                                             \
      return INT_VAL(0);                                                                                                \
    }                                                                                                                   \
                                                                                                                        \
    int count       = items.count; /* We need to store this, because the listlike might change during the loop */       \
    int occurrences = 0;                                                                                                \
                                                                                                                        \
    if (IS_CALLABLE(argv[1])) {                                                                                         \
      int fn_arity = callable_get_arity(AS_OBJ(argv[1]));                                                               \
      if (fn_arity != 1) {                                                                                              \
        runtime_error("Function passed to \"" STR(count) "\" must take 1 argument, but got %d.", fn_arity);             \
        return NIL_VAL;                                                                                                 \
      }                                                                                                                 \
                                                                                                                        \
      /* Function predicate */                                                                                          \
      for (int i = 0; i < count; i++) {                                                                                 \
        /* Execute the provided function on the item */                                                                 \
        push(argv[1]);         /* Push the function */                                                                  \
        push(items.values[i]); /* Push the item */                                                                      \
        Value result = exec_callable(AS_OBJ(argv[1]), 1);                                                               \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                             \
          return NIL_VAL; /* Propagate the error */                                                                     \
        }                                                                                                               \
                                                                                                                        \
        /* We don't use is_falsey here, because we want to check for a boolean value. */                                \
        if (IS_BOOL(result) && AS_BOOL(result)) {                                                                       \
          occurrences++;                                                                                                \
        }                                                                                                               \
      }                                                                                                                 \
    } else {                                                                                                            \
      /* Value equality */                                                                                              \
      for (int i = 0; i < count; i++) {                                                                                 \
        if (values_equal(argv[1], items.values[i])) {                                                                   \
          occurrences++;                                                                                                \
        }                                                                                                               \
      }                                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    return INT_VAL(occurrences);                                                                                        \
  }

// Implementation for the 'concat' method for listlike objects (Objects that have a 'ValueArray items' field).
#define BUILTIN_LISTLIKE_CONCAT(type)                                            \
  BUILTIN_METHOD_DOC(\
     TYPENAME_##type,\
     concat,\
     DOC_ARG("other", TYPENAME_##type),\
     TYPENAME_##type,\
    "Concatenates two " STR(TYPENAME_##type) "s. Returns a new " STR(TYPENAME_##type) " with the items of the receiver followed by the "\
    "items of 'other'.");                                                          \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, concat) {                                 \
    BUILTIN_ARGC_EXACTLY(1)                                                      \
    BUILTIN_CHECK_RECEIVER(type)                                                 \
    BUILTIN_CHECK_ARG_AT(1, type)                                                \
                                                                                 \
    ValueArray items1 = LISTLIKE_GET_VALUEARRAY(argv[0]);                        \
    ValueArray items2 = LISTLIKE_GET_VALUEARRAY(argv[1]);                        \
                                                                                 \
    ValueArray concatenated = prealloc_value_array(items1.count + items2.count); \
                                                                                 \
    for (int i = 0; i < items1.count; i++) {                                     \
      concatenated.values[i] = items1.values[i];                                 \
    }                                                                            \
                                                                                 \
    for (int i = 0; i < items2.count; i++) {                                     \
      concatenated.values[items1.count + i] = items2.values[i];                  \
    }                                                                            \
                                                                                 \
    /* No need for GC protection - taking an array will not trigger a GC. */     \
    return OBJ_VAL(BUILTIN_LISTLIKE_TAKE_ARRAY(concatenated));                   \
  }