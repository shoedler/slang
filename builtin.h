#ifndef builtin_h
#define builtin_h

#include "builtin_util.h"
#include "value.h"

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

// Registers the built-in map class and its methods.
extern void register_builtin_map_class();

//
// Built-in function-, method- and class-declarations
//

// Built-in functions
BUILTIN_DECLARE_FN(clock);
BUILTIN_DECLARE_FN(log);
BUILTIN_DECLARE_FN(type_of);
BUILTIN_DECLARE_FN(type_name);
BUILTIN_DECLARE_FN(cwd);

// Obj built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_OBJ, hash)

// Bool built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_BOOL, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_BOOL, to_str)

// Nil built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_NIL, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_NIL, to_str)

// Number built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_NUMBER, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_NUMBER, to_str)

// String built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, len)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, split)
BUILTIN_DECLARE_METHOD(TYPENAME_STRING, trim)

// Seq built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, len)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, push)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, pop)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, has)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, first)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, last)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, each)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, map)
BUILTIN_DECLARE_METHOD(TYPENAME_SEQ, filter)

// Map built-in methods
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, __ctor)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, to_str)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, len)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, entries)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, values)
BUILTIN_DECLARE_METHOD(TYPENAME_MAP, keys)

#endif