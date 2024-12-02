#ifndef NATIVE_H
#define NATIVE_H

#include <stddef.h>
#include <stdlib.h>
#include "common.h"
#include "value.h"
#include "vm.h"

// Register native functions
extern void native_register_functions();

// Registers the native object class and its methods.
extern ObjClass* native_obj_class_partial_init();
extern void native_obj_class_finalize();

// Registers the native nil class and its methods.
extern ObjClass* native_nil_class_partial_init();
extern void native_nil_class_finalize();

// Registers the native bool class and its methods.
extern ObjClass* native_bool_class_partial_init();
extern void native_bool_class_finalize();

// Registers the native number class and its methods.
extern ObjClass* native_num_class_partial_init();
extern void native_num_class_finalize();

// Registers the native int class and its methods.
extern ObjClass* native_int_class_partial_init(ObjClass* num_base_class);
extern void native_int_class_finalize();

// Registers the native float class and its methoods.
extern ObjClass* native_float_class_partial_init(ObjClass* num_base_class);
extern void native_float_class_finalize();

// Registers the native string class and its methods.
extern ObjClass* native_str_class_partial_init();
extern void native_str_class_finalize();

// Registers the native seq class and its methods.
extern ObjClass* native_seq_class_partial_init();
extern void native_seq_class_finalize();

// Registers the native tuple class and its methods.
extern ObjClass* native_tuple_class_partial_init();
extern void native_tuple_class_finalize();

// Registers the native fn class and its methods.
extern ObjClass* native_fn_class_partial_init();
extern void native_fn_class_finalize();

// Registers the native class class and its methods.
extern ObjClass* native_class_class_partial_init();
extern void native_class_class_finalize();

// Registers the native file module
extern void native_register_file_module();

// Registers the native perf module
extern void native_register_perf_module();

// Registers the native debug module
extern void native_register_debug_module();

// Registers the native gc module
extern void native_register_gc_module();

// Registers the native math module
extern void native_register_math_module();

//
// Native functions
//

/**
 * clock() -> TYPENAME_FLOAT
 * @brief Returns the current execution time in seconds.
 */
extern Value native_clock(int argc, Value argv[]);

/**
 * cwd() -> TYPENAME_STRING
 * @brief Returns the current working directory or TYPENAME_NIL if no module is active.
 */
extern Value native_cwd(int argc, Value argv[]);

/**
 * log(arg1: TYPENAME_VALUE, ...args: TYPENAME_VALUE) -> TYPENAME_NIL
 * @brief Prints the arguments to the console.
 */
extern Value native_log(int argc, Value argv[]);

/**
 * typeof(value: TYPENAME_VALUE) -> TYPENAME_CLASS
 * @brief Returns the class of the value.
 */
extern Value native_typeof(int argc, Value argv[]);

//
// Default accessors for native classes.
//

// Default prop setter for types that don't support it.
bool native_set_prop_not_supported(Value receiver, ObjString* name, Value value);

// Default subs getter for types that don't support it.
bool native_get_subs_not_supported(Value receiver, Value index, Value* result);

// Default subs setter for types that don't support it.
bool native_set_subs_not_supported(Value receiver, Value index, Value value);

// Default equals for types that don't support it.
bool native_equals_not_supported(Value self, Value other);

// Default hash for types that don't support it.
uint64_t native_hash_not_supported(Value self);

// Default equals for object-based types.
bool native_default_obj_equals(Value self, Value other);

// Default hash for object-based types.
uint64_t native_default_obj_hash(Value self);

// Default prop getter for any type.
#define NATIVE_DEFAULT_GET_PROP_BODY(class)                                                       \
  if (bind_method(class, name, result)) {                                                         \
    return true;                                                                                  \
  }                                                                                               \
  vm_error("Property '%s' does not exist on value of type %s.", name->chars, class->name->chars); \
  return false;

//
// Macros for argument checking in native functions and general utilities.
//

#define NATIVE_BIN_OP_ILLEGAL_TYPES(op)                                                                  \
  vm_error("Incompatible types for binary operand '%s': %s %s %s.", #op, argv[0].type->name->chars, #op, \
           argv[1].type->name->chars);

#define NATIVE_CHECK_RECEIVER(class)                                                                       \
  if (argv[0].type != class) {                                                                             \
    vm_error("Expected receiver of type %s but got %s.", class->name->chars, (argv[0]).type->name->chars); \
    return nil_value();                                                                                    \
  }

#define NATIVE_CHECK_RECEIVER_INHERITS(class)                                                                      \
  if (!vm_inherits(argv[0].type, class)) {                                                                         \
    vm_error("Expected receiver to inherit type %s but got %s.", class->name->chars, (argv[0]).type->name->chars); \
    return nil_value();                                                                                            \
  }

#define NATIVE_CHECK_ARG_AT(index, class)                                                                                    \
  if ((argv[index]).type != class) {                                                                                         \
    vm_error("Expected argument %d of type %s but got %s.", index - 1, class->name->chars, (argv[index]).type->name->chars); \
    return nil_value();                                                                                                      \
  }

