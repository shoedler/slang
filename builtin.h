#ifndef builtin_h
#define builtin_h

#include "builtin_util.h"
#include "value.h"
#include "vm.h"

// Register built-in functions
extern void register_builtin_functions();

// Registers the built-in object class and its methods.
extern void finalize_builtin_obj_class();

// Registers the built-in nil class and its methods.
extern void finalize_builtin_nil_class();

// Registers the built-in bool class and its methods.
extern void finalize_builtin_bool_class();

// Registers the built-in number class and its methods.
extern void finalize_builtin_num_class();

// Registers the built-in int class and its methods.
extern void finalize_builtin_int_class();

// Registers the built-in float class and its methods.
extern void finalize_builtin_float_class();

// Registers the built-in string class and its methods.
extern void finalize_builtin_str_class();

// Registers the built-in seq class and its methods.
extern void finalize_builtin_seq_class();

// Registers the built-in tuple class and its methods.
extern void finalize_builtin_tuple_class();

// Registers the built-in fn class and its methods.
extern void finalize_builtin_fn_class();

// Registers the built-in class class and its methods.
extern void finalize_builtin_class_class();

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

/**
 * TYPENAME_T.SP_METHOD_HAS(value: TYPENAME_VALUE) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the TYPENAME_T contains a vlaue which equals 'value'.
 *
 * TYPENAME_T.SP_METHOD_HAS(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the TYPENAME_T contains a value for which 'pred' evaluates to VALUE_STR_TRUE.
 */
// before this.
#define BUILTIN_ENUMERABLE_HAS(type, doc_return_kind)                                  \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, SP_METHOD_HAS) {                                \
    UNUSED(argc);                                                                      \
    BUILTIN_CHECK_RECEIVER(type)                                                       \
                                                                                       \
    ValueArray items;                                                                  \
    BUILTIN_ENUMERABLE_GET_VALUE_ARRAY(argv[0]);                                       \
    if (items.count == 0) {                                                            \
      return bool_value(false);                                                        \
    }                                                                                  \
                                                                                       \
    int count = items.count; /* We need to store this. Might change during the loop */ \
                                                                                       \
    if (IS_CALLABLE(argv[1])) {                                                        \
      /* Function predicate */                                                         \
      for (int i = 0; i < count; i++) {                                                \
        /* Execute the provided function on the item */                                \
        push(argv[1]);         /* Push the function */                                 \
        push(items.values[i]); /* Push the item */                                     \
        Value result = exec_callable(argv[1], 1);                                      \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                            \
          return nil_value(); /* Propagate the error */                                \
        }                                                                              \
        /* We don't use is_falsey here, because we want a boolean value. */            \
        if (IS_BOOL(result) && result.as.boolean) {                                    \
          return bool_value(true);                                                     \
        }                                                                              \
      }                                                                                \
    } else {                                                                           \
      /* Value equality */                                                             \
      for (int i = 0; i < count; i++) {                                                \
        if (values_equal(argv[1], items.values[i])) {                                  \
          return bool_value(true);                                                     \
        }                                                                              \
      }                                                                                \
    }                                                                                  \
                                                                                       \
    return bool_value(false);                                                          \
  }

/**
 * TYPENAME_T.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of the TYPENAME_T.
 */
