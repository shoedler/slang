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

// Registers the built-in string class and its methods.
extern void register_builtin_str_class();

// Registers the built-in seq class and its methods.
extern void register_builtin_seq_class();

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
BUILTIN_DECLARE_METHOD(TYPENAME_NUMBER, SP_METHOD_CTOR)
BUILTIN_DECLARE_METHOD(TYPENAME_NUMBER, SP_METHOD_TO_STR)

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

#endif