#define NATIVE_CHECK_ARG_AT_INHERITS(index, class)                                                       \
  if (!vm_inherits((argv[index]).type, class)) {                                                         \
    vm_error("Expected argument %d to be a descendant of %s but got %s.", index - 1, class->name->chars, \
             (argv[index]).type->name->chars);                                                           \
    return nil_value();                                                                                  \
  }

#define NATIVE_CHECK_ARG_AT_IS_CALLABLE(index)                                                             \
  if (!is_callable(argv[index])) {                                                                         \
    vm_error("Expected argument %d to be callable but got %s.", index - 1, argv[index].type->name->chars); \
    return nil_value();                                                                                    \
  }

//
// Macros for native listlike accessors
//

#define NATIVE_LISTLIKE_GET_PROP_BODY()                     \
  if (name == vm.special_prop_names[SPECIAL_PROP_LEN]) {    \
    ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(receiver); \
    *result          = int_value(items.count);              \
    return true;                                            \
  }                                                         \
  NATIVE_DEFAULT_GET_PROP_BODY(receiver.type)

#define NATIVE_LISTLIKE_GET_SUBS_BODY()                                                                                        \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(receiver);                                                                      \
  if (!is_int(index)) {                                                                                                        \
    vm_error("Type %s does not support get-subscripting with %s. Expected " STR(TYPENAME_INT) ".", receiver.type->name->chars, \
             index.type->name->chars);                                                                                         \
    return false;                                                                                                              \
  }                                                                                                                            \
  long long idx = index.as.integer;                                                                                            \
  if (idx >= items.count) {                                                                                                    \
    *result = nil_value();                                                                                                     \
    return true;                                                                                                               \
  }                                                                                                                            \
                                                                                                                               \
  /* Negative index */                                                                                                         \
  if (idx < 0) {                                                                                                               \
    idx += items.count;                                                                                                        \
  }                                                                                                                            \
  if (idx < 0) {                                                                                                               \
    *result = nil_value();                                                                                                     \
    return true;                                                                                                               \
  }                                                                                                                            \
  *result = items.values[idx];                                                                                                 \
  return true;

//
// Macros for native listlike methods.
//

/**
 * TYPENAME_T.SP_METHOD_HAS(value: TYPENAME_VALUE) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the TYPENAME_T contains a value which equals 'value'.
 *
 * TYPENAME_T.SP_METHOD_HAS(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the TYPENAME_T contains a value for which 'pred' evaluates to VALUE_STR_TRUE.
 */
#define NATIVE_LISTLIKE_HAS_BODY(class)                                              \
  UNUSED(argc);                                                                      \
  NATIVE_CHECK_RECEIVER(class)                                                       \
                                                                                     \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                             \
  if (items.count == 0) {                                                            \
    return bool_value(false);                                                        \
  }                                                                                  \
                                                                                     \
  int count = items.count; /* We need to store this. Might change during the loop */ \
                                                                                     \
  if (is_fn(argv[1])) {                                                              \
    /* Function predicate */                                                         \
    for (int i = 0; i < count; i++) {                                                \
      /* Execute the provided function on the item */                                \
      vm_push(argv[1]);         /* Push the function */                              \
      vm_push(items.values[i]); /* Push the item */                                  \
      Value result = vm_exec_callable(argv[1], 1);                                   \
      if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                          \
        return nil_value(); /* Propagate the error */                                \
      }                                                                              \
      /* We don't use vm_is_falsey here, because we want a boolean value. */         \
      if (is_bool(result) && result.as.boolean) {                                    \
        return bool_value(true);                                                     \
      }                                                                              \
    }                                                                                \
  } else {                                                                           \
    /* Value equality */                                                             \
    for (int i = 0; i < count; i++) {                                                \
      if (argv[1].type->__equals(argv[1], items.values[i])) {                        \
        return bool_value(true);                                                     \
      }                                                                              \
    }                                                                                \
  }                                                                                  \
                                                                                     \
  return bool_value(false);

/**
 * TYPENAME_T.SP_METHOD_TO_STR() -> TYPENAME_STRING
 * @brief Returns a string representation of the TYPENAME_T.
 */