#define BUILTIN_LISTLIKE_TO_STR(uc_type, start_chars, delim_chars, end_chars)                                           \
  BUILTIN_METHOD_IMPL(TYPENAME_##uc_type, SP_METHOD_TO_STR) {                                                           \
    UNUSED(argc);                                                                                                       \
    BUILTIN_CHECK_RECEIVER(uc_type)                                                                                     \
                                                                                                                        \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                \
    size_t buf_size  = 64; /* Start with a reasonable size */                                                           \
    char* chars      = malloc(buf_size);                                                                                \
                                                                                                                        \
    strcpy(chars, start_chars);                                                                                         \
    for (int i = 0; i < items.count; i++) {                                                                             \
      /* Execute the to_str method on the item */                                                                       \
      push(items.values[i]); /* Push the receiver (item at i) for to_str */                                             \
      ObjString* item_str = AS_STRING(exec_callable(fn_value(items.values[i].type->__to_str), 0));                      \
      if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                               \
        return nil_value();                                                                                             \
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
    return str_value(str_obj);                                                                                          \
  }

/**
 * TYPENAME_T.SP_METHOD_SLICE(start: TYPENAME_INT, end: TYPENAME_INT | TYPENAME_NIL) -> TYPENAME_T
 * @brief Returns a new TYPENAME_T containing the items from 'start' to 'end' ('end' is exclusive).
 * 'end' can be negative to count from the end of the TYPENAME_T. If 'start' is greater than or equal to 'end', an empty
 * TYPENAME_T is returned. If 'end' is TYPENAME_NIL, all items from 'start' to the end of the TYPENAME_T are included.
 */
#define BUILTIN_LISTLIKE_SLICE(type)                                         \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, SP_METHOD_SLICE) {                    \
    UNUSED(argc);                                                            \
    BUILTIN_CHECK_RECEIVER(type)                                             \
    BUILTIN_CHECK_ARG_AT(1, INT)                                             \
    if (IS_NIL(argv[2])) {                                                   \
      argv[2] = int_value(AS_##type(argv[0])->items.count);                  \
    }                                                                        \
    BUILTIN_CHECK_ARG_AT(2, INT)                                             \
                                                                             \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                     \
    int count        = items.count;                                          \
                                                                             \
    if (count == 0) {                                                        \
      return BUILTIN_LISTLIKE_NEW_EMPTY();                                   \
    }                                                                        \
                                                                             \
    int start = (int)argv[1].as.integer;                                     \
    int end   = (int)argv[2].as.integer;                                     \
                                                                             \
    /* Handle negative indices */                                            \
    if (start < 0) {                                                         \
      start = count + start;                                                 \
    }                                                                        \
    if (end < 0) {                                                           \
      end = count + end;                                                     \
    }                                                                        \
                                                                             \
    /* Clamp out-of-bounds indices */                                        \
    if (start < 0) {                                                         \
      start = 0;                                                             \
    }                                                                        \
    if (end > count) {                                                       \
      end = count;                                                           \
    }                                                                        \
                                                                             \
    /* Handle invalid or 0 length ranges */                                  \
    if (start >= end) {                                                      \
      return BUILTIN_LISTLIKE_NEW_EMPTY();                                   \
    }                                                                        \
    ValueArray sliced = prealloc_value_array(end - start);                   \
    for (int i = start; i < end; i++) {                                      \
      sliced.values[i - start] = items.values[i];                            \
    }                                                                        \
                                                                             \
    /* No need for GC protection - taking an array will not trigger a GC. */ \
    return BUILTIN_LISTLIKE_TAKE_ARRAY(sliced);                              \
  }

/**
 * TYPENAME_T.index_of(value: TYPENAME_VALUE) -> TYPENAME_INT
 * @brief Returns the index of the first item which equals 'value' in a TYPENAME_T. Returns VALUE_STR_NIL if 'value' is not found.
 *
 * TYPENAME_T.index_of(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_INT
 * @brief Returns the index of the first item in a TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE. Returns VALUE_STR_NIL
 * if no item satisfies the predicate.
 */
#define BUILTIN_LISTLIKE_INDEX_OF(type)                                                                     \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, index_of) {                                                          \
    UNUSED(argc);                                                                                           \
    BUILTIN_CHECK_RECEIVER(type)                                                                            \
                                                                                                            \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                    \
    if (items.count == 0) {                                                                                 \
      return nil_value();                                                                                   \
    }                                                                                                       \
                                                                                                            \
    int count = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                            \
    if (IS_CALLABLE(argv[1])) {                                                                             \
      /* Function predicate */                                                                              \
      for (int i = 0; i < count; i++) {                                                                     \
        /* Execute the provided function on the item */                                                     \
        push(argv[1]);         /* Push the function */                                                      \
        push(items.values[i]); /* Push the item */                                                          \
        Value result = exec_callable(argv[1], 1);                                                           \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                 \
          return nil_value(); /* Propagate the error */                                                     \
        }                                                                                                   \
                                                                                                            \
        /* We don't use is_falsey here, because we want to check for a boolean value. */                    \
        if (IS_BOOL(result) && result.as.boolean) {                                                         \
          return int_value(i);                                                                              \
        }                                                                                                   \
      }                                                                                                     \
    } else {                                                                                                \
      /* Value equality */                                                                                  \
      for (int i = 0; i < count; i++) {                                                                     \
        if (values_equal(argv[1], items.values[i])) {                                                       \
          return int_value(i);                                                                              \
        }                                                                                                   \
      }                                                                                                     \
    }                                                                                                       \
                                                                                                            \
    return nil_value();                                                                                     \
  }

