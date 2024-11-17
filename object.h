#ifndef object_h
#define object_h

#include <stdatomic.h>
#include "chunk.h"
#include "common.h"
#include "hashtable.h"
#include "value.h"

// The type of an object. Tells the garbage collector how to free the object.
typedef enum {
  OBJ_GC_CLASS,
  OBJ_GC_CLOSURE,
  OBJ_GC_FUNCTION,
  OBJ_GC_OBJECT,
  OBJ_GC_NATIVE,
  OBJ_GC_SEQ,
  OBJ_GC_TUPLE,
  OBJ_GC_STRING,
  OBJ_GC_UPVALUE,
  OBJ_GC_BOUND_METHOD,
} ObjGcType;

// The base object construct.
struct Obj {
  ObjGcType type;
  atomic_bool is_marked;
  uint64_t hash;
  struct Obj* next;
};

struct ObjSeq {
  Obj obj;
  ValueArray items;
};

struct ObjTuple {
  Obj obj;
  ValueArray items;
};

struct ObjObject;

typedef struct {
  Obj obj;
  int arity;
  int upvalue_count;
  Chunk chunk;
  ObjString* name;
  struct ObjObject* globals_context;
} ObjFunction;

// The type of a native function. Native functions are functions that are
// implemented in C and are not part of the language itself. Things to note:
// - argc is the number of arguments the function takes. If you provide argc=1, a signature of native(a) will
// be expected.
// - argv is the array of arguments the function takes. argv[0] is ALWAYS the receiver (in case of
// invocations) or the ObjNative itself. Same as in managed functions.
typedef Value (*NativeFn)(int argc, Value argv[]);

typedef struct {
  Obj obj;
  NativeFn function;
  ObjString* name;
  int arity;
} ObjNative;

struct ObjString {
  Obj obj;
  int length;
  char* chars;
};

typedef struct ObjUpvalue {
  Obj obj;
  Value* location;
  Value closed;
  struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
  Obj obj;
  ObjFunction* function;
  ObjUpvalue** upvalues;
  int upvalue_count;
} ObjClosure;

// Prop-getter. Returns false if an error occurred, true otherwise.
typedef bool (*GetPropFn)(Value receiver, ObjString* name, Value* result);
// Prop-setter. Returns false if an error occurred, true otherwise.
typedef bool (*SetPropFn)(Value receiver, ObjString* name, Value value);
// Subscript-getter. Returns false if an error occurred, true otherwise.
typedef bool (*GetSubscriptFn)(Value receiver, Value index, Value* result);
// Subscript-setter. Returns false if an error occurred, true otherwise.
typedef bool (*SetSubscriptFn)(Value receiver, Value index, Value value);

typedef struct ObjClass {
  Obj obj;
  ObjString* name;
  HashTable methods;
  HashTable static_methods;
  struct ObjClass* base;

  GetPropFn __get_prop;
  SetPropFn __set_prop;
  GetSubscriptFn __get_subs;
  SetSubscriptFn __set_subs;

  // Special methods for quick access.
  Obj* __ctor;
  Obj* __has;
  Obj* __to_str;
  Obj* __slice;
} ObjClass;

typedef struct ObjObject {
  Obj obj;
  ObjClass* instance_class;  // Only used for garbage collection, to be able to mark the class of an object.
  HashTable fields;
} ObjObject;

typedef struct {
  Obj obj;
  Value receiver;
  Obj* method;
} ObjBoundMethod;

// Creates, initializes and allocates a new bound method object. Might trigger
// garbage collection.
ObjBoundMethod* new_bound_method(Value receiver, Obj* method);

// Creates, initializes and allocates a new object. Might trigger
// garbage collection.
ObjObject* new_instance(ObjClass* klass);

// Creates, initializes and allocates a new class object. Might trigger garbage
// collection. Must be finalized with finalize_new_class at some point.
ObjClass* new_class(ObjString* name, ObjClass* base);

// Finalizes a new class object. This is used to set up the special methods and properties for quick access.
// Also ensures that the class inherits the special methods and properties from its base class.
void finalize_new_class(ObjClass* klass);

// Creates, initializes and allocates a new closure object. Might trigger
// garbage collection.
ObjClosure* new_closure(ObjFunction* function);

// Creates, initializes and allocates a new function object. Might trigger
// garbage collection.
ObjFunction* new_function();

// Creates, initializes and allocates a new seq object. Might trigger garbage
// collection.
ObjSeq* new_seq();

// Creates, initializes and allocates a new tuple object. Might trigger garbage
// collection.
ObjTuple* new_tuple();

// Creates, initializes and allocates a new native function object. Might
// trigger garbage collection.
ObjNative* new_native(NativeFn function, ObjString* name, int arity);

// Creates, initializes and allocates a new upvalue object. Might trigger
// garbage collection.
ObjUpvalue* new_upvalue(Value* slot);

// Copies a string literal into a heap-allocated string object.
// This does not take ownership of the string, but rather - as the name suggests - copies it. Might
// trigger garbage collection.
ObjString* copy_string(const char* chars, int length);

// Creates, initializes and allocates a new value array for listlike objects (Seq, Tuple). Initializes the
// value array and a capacity to add 'count' values without resizing. It's intended to add items
// directly to items.values[idx], no need to use write_value_array. You *MUST* fill the array up to 'count' with some sort of
// value. Might trigger garbage collection.
ValueArray prealloc_value_array(int count);

// Creates a string object from a C string.
// This takes ownership of the string. This means that the string will be freed
// when the object is freed. Might trigger garbage collection.
ObjString* take_string(char* chars, int length);

// Creates a new seq object from a value array.
// This takes ownership of the value array. This means that the value array will
// be freed when the object is freed. Specifically does not trigger garbage collection, such that you can fill the value array
// beforehand with unreachable values without worrying that this operation might free them.
ObjSeq* take_seq(ValueArray* items);

// Creates a new tuple object from a value array.
// After this, the tuple cannot be modified, since the hash is calculated here.
// This takes ownership of the value array. This means that the value array will
// be freed when the object is freed. Specifically does not trigger garbage collection, such that you can fill the value array
// beforehand with unreachable values without worrying that this operation might free them.
ObjTuple* take_tuple(ValueArray* items);

// Creates a new object from a hashtable.
// This takes ownership of the hashtable. This means that the hashtable will be
// freed when the object is freed. Might trigger garbage collection.
ObjObject* take_object(HashTable* fields);

#endif