#define NATIVE_LISTLIKE_TO_STR_BODY(class, start_chars, delim_chars, end_chars)                                       \
  UNUSED(argc);                                                                                                       \
  NATIVE_CHECK_RECEIVER(class)                                                                                        \
                                                                                                                      \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                              \
  size_t buf_size  = 64; /* Start with a reasonable size */                                                           \
  char* chars      = malloc(buf_size);                                                                                \
                                                                                                                      \
  strcpy(chars, start_chars);                                                                                         \
  for (int i = 0; i < items.count; i++) {                                                                             \
    /* Execute the to_str method on the item */                                                                       \
    vm_push(items.values[i]); /* Push the receiver (item at i) for to_str */                                          \
    ObjString* item_str = AS_STR(vm_exec_callable(fn_value(items.values[i].type->__to_str), 0));                      \
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                             \
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
  return str_value(str_obj);

/**
 * TYPENAME_T.SP_METHOD_SLICE(start: TYPENAME_INT, end: TYPENAME_INT | TYPENAME_NIL) -> TYPENAME_T
 * @brief Returns a new TYPENAME_T containing the items from 'start' to 'end' ('end' is exclusive).
 * 'end' can be negative to count from the end of the TYPENAME_T. If 'start' is greater than or equal to 'end', an empty
 * TYPENAME_T is returned. If 'end' is TYPENAME_NIL, all items from 'start' to the end of the TYPENAME_T are included.
 */
#define NATIVE_LISTLIKE_SLICE_BODY(class)                                                   \
  UNUSED(argc);                                                                             \
  NATIVE_CHECK_RECEIVER(class)                                                              \
  NATIVE_CHECK_ARG_AT(1, vm.int_class)                                                      \
  if (is_nil(argv[2])) {                                                                    \
    argv[2] = int_value(AS_SEQ(argv[0])->items.count); /* hack: use Seq to extract count */ \
  }                                                                                         \
  NATIVE_CHECK_ARG_AT(2, vm.int_class)                                                      \
                                                                                            \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                    \
  int count        = items.count;                                                           \
                                                                                            \
  if (count == 0) {                                                                         \
    return NATIVE_LISTLIKE_NEW_EMPTY();                                                     \
  }                                                                                         \
                                                                                            \
  int start = (int)argv[1].as.integer;                                                      \
  int end   = (int)argv[2].as.integer;                                                      \
                                                                                            \
  /* Handle negative indices */                                                             \
  if (start < 0) {                                                                          \
    start = count + start;                                                                  \
  }                                                                                         \
  if (end < 0) {                                                                            \
    end = count + end;                                                                      \
  }                                                                                         \
                                                                                            \
  /* Clamp out-of-bounds indices */                                                         \
  if (start < 0) {                                                                          \
    start = 0;                                                                              \
  }                                                                                         \
  if (end > count) {                                                                        \
    end = count;                                                                            \
  }                                                                                         \
                                                                                            \
  /* Handle invalid or 0 length ranges */                                                   \
  if (start >= end) {                                                                       \
    return NATIVE_LISTLIKE_NEW_EMPTY();                                                     \
  }                                                                                         \
  ValueArray sliced = value_array_init_of_size(end - start);                                \
  for (int i = start; i < end; i++) {                                                       \
    sliced.values[sliced.count++] = items.values[i];                                        \
  }                                                                                         \
                                                                                            \
  /* No need for GC protection - taking an array will not trigger a GC. */                  \
  return NATIVE_LISTLIKE_TAKE_ARRAY(sliced);

/**
 * TYPENAME_T.pos(value: TYPENAME_VALUE) -> TYPENAME_INT
 * @brief Returns the index of the first item which equals 'value' in a TYPENAME_T. Returns VALUE_STR_NIL if 'value' is not found.
 *
 * TYPENAME_T.pos(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_INT
 * @brief Returns the index of the first item in a TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE. Returns VALUE_STR_NIL
 * if no item satisfies the predicate.
 */
#define NATIVE_LISTLIKE_POS_BODY(listlike_type)                                                           \
  UNUSED(argc);                                                                                           \
  NATIVE_CHECK_RECEIVER(listlike_type)                                                                    \
                                                                                                          \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                  \
  if (items.count == 0) {                                                                                 \
    return nil_value();                                                                                   \
  }                                                                                                       \
                                                                                                          \
  int count = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                          \
  if (is_fn(argv[1])) {                                                                                   \
    /* Function predicate */                                                                              \
    for (int i = 0; i < count; i++) {                                                                     \
      /* Execute the provided function on the item */                                                     \
      vm_push(argv[1]);         /* Push the function */                                                   \
      vm_push(items.values[i]); /* Push the item */                                                       \
      Value result = vm_exec_callable(argv[1], 1);                                                        \
      if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                               \
        return nil_value(); /* Propagate the error */                                                     \
      }                                                                                                   \
                                                                                                          \
      /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                 \
      if (is_bool(result) && result.as.boolean) {                                                         \
        return int_value(i);                                                                              \
      }                                                                                                   \
    }                                                                                                     \
  } else {                                                                                                \
    /* Value equality */                                                                                  \
    for (int i = 0; i < count; i++) {                                                                     \
      if (argv[1].type->__equals(argv[1], items.values[i])) {                                             \
        return int_value(i);                                                                              \
      }                                                                                                   \
    }                                                                                                     \
  }                                                                                                       \
                                                                                                          \
  return nil_value();

/**
 * TYPENAME_T.first(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_VALUE
 * @brief Returns the first item of a TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE. Returns VALUE_STR_NIL if the
 * TYPENAME_T is empty or no item satisfies the predicate.
 */