/**
 * TYPENAME_T.first(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_VALUE
 * @brief Returns the first item of a TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE. Returns VALUE_STR_NIL if the
 * TYPENAME_T is empty or no item satisfies the predicate.
 */
#define BUILTIN_LISTLIKE_FIRST(type)                                                                        \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, first) {                                                             \
    UNUSED(argc);                                                                                           \
    BUILTIN_CHECK_RECEIVER(type)                                                                            \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                     \
                                                                                                            \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                    \
    if (items.count == 0) {                                                                                 \
      return nil_value();                                                                                   \
    }                                                                                                       \
                                                                                                            \
    int count = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                            \
    /* Function predicate */                                                                                \
    for (int i = 0; i < count; i++) {                                                                       \
      /* Execute the provided function on the item */                                                       \
      push(argv[1]);         /* Push the function */                                                        \
      push(items.values[i]); /* Push the item */                                                            \
      Value result = exec_callable(argv[1], 1);                                                             \
      if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                   \
        return nil_value(); /* Propagate the error */                                                       \
      }                                                                                                     \
                                                                                                            \
      /* We don't use is_falsey here, because we want to check for a boolean value. */                      \
      if (IS_BOOL(result) && result.as.boolean) {                                                           \
        return items.values[i];                                                                             \
      }                                                                                                     \
    }                                                                                                       \
                                                                                                            \
    return nil_value();                                                                                     \
  }

/**
 * TYPENAME_T.last(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_VALUE
 * @brief Returns the last item of a TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE. Returns VALUE_STR_NIL if the
 * TYPENAME_T is empty or no item satisfies the predicate.
 */
#define BUILTIN_LISTLIKE_LAST(type)                                                                         \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, last) {                                                              \
    UNUSED(argc);                                                                                           \
    BUILTIN_CHECK_RECEIVER(type)                                                                            \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                     \
                                                                                                            \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                    \
    if (items.count == 0) {                                                                                 \
      return nil_value();                                                                                   \
    }                                                                                                       \
                                                                                                            \
    int count = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                            \
    /* Function predicate */                                                                                \
    for (int i = count - 1; i >= 0; i--) {                                                                  \
      /* Execute the provided function on the item */                                                       \
      push(argv[1]);         /* Push the function */                                                        \
      push(items.values[i]); /* Push the item */                                                            \
      Value result = exec_callable(argv[1], 1);                                                             \
      if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                   \
        return nil_value(); /* Propagate the error */                                                       \
      }                                                                                                     \
                                                                                                            \
      /* We don't use is_falsey here, because we want to check for a boolean value. */                      \
      if (IS_BOOL(result) && result.as.boolean) {                                                           \
        return items.values[i];                                                                             \
      }                                                                                                     \
    }                                                                                                       \
                                                                                                            \
    return nil_value();                                                                                     \
  }

/**
 * TYPENAME_T.each(fn: TYPENAME_FUNCTION) -> TYPENAME_NIL
 * @brief Executes 'fn' for each item in the TYPENAME_T. 'fn' should take one or two arguments: the item and the index of the
 * item. The latter is optional.
 */
#define BUILTIN_LISTLIKE_EACH(type)                                                                                \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, each) {                                                                     \
    UNUSED(argc);                                                                                                  \
    BUILTIN_CHECK_RECEIVER(type)                                                                                   \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                            \
                                                                                                                   \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity     = callable_get_arity(argv[1]);                                                                \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                   \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
    switch (fn_arity) {                                                                                            \
      case 1: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0: Push the item */                                                         \
          exec_callable(argv[1], 1);                                                                               \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return nil_value(); /* Propagate the error */                                                          \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      case 2: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          push(int_value(i));    /* arg1 (2): Push the index */                                                    \
          exec_callable(argv[1], 2);                                                                               \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return nil_value(); /* Propagate the error */                                                          \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      default: {                                                                                                   \
        runtime_error("Function passed to \"" STR(each) "\" must take 1 or 2 arguments, but got %d.", fn_arity);   \
        return nil_value();                                                                                        \
      }                                                                                                            \
    }                                                                                                              \
                                                                                                                   \
    return nil_value();                                                                                            \
  }