#define NATIVE_LISTLIKE_FIRST_BODY(type)                                                                  \
  UNUSED(argc);                                                                                           \
  NATIVE_CHECK_RECEIVER(type)                                                                             \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                      \
                                                                                                          \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                  \
  if (items.count == 0) {                                                                                 \
    return nil_value();                                                                                   \
  }                                                                                                       \
                                                                                                          \
  int count = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                          \
  /* Function predicate */                                                                                \
  for (int i = 0; i < count; i++) {                                                                       \
    /* Execute the provided function on the item */                                                       \
    vm_push(argv[1]);         /* Push the function */                                                     \
    vm_push(items.values[i]); /* Push the item */                                                         \
    Value result = vm_exec_callable(argv[1], 1);                                                          \
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                 \
      return nil_value(); /* Propagate the error */                                                       \
    }                                                                                                     \
                                                                                                          \
    /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                   \
    if (is_bool(result) && result.as.boolean) {                                                           \
      return items.values[i];                                                                             \
    }                                                                                                     \
  }                                                                                                       \
                                                                                                          \
  return nil_value();

/**
 * TYPENAME_T.last(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_VALUE
 * @brief Returns the last item of a TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE. Returns VALUE_STR_NIL if the
 * TYPENAME_T is empty or no item satisfies the predicate.
 */
#define NATIVE_LISTLIKE_LAST_BODY(type)                                                                   \
  UNUSED(argc);                                                                                           \
  NATIVE_CHECK_RECEIVER(type)                                                                             \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                      \
                                                                                                          \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                  \
  if (items.count == 0) {                                                                                 \
    return nil_value();                                                                                   \
  }                                                                                                       \
                                                                                                          \
  int count = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                          \
  /* Function predicate */                                                                                \
  for (int i = count - 1; i >= 0; i--) {                                                                  \
    /* Execute the provided function on the item */                                                       \
    vm_push(argv[1]);         /* Push the function */                                                     \
    vm_push(items.values[i]); /* Push the item */                                                         \
    Value result = vm_exec_callable(argv[1], 1);                                                          \
    if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                 \
      return nil_value(); /* Propagate the error */                                                       \
    }                                                                                                     \
                                                                                                          \
    /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                   \
    if (is_bool(result) && result.as.boolean) {                                                           \
      return items.values[i];                                                                             \
    }                                                                                                     \
  }                                                                                                       \
                                                                                                          \
  return nil_value();

/**
 * TYPENAME_T.each(fn: TYPENAME_FUNCTION) -> TYPENAME_NIL
 * @brief Executes 'fn' for each item in the TYPENAME_T. 'fn' should take one or two arguments: the item and the index of the
 * item. The latter is optional.
 */
#define NATIVE_LISTLIKE_EACH_BODY(type)                                                                          \
  UNUSED(argc);                                                                                                  \
  NATIVE_CHECK_RECEIVER(type)                                                                                    \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                             \
                                                                                                                 \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                         \
  int fn_arity     = callable_get_arity(argv[1]);                                                                \
  int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                 \
  /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
  switch (fn_arity) {                                                                                            \
    case 1: {                                                                                                    \
      for (int i = 0; i < count; i++) {                                                                          \
        /* Execute the provided function on the item */                                                          \
        vm_push(argv[1]);         /* Push the function */                                                        \
        vm_push(items.values[i]); /* arg0: Push the item */                                                      \
        vm_exec_callable(argv[1], 1);                                                                            \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                    \
          return nil_value(); /* Propagate the error */                                                          \
        }                                                                                                        \
      }                                                                                                          \
      break;                                                                                                     \
    }                                                                                                            \
    case 2: {                                                                                                    \
      for (int i = 0; i < count; i++) {                                                                          \
        /* Execute the provided function on the item */                                                          \
        vm_push(argv[1]);         /* Push the function */                                                        \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                  \
        vm_push(int_value(i));    /* arg1 (2): Push the index */                                                 \
        vm_exec_callable(argv[1], 2);                                                                            \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                    \
          return nil_value(); /* Propagate the error */                                                          \
        }                                                                                                        \
      }                                                                                                          \
      break;                                                                                                     \
    }                                                                                                            \
    default: {                                                                                                   \
      vm_error("Function passed to \"" STR(each) "\" must take 1 or 2 arguments, but got %d.", fn_arity);        \
      return nil_value();                                                                                        \
    }                                                                                                            \
  }                                                                                                              \
                                                                                                                 \
  return nil_value();

/**
 * TYPENAME_T.map(fn: TYPENAME_FUNCTION) -> TYPENAME_T
 * @brief Maps each item in the TYPENAME_T to a new value by executing 'fn' on it. Returns a new TYPENAME_T containing the mapped
 * values. 'fn' should take one or two arguments: the item and the index of the item. The latter is optional.
 */
#define NATIVE_LISTLIKE_MAP_BODY(type)                                                                            \
  UNUSED(argc);                                                                                                   \
  NATIVE_CHECK_RECEIVER(type)                                                                                     \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                              \
                                                                                                                  \
  ValueArray items  = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                         \
  int fn_arity      = callable_get_arity(argv[1]);                                                                \
  int count         = items.count; /* We need to store this, because the listlike might change during the loop */ \
  ValueArray mapped = value_array_init_of_size(count);                                                            \
                                                                                                                  \
  /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                        \
  switch (fn_arity) {                                                                                             \
    case 1: {                                                                                                     \
      while (mapped.count < count) {                                                                              \
        /* Execute the provided function on the item */                                                           \
        vm_push(argv[1]);                    /* Push the function */                                              \
        vm_push(items.values[mapped.count]); /* arg0 (1): Push the item */                                        \
        mapped.values[mapped.count] = vm_exec_callable(argv[1], 1);                                               \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                     \
          return nil_value(); /* Propagate the error */                                                           \
        }                                                                                                         \
        vm_push(mapped.values[mapped.count]); /* GC Protection */                                                 \
        mapped.count++;                                                                                           \
      }                                                                                                           \
      break;                                                                                                      \
    }                                                                                                             \
    case 2: {                                                                                                     \
      while (mapped.count < count) {                                                                              \
        /* Execute the provided function on the item */                                                           \
        vm_push(argv[1]);                    /* Push the function */                                              \
        vm_push(items.values[mapped.count]); /* arg0 (1): Push the item */                                        \
        vm_push(int_value(mapped.count));    /* arg1 (2): Push the index */                                       \
        mapped.values[mapped.count] = vm_exec_callable(argv[1], 2);                                               \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                     \
          return nil_value(); /* Propagate the error */                                                           \
        }                                                                                                         \
        vm_push(mapped.values[mapped.count]); /* GC Protection */                                                 \
        mapped.count++;                                                                                           \
      }                                                                                                           \
      break;                                                                                                      \
    }                                                                                                             \
    default: {                                                                                                    \
      vm_error("Function passed to \"" STR(map) "\" must take 1 or 2 arguments, but got %d.", fn_arity);          \
      return nil_value();                                                                                         \
    }                                                                                                             \
  }                                                                                                               \
                                                                                                                  \
  /* Take at the end so that tuples calc their hash correctly */                                                  \
  vm_push(NATIVE_LISTLIKE_TAKE_ARRAY(mapped));                                                                    \
  Value result = vm_pop();                                                                                        \
                                                                                                                  \
  /* Remove the values which were pushed for GC Protection */                                                     \
  vm.stack_top -= count;                                                                                          \
  return result;

/**
 * TYPENAME_T.sift(fn: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_T
 * @brief Filters the items of a TYPENAME_T by executing 'fn' on each item. Returns a new TYPENAME_T with the items for which 'fn'
 * evaluates to VALUE_STR_TRUE. 'fn' should take one or two arguments: the item and the index of the item. The latter is optional.
 */
#define NATIVE_LISTLIKE_SIFT_BODY(type)                                                                               \
  UNUSED(argc);                                                                                                       \
  NATIVE_CHECK_RECEIVER(type)                                                                                         \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                                  \
                                                                                                                      \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                              \
  int fn_arity     = callable_get_arity(argv[1]);                                                                     \
  int count        = items.count; /* We need to store this, because the listlike might change during the loop */      \
                                                                                                                      \
  ValueArray filtered_items;                                                                                          \
  value_array_init(&filtered_items);                                                                                  \
  int filtered_count = 0; /* Need to track this so we can clean the stack from the pushed values for GC protection */ \
                                                                                                                      \
  /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                            \
  switch (fn_arity) {                                                                                                 \
    case 1: {                                                                                                         \
      for (int i = 0; i < count; i++) {                                                                               \
        /* Execute the provided function on the item */                                                               \
        vm_push(argv[1]);         /* Push the function */                                                             \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                       \
        Value result = vm_exec_callable(argv[1], 1);                                                                  \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                         \
          return nil_value(); /* Propagate the error */                                                               \
        }                                                                                                             \
                                                                                                                      \
        /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                           \
        if (is_bool(result) && result.as.boolean) {                                                                   \
          vm_push(result); /* GC Protection */                                                                        \
          filtered_count++;                                                                                           \
          value_array_write(&filtered_items, items.values[i]);                                                        \
        }                                                                                                             \
      }                                                                                                               \
      break;                                                                                                          \
    }                                                                                                                 \
    case 2: {                                                                                                         \
      for (int i = 0; i < count; i++) {                                                                               \
        /* Execute the provided function on the item */                                                               \
        vm_push(argv[1]);         /* Push the function */                                                             \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                       \
        vm_push(int_value(i));    /* arg1 (2): Push the index */                                                      \
        Value result = vm_exec_callable(argv[1], 2);                                                                  \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                         \
          return nil_value(); /* Propagate the error */                                                               \
        }                                                                                                             \
                                                                                                                      \
        /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                           \
        if (is_bool(result) && result.as.boolean) {                                                                   \
          vm_push(result); /* GC Protection */                                                                        \
          filtered_count++;                                                                                           \
          value_array_write(&filtered_items, items.values[i]);                                                        \
        }                                                                                                             \
      }                                                                                                               \
      break;                                                                                                          \
    }                                                                                                                 \
    default: {                                                                                                        \
      vm_error("Function passed to \"" STR(sift) "\" must take 1 or 2 arguments, but got %d.", fn_arity);             \
      return nil_value();                                                                                             \
    }                                                                                                                 \
  }                                                                                                                   \
                                                                                                                      \
  /* Take at the end so that tuples calc their hash correctly */                                                      \
  vm_push(NATIVE_LISTLIKE_TAKE_ARRAY(filtered_items));                                                                \
  Value result = vm_pop();                                                                                            \
                                                                                                                      \
  /* Remove the values which were pushed for GC Protection */                                                         \
  vm.stack_top -= filtered_count;                                                                                     \
  return result;