/**
 * TYPENAME_T.map(fn: TYPENAME_FUNCTION) -> TYPENAME_T
 * @brief Maps each item in the TYPENAME_T to a new value by executing 'fn' on it. Returns a new TYPENAME_T containing the mapped
 * values. 'fn' should take one or two arguments: the item and the index of the item. The latter is optional.
 */
#define BUILTIN_LISTLIKE_MAP(type)                                                                                  \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, map) {                                                                       \
    UNUSED(argc);                                                                                                   \
    BUILTIN_CHECK_RECEIVER(type)                                                                                    \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                             \
                                                                                                                    \
    ValueArray items  = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity      = callable_get_arity(argv[1]);                                                                \
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
          mapped.values[i] = exec_callable(argv[1], 1);                                                             \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return nil_value(); /* Propagate the error */                                                           \
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
          push(int_value(i));    /* arg1 (2): Push the index */                                                     \
          mapped.values[i] = exec_callable(argv[1], 2);                                                             \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return nil_value(); /* Propagate the error */                                                           \
          }                                                                                                         \
          push(mapped.values[i]); /* GC Protection */                                                               \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      default: {                                                                                                    \
        runtime_error("Function passed to \"" STR(map) "\" must take 1 or 2 arguments, but got %d.", fn_arity);     \
        return nil_value();                                                                                         \
      }                                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    /* Take at the end so that tuples calc their hash correctly */                                                  \
    push(BUILTIN_LISTLIKE_TAKE_ARRAY(mapped));                                                                      \
    Value result = pop();                                                                                           \
                                                                                                                    \
    /* Remove the values which were pushed for GC Protection */                                                     \
    vm.stack_top -= count;                                                                                          \
    return result;                                                                                                  \
  }

/**
 * TYPENAME_T.filter(fn: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_T
 * @brief Filters the items of a TYPENAME_T by executing 'fn' on each item. Returns a new TYPENAME_T with the items for which 'fn'
 * evaluates to VALUE_STR_TRUE. 'fn' should take one or two arguments: the item and the index of the item. The latter is optional.
 */
#define BUILTIN_LISTLIKE_FILTER(type)                                                                                   \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, filter) {                                                                        \
    UNUSED(argc);                                                                                                       \
    BUILTIN_CHECK_RECEIVER(type)                                                                                        \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                                 \
                                                                                                                        \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                                \
    int fn_arity     = callable_get_arity(argv[1]);                                                                     \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */      \
                                                                                                                        \
    ValueArray filtered_items;                                                                                          \
    init_value_array(&filtered_items);                                                                                  \
    int filtered_count = 0; /* Need to track this so we can clean the stack from the pushed values for GC protection */ \
                                                                                                                        \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                            \
    switch (fn_arity) {                                                                                                 \
      case 1: {                                                                                                         \
        for (int i = 0; i < count; i++) {                                                                               \
          /* Execute the provided function on the item */                                                               \
          push(argv[1]);         /* Push the function */                                                                \
          push(items.values[i]); /* arg0 (1): Push the item */                                                          \
          Value result = exec_callable(argv[1], 1);                                                                     \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                           \
            return nil_value(); /* Propagate the error */                                                               \
          }                                                                                                             \
                                                                                                                        \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                              \
          if (IS_BOOL(result) && result.as.boolean) {                                                                   \
            push(result); /* GC Protection */                                                                           \
            filtered_count++;                                                                                           \
            write_value_array(&filtered_items, items.values[i]);                                                        \
          }                                                                                                             \
        }                                                                                                               \
        break;                                                                                                          \
      }                                                                                                                 \
      case 2: {                                                                                                         \
        for (int i = 0; i < count; i++) {                                                                               \
          /* Execute the provided function on the item */                                                               \
          push(argv[1]);         /* Push the function */                                                                \
          push(items.values[i]); /* arg0 (1): Push the item */                                                          \
          push(int_value(i));    /* arg1 (2): Push the index */                                                         \
          Value result = exec_callable(argv[1], 2);                                                                     \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                           \
            return nil_value(); /* Propagate the error */                                                               \
          }                                                                                                             \
                                                                                                                        \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                              \
          if (IS_BOOL(result) && result.as.boolean) {                                                                   \
            push(result); /* GC Protection */                                                                           \
            filtered_count++;                                                                                           \
            write_value_array(&filtered_items, items.values[i]);                                                        \
          }                                                                                                             \
        }                                                                                                               \
        break;                                                                                                          \
      }                                                                                                                 \
      default: {                                                                                                        \
        runtime_error("Function passed to \"" STR(filter) "\" must take 1 or 2 arguments, but got %d.", fn_arity);      \
        return nil_value();                                                                                             \
      }                                                                                                                 \
    }                                                                                                                   \
                                                                                                                        \
    /* Take at the end so that tuples calc their hash correctly */                                                      \
    push(BUILTIN_LISTLIKE_TAKE_ARRAY(filtered_items));                                                                  \
    Value result = pop();                                                                                               \
                                                                                                                        \
    /* Remove the values which were pushed for GC Protection */                                                         \
    vm.stack_top -= filtered_count;                                                                                     \
    return result;                                                                                                      \
  }

/**
 * TYPENAME_T.join(sep: TYPENAME_STRING) -> TYPENAME_STRING
 * @brief Joins the items of a TYPENAME_T into a single TYPENAME_STRING, separated by 'sep'.
 */
#define BUILTIN_LISTLIKE_JOIN(uc_type)                                                                   \
  BUILTIN_METHOD_IMPL(TYPENAME_##uc_type, join) {                                                        \
    UNUSED(argc);                                                                                        \
    BUILTIN_CHECK_RECEIVER(uc_type)                                                                      \
    BUILTIN_CHECK_ARG_AT(1, STRING)                                                                      \
                                                                                                         \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                 \
    ObjString* sep   = AS_STRING(argv[1]);                                                               \
                                                                                                         \
    size_t buf_size = 64; /* Start with a reasonable size */                                             \
    char* chars     = malloc(buf_size);                                                                  \
    chars[0]        = '\0'; /* Start with an empty string, so we can use strcat */                       \
                                                                                                         \
    for (int i = 0; i < items.count; i++) {                                                              \
      /* Maybe this is faster (checking if the item is a string)  - unsure though */                     \
      ObjString* item_str = NULL;                                                                        \
      if (!IS_STRING(items.values[i])) {                                                                 \
        /* Execute the to_str method on the item */                                                      \
        push(items.values[i]); /* Push the receiver (item at i) for to_str, or */                        \
        item_str = AS_STRING(exec_callable(fn_value(items.values[i].type->__to_str), 0));                \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                              \
          return nil_value();                                                                            \
        }                                                                                                \
      } else {                                                                                           \
        item_str = AS_STRING(items.values[i]);                                                           \
      }                                                                                                  \
                                                                                                         \
      /* Expand chars to fit the separator plus the next item */                                         \
      size_t new_buf_size = strlen(chars) + item_str->length + sep->length; /* Consider the separator */ \
                                                                                                         \
      /* Expand if necessary */                                                                          \
      if (new_buf_size > buf_size) {                                                                     \
        buf_size        = new_buf_size;                                                                  \
        size_t old_size = strlen(chars);                                                                 \
        chars           = realloc(chars, buf_size);                                                      \
        chars[old_size] = '\0'; /* Ensure null-termination at the end of the old string */               \
      }                                                                                                  \
                                                                                                         \
      /* Append the string */                                                                            \
      strcat(chars, item_str->chars);                                                                    \
      if (i < items.count - 1) {                                                                         \
        strcat(chars, sep->chars);                                                                       \
      }                                                                                                  \
    }                                                                                                    \
                                                                                                         \
    /* Intuitively, you'd expect to use take_string here, but we don't know where malloc */              \
    /* allocates the memory - we don't want this block in our own memory pool. */                        \
    ObjString* str_obj = copy_string(chars, (int)strlen(chars));                                         \
    free(chars);                                                                                         \
    return str_value(str_obj);                                                                           \
  }

/**
 * TYPENAME_T.reverse() -> TYPENAME_T
 * @brief Reverses the items of a TYPENAME_T. Returns a new TYPENAME_T with the items in reverse order.
 */
#define BUILTIN_LISTLIKE_REVERSE(type)                                       \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, reverse) {                            \
    UNUSED(argc);                                                            \
    BUILTIN_CHECK_RECEIVER(type)                                             \
                                                                             \
    ValueArray items    = LISTLIKE_GET_VALUEARRAY(argv[0]);                  \
    ValueArray reversed = prealloc_value_array(items.count);                 \
                                                                             \
    /* No GC protection needed */                                            \
    for (int i = items.count - 1; i >= 0; i--) {                             \
      reversed.values[items.count - 1 - i] = items.values[i];                \
    }                                                                        \
                                                                             \
    /* No need for GC protection - taking an array will not trigger a GC. */ \
    return BUILTIN_LISTLIKE_TAKE_ARRAY(reversed);                            \
  }