/**
 * TYPENAME_T.join(sep: TYPENAME_STRING) -> TYPENAME_STRING
 * @brief Joins the items of a TYPENAME_T into a single TYPENAME_STRING, separated by 'sep'.
 */
#define NATIVE_LISTLIKE_JOIN_BODY(class)                                                               \
  UNUSED(argc);                                                                                        \
  NATIVE_CHECK_RECEIVER(class)                                                                         \
  NATIVE_CHECK_ARG_AT(1, vm.str_class)                                                                 \
                                                                                                       \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                               \
  ObjString* sep   = AS_STR(argv[1]);                                                                  \
                                                                                                       \
  size_t buf_size = 64; /* Start with a reasonable size */                                             \
  char* chars     = malloc(buf_size);                                                                  \
  chars[0]        = '\0'; /* Start with an empty string, so we can use strcat */                       \
                                                                                                       \
  for (int i = 0; i < items.count; i++) {                                                              \
    /* Maybe this is faster (checking if the item is a string)  - unsure though */                     \
    ObjString* item_str = NULL;                                                                        \
    if (!is_str(items.values[i])) {                                                                    \
      /* Execute the to_str method on the item */                                                      \
      vm_push(items.values[i]); /* Push the receiver (item at i) for to_str, or */                     \
      item_str = AS_STR(vm_exec_callable(fn_value(items.values[i].type->__to_str), 0));                \
      if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                            \
        return nil_value();                                                                            \
      }                                                                                                \
    } else {                                                                                           \
      item_str = AS_STR(items.values[i]);                                                              \
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
  return str_value(str_obj);

/**
 * TYPENAME_T.flip() -> TYPENAME_T
 * @brief Reverses the items of a TYPENAME_T. Returns a new TYPENAME_T with the items in flip order.
 */
#define NATIVE_LISTLIKE_FLIP_BODY(class)                                   \
  UNUSED(argc);                                                            \
  NATIVE_CHECK_RECEIVER(class)                                             \
                                                                           \
  ValueArray items    = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                \
  ValueArray reversed = value_array_init_of_size(items.count);             \
                                                                           \
  /* No GC protection needed */                                            \
  for (int i = items.count - 1; i >= 0; i--) {                             \
    reversed.values[items.count - 1 - i] = items.values[i];                \
  }                                                                        \
  reversed.count = items.count;                                            \
                                                                           \
  /* No need for GC protection - taking an array will not trigger a GC. */ \
  return NATIVE_LISTLIKE_TAKE_ARRAY(reversed);

/**
 * TYPENAME_T.every(fn: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if 'fn' evaluates to VALUE_STR_TRUE for every item in the TYPENAME_T. 'fn' should take one or two
 * arguments: the item and the index of the item. The latter is optional. Returns VALUE_STR_FALSE if the TYPENAME_T is empty.
 */
#define NATIVE_LISTLIKE_EVERY_BODY(class)                                                                        \
  UNUSED(argc);                                                                                                  \
  NATIVE_CHECK_RECEIVER(class)                                                                                   \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                             \
                                                                                                                 \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                         \
  int fn_arity     = callable_get_arity(argv[1]);                                                                \
  int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                 \
  /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
  switch (fn_arity) {                                                                                            \
    case 1: {                                                                                                    \
      for (int i = 0; i < count; i++) {                                                                          \
        /* Execute the provided function on the item */                                                          \
        vm_push(argv[1]);         /* Push the function */                                                        \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                  \
        Value result = vm_exec_callable(argv[1], 1);                                                             \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                    \
          return nil_value(); /* Propagate the error */                                                          \
        }                                                                                                        \
                                                                                                                 \
        /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                      \
        if (!is_bool(result) || !result.as.boolean) {                                                            \
          return bool_value(false);                                                                              \
        }                                                                                                        \
      }                                                                                                          \
      break;                                                                                                     \
    }                                                                                                            \
    case 2: {                                                                                                    \
      for (int i = 0; i < count; i++) {                                                                          \
        /* Execute the provided function on the item */                                                          \
        vm_push(argv[1]);         /* Push the function */                                                        \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                  \
        vm_push(int_value(i));    /* arg1 (2): Push the index */                                                 \
        Value result = vm_exec_callable(argv[1], 2);                                                             \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                    \
          return nil_value(); /* Propagate the error */                                                          \
        }                                                                                                        \
                                                                                                                 \
        /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                      \
        if (!is_bool(result) || !result.as.boolean) {                                                            \
          return bool_value(false);                                                                              \
        }                                                                                                        \
      }                                                                                                          \
      break;                                                                                                     \
    }                                                                                                            \
    default: {                                                                                                   \
      vm_error("Function passed to \"" STR(every) "\" must take 1 or 2 arguments, but got %d.", fn_arity);       \
      return nil_value();                                                                                        \
    }                                                                                                            \
  }                                                                                                              \
  if (fn_arity > 1) {                                                                                            \
  } else {                                                                                                       \
  }                                                                                                              \
                                                                                                                 \
  return bool_value(true);