/**
 * TYPENAME_T.every(fn: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if 'fn' evaluates to VALUE_STR_TRUE for every item in the TYPENAME_T. 'fn' should take one or two
 * arguments: the item and the index of the item. The latter is optional. Returns VALUE_STR_FALSE if the TYPENAME_T is empty.
 */
#define BUILTIN_LISTLIKE_EVERY(type)                                                                               \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, every) {                                                                    \
    UNUSED(argc);                                                                                                  \
    BUILTIN_CHECK_RECEIVER(type)                                                                                   \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                            \
                                                                                                                   \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity     = callable_get_arity(argv[1]);                                                                \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                   \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
    switch (fn_arity) {                                                                                            \
      case 1: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          Value result = exec_callable(argv[1], 1);                                                                \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return nil_value(); /* Propagate the error */                                                          \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (!IS_BOOL(result) || !result.as.boolean) {                                                            \
            return bool_value(false);                                                                              \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      case 2: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          push(int_value(i));    /* arg1 (2): Push the index */                                                    \
          Value result = exec_callable(argv[1], 2);                                                                \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return nil_value(); /* Propagate the error */                                                          \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (!IS_BOOL(result) || !result.as.boolean) {                                                            \
            return bool_value(false);                                                                              \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      default: {                                                                                                   \
        runtime_error("Function passed to \"" STR(every) "\" must take 1 or 2 arguments, but got %d.", fn_arity);  \
        return nil_value();                                                                                        \
      }                                                                                                            \
    }                                                                                                              \
    if (fn_arity > 1) {                                                                                            \
    } else {                                                                                                       \
    }                                                                                                              \
                                                                                                                   \
    return bool_value(true);                                                                                       \
  }

/**
 * TYPENAME_T.some(fn: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if 'fn' evaluates to VALUE_STR_TRUE for at least one item in the TYPENAME_T. 'fn' should take one
 * or two arguments: the item and the index of the item. The latter is optional. Returns VALUE_STR_FALSE if the TYPENAME_T is
 * empty.
 */
#define BUILTIN_LISTLIKE_SOME(type)                                                                                \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, some) {                                                                     \
    UNUSED(argc);                                                                                                  \
    BUILTIN_CHECK_RECEIVER(type)                                                                                   \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(1)                                                                            \
                                                                                                                   \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    int fn_arity     = callable_get_arity(argv[1]);                                                                \
    int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                   \
    /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
    switch (fn_arity) {                                                                                            \
      case 1: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          Value result = exec_callable(argv[1], 1);                                                                \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return nil_value(); /* Propagate the error */                                                          \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (IS_BOOL(result) && result.as.boolean) {                                                              \
            return bool_value(true);                                                                               \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      case 2: {                                                                                                    \
        for (int i = 0; i < count; i++) {                                                                          \
          /* Execute the provided function on the item */                                                          \
          push(argv[1]);         /* Push the function */                                                           \
          push(items.values[i]); /* arg0 (1): Push the item */                                                     \
          push(int_value(i));    /* arg1 (2): Push the index */                                                    \
          Value result = exec_callable(argv[1], 2);                                                                \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                      \
            return nil_value(); /* Propagate the error */                                                          \
          }                                                                                                        \
                                                                                                                   \
          /* We don't use is_falsey here, because we want to check for a boolean value. */                         \
          if (IS_BOOL(result) && result.as.boolean) {                                                              \
            return bool_value(true);                                                                               \
          }                                                                                                        \
        }                                                                                                          \
        break;                                                                                                     \
      }                                                                                                            \
      default: {                                                                                                   \
        runtime_error("Function passed to \"" STR(some) "\" must take 1 or 2 arguments, but got %d.", fn_arity);   \
        return nil_value();                                                                                        \
      }                                                                                                            \
    }                                                                                                              \
                                                                                                                   \
    return bool_value(false);                                                                                      \
  }

/**
 * TYPENAME_T.reduce(initial: TYPENAME_VALUE, fn: TYPENAME_FUNCTION) -> TYPENAME_VALUE
 * @brief Reduces the items of a TYPENAME_T to a single value by executing 'fn' on each item. 'fn' should take two or three
 * arguments: the accumulator, the item and the index. The latter is optional. The initial value of the accumulator is 'initial'.
 */
#define BUILTIN_LISTLIKE_REDUCE(type)                                                                               \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, reduce) {                                                                    \
    UNUSED(argc);                                                                                                   \
    BUILTIN_CHECK_RECEIVER(type)                                                                                    \
    BUILTIN_CHECK_ARG_AT_IS_CALLABLE(2)                                                                             \
                                                                                                                    \
    ValueArray items  = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                           \
    Value accumulator = argv[1];                                                                                    \
    int fn_arity      = callable_get_arity(argv[2]);                                                                \
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
          accumulator = exec_callable(argv[2], 2);                                                                  \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return nil_value(); /* Propagate the error */                                                           \
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
          push(int_value(i));    /* arg2 (3): Push the index */                                                     \
          accumulator = exec_callable(argv[2], 3);                                                                  \
          if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
            return nil_value(); /* Propagate the error */                                                           \
          }                                                                                                         \
        }                                                                                                           \
        break;                                                                                                      \
      }                                                                                                             \
      default: {                                                                                                    \
        runtime_error("Function passed to \"" STR(reduce) "\" must take 2 or 3 arguments, but got %d.", fn_arity);  \
        return nil_value();                                                                                         \
      }                                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    return accumulator;                                                                                             \
  }

/**
 * TYPENAME_T.count(value: TYPENAME_VALUE) -> TYPENAME_INT
 * @brief Returns the number of occurrences of 'value' in the TYPENAME_T.
 *
 * TYPENAME_T.count(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_INT
 * @brief Returns the number of items in the TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE.
 */
#define BUILTIN_LISTLIKE_COUNT(type)                                                                              \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, count) {                                                                   \
    UNUSED(argc);                                                                                                 \
    BUILTIN_CHECK_RECEIVER(type)                                                                                  \
                                                                                                                  \
    ValueArray items = LISTLIKE_GET_VALUEARRAY(argv[0]);                                                          \
    if (items.count == 0) {                                                                                       \
      return int_value(0);                                                                                        \
    }                                                                                                             \
                                                                                                                  \
    int count       = items.count; /* We need to store this, because the listlike might change during the loop */ \
    int occurrences = 0;                                                                                          \
                                                                                                                  \
    if (IS_CALLABLE(argv[1])) {                                                                                   \
      int fn_arity = callable_get_arity(argv[1]);                                                                 \
      if (fn_arity != 1) {                                                                                        \
        runtime_error("Function passed to \"" STR(count) "\" must take 1 argument, but got %d.", fn_arity);       \
        return nil_value();                                                                                       \
      }                                                                                                           \
                                                                                                                  \
      /* Function predicate */                                                                                    \
      for (int i = 0; i < count; i++) {                                                                           \
        /* Execute the provided function on the item */                                                           \
        push(argv[1]);         /* Push the function */                                                            \
        push(items.values[i]); /* Push the item */                                                                \
        Value result = exec_callable(argv[1], 1);                                                                 \
        if (vm.flags & VM_FLAG_HAS_ERROR) {                                                                       \
          return nil_value(); /* Propagate the error */                                                           \
        }                                                                                                         \
                                                                                                                  \
        /* We don't use is_falsey here, because we want to check for a boolean value. */                          \
        if (IS_BOOL(result) && result.as.boolean) {                                                               \
          occurrences++;                                                                                          \
        }                                                                                                         \
      }                                                                                                           \
    } else {                                                                                                      \
      /* Value equality */                                                                                        \
      for (int i = 0; i < count; i++) {                                                                           \
        if (values_equal(argv[1], items.values[i])) {                                                             \
          occurrences++;                                                                                          \
        }                                                                                                         \
      }                                                                                                           \
    }                                                                                                             \
                                                                                                                  \
    return int_value(occurrences);                                                                                \
  }

/**
 * TYPENAME_T.concat(other: TYPENAME_T) -> TYPENAME_T
 * @brief Concatenates two TYPENAME_Ts. Returns a new TYPENAME_T with the items of the receiver followed by the items of 'other'.
 */
#define BUILTIN_LISTLIKE_CONCAT(type)                                            \
  BUILTIN_METHOD_IMPL(TYPENAME_##type, concat) {                                 \
    UNUSED(argc);                                                                \
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
    return BUILTIN_LISTLIKE_TAKE_ARRAY(concatenated);                            \
  }