/**
 * TYPENAME_T.some(fn: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if 'fn' evaluates to VALUE_STR_TRUE for at least one item in the TYPENAME_T. 'fn' should take one
 * or two arguments: the item and the index of the item. The latter is optional. Returns VALUE_STR_FALSE if the TYPENAME_T is
 * empty.
 */
#define NATIVE_LISTLIKE_SOME_BODY(class)                                                                         \
  UNUSED(argc);                                                                                                  \
  NATIVE_CHECK_RECEIVER(class)                                                                                   \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(1)                                                                             \
                                                                                                                 \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                         \
  int fn_arity     = callable_get_arity(argv[1]);                                                                \
  int count        = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                 \
  /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                       \
  switch (fn_arity) {                                                                                            \
    case 1: {                                                                                                    \
      for (int i = 0; i < count; i++) {                                                                          \
        /* Execute the provided function on the item */                                                          \
        vm_push(argv[1]);         /* Push the function */                                                        \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                  \
        Value result = vm_exec_callable(argv[1], 1);                                                             \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                    \
          return nil_value(); /* Propagate the error */                                                          \
        }                                                                                                        \
                                                                                                                 \
        /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                      \
        if (is_bool(result) && result.as.boolean) {                                                              \
          return bool_value(true);                                                                               \
        }                                                                                                        \
      }                                                                                                          \
      break;                                                                                                     \
    }                                                                                                            \
    case 2: {                                                                                                    \
      for (int i = 0; i < count; i++) {                                                                          \
        /* Execute the provided function on the item */                                                          \
        vm_push(argv[1]);         /* Push the function */                                                        \
        vm_push(items.values[i]); /* arg0 (1): Push the item */                                                  \
        vm_push(int_value(i));    /* arg1 (2): Push the index */                                                 \
        Value result = vm_exec_callable(argv[1], 2);                                                             \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                    \
          return nil_value(); /* Propagate the error */                                                          \
        }                                                                                                        \
                                                                                                                 \
        /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                      \
        if (is_bool(result) && result.as.boolean) {                                                              \
          return bool_value(true);                                                                               \
        }                                                                                                        \
      }                                                                                                          \
      break;                                                                                                     \
    }                                                                                                            \
    default: {                                                                                                   \
      vm_error("Function passed to \"" STR(some) "\" must take 1 or 2 arguments, but got %d.", fn_arity);        \
      return nil_value();                                                                                        \
    }                                                                                                            \
  }                                                                                                              \
                                                                                                                 \
  return bool_value(false);

/**
 * TYPENAME_T.fold(initial: TYPENAME_VALUE, fn: TYPENAME_FUNCTION) -> TYPENAME_VALUE
 * @brief Reduces the items of a TYPENAME_T to a single value by executing 'fn' on each item. 'fn' should take two or three
 * arguments: the accumulator, the item and the index. The latter is optional. The initial value of the accumulator is 'initial'.
 */
#define NATIVE_LISTLIKE_FOLD_BODY(class)                                                                          \
  UNUSED(argc);                                                                                                   \
  NATIVE_CHECK_RECEIVER(class)                                                                                    \
  NATIVE_CHECK_ARG_AT_IS_CALLABLE(2)                                                                              \
                                                                                                                  \
  ValueArray items  = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                         \
  Value accumulator = argv[1];                                                                                    \
  int fn_arity      = callable_get_arity(argv[2]);                                                                \
  int count         = items.count; /* We need to store this, because the listlike might change during the loop */ \
                                                                                                                  \
  /* Loops are duplicated to avoid the overhead of checking the arity on each iteration */                        \
  switch (fn_arity) {                                                                                             \
    case 2: {                                                                                                     \
      for (int i = 0; i < count; i++) {                                                                           \
        /* Execute the provided function on the item */                                                           \
        vm_push(argv[2]);         /* Push the function */                                                         \
        vm_push(accumulator);     /* arg0 (1): Push the accumulator */                                            \
        vm_push(items.values[i]); /* arg1 (2): Push the item */                                                   \
        accumulator = vm_exec_callable(argv[2], 2);                                                               \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                     \
          return nil_value(); /* Propagate the error */                                                           \
        }                                                                                                         \
      }                                                                                                           \
      break;                                                                                                      \
    }                                                                                                             \
    case 3: {                                                                                                     \
      for (int i = 0; i < count; i++) {                                                                           \
        /* Execute the provided function on the item */                                                           \
        vm_push(argv[2]);         /* Push the function */                                                         \
        vm_push(accumulator);     /* arg0 (1): Push the accumulator */                                            \
        vm_push(items.values[i]); /* arg1 (2): Push the item */                                                   \
        vm_push(int_value(i));    /* arg2 (3): Push the index */                                                  \
        accumulator = vm_exec_callable(argv[2], 3);                                                               \
        if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                     \
          return nil_value(); /* Propagate the error */                                                           \
        }                                                                                                         \
      }                                                                                                           \
      break;                                                                                                      \
    }                                                                                                             \
    default: {                                                                                                    \
      vm_error("Function passed to \"" STR(fold) "\" must take 2 or 3 arguments, but got %d.", fn_arity);         \
      return nil_value();                                                                                         \
    }                                                                                                             \
  }                                                                                                               \
                                                                                                                  \
  return accumulator;

/**
 * TYPENAME_T.count(value: TYPENAME_VALUE) -> TYPENAME_INT
 * @brief Returns the number of occurrences of 'value' in the TYPENAME_T.
 *
 * TYPENAME_T.count(pred: TYPENAME_FUNCTION -> TYPENAME_BOOL) -> TYPENAME_INT
 * @brief Returns the number of items in the TYPENAME_T for which 'pred' evaluates to VALUE_STR_TRUE.
 */
#define NATIVE_LISTLIKE_COUNT_BODY(class)                                                                       \
  UNUSED(argc);                                                                                                 \
  NATIVE_CHECK_RECEIVER(class)                                                                                  \
                                                                                                                \
  ValueArray items = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                                                        \
  if (items.count == 0) {                                                                                       \
    return int_value(0);                                                                                        \
  }                                                                                                             \
                                                                                                                \
  int count       = items.count; /* We need to store this, because the listlike might change during the loop */ \
  int occurrences = 0;                                                                                          \
                                                                                                                \
  if (is_fn(argv[1])) {                                                                                         \
    int fn_arity = callable_get_arity(argv[1]);                                                                 \
    if (fn_arity != 1) {                                                                                        \
      vm_error("Function passed to \"" STR(count) "\" must take 1 argument, but got %d.", fn_arity);            \
      return nil_value();                                                                                       \
    }                                                                                                           \
                                                                                                                \
    /* Function predicate */                                                                                    \
    for (int i = 0; i < count; i++) {                                                                           \
      /* Execute the provided function on the item */                                                           \
      vm_push(argv[1]);         /* Push the function */                                                         \
      vm_push(items.values[i]); /* Push the item */                                                             \
      Value result = vm_exec_callable(argv[1], 1);                                                              \
      if (VM_HAS_FLAG(VM_FLAG_HAS_ERROR)) {                                                                     \
        return nil_value(); /* Propagate the error */                                                           \
      }                                                                                                         \
                                                                                                                \
      /* We don't use vm_is_falsey here, because we want to check for a boolean value. */                       \
      if (is_bool(result) && result.as.boolean) {                                                               \
        occurrences++;                                                                                          \
      }                                                                                                         \
    }                                                                                                           \
  } else {                                                                                                      \
    /* Value equality */                                                                                        \
    for (int i = 0; i < count; i++) {                                                                           \
      if (argv[1].type->__equals(argv[1], items.values[i])) {                                                   \
        occurrences++;                                                                                          \
      }                                                                                                         \
    }                                                                                                           \
  }                                                                                                             \
                                                                                                                \
  return int_value(occurrences);

/**
 * TYPENAME_T.concat(other: TYPENAME_T) -> TYPENAME_T
 * @brief Concatenates two TYPENAME_Ts. Returns a new TYPENAME_T with the items of the receiver followed by the items of 'other'.
 */
#define NATIVE_LISTLIKE_CONCAT_BODY(class)                                         \
  UNUSED(argc);                                                                    \
  NATIVE_CHECK_RECEIVER(class)                                                     \
  NATIVE_CHECK_ARG_AT(1, class)                                                    \
                                                                                   \
  ValueArray items1 = NATIVE_LISTLIKE_GET_ARRAY(argv[0]);                          \
  ValueArray items2 = NATIVE_LISTLIKE_GET_ARRAY(argv[1]);                          \
                                                                                   \
  ValueArray concatenated = value_array_init_of_size(items1.count + items2.count); \
  for (int i = 0; i < items1.count; i++) {                                         \
    concatenated.values[i] = items1.values[i];                                     \
  }                                                                                \
                                                                                   \
  for (int i = 0; i < items2.count; i++) {                                         \
    concatenated.values[items1.count + i] = items2.values[i];                      \
  }                                                                                \
  concatenated.count = items1.count + items2.count;                                \
                                                                                   \
  /* No need for GC protection - taking an array will not trigger a GC. */         \
  return NATIVE_LISTLIKE_TAKE_ARRAY(concatenated);

#endif  // NATIVE_H
