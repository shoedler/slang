#ifndef object_h
#define object_h

#include "chunk.h"
#include "common.h"
#include "hashtable.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

// Determines whether a value is of type bound method.
#define IS_BOUND_METHOD(value) is_obj_type(value, OBJ_BOUND_METHOD)

// Determines whether a value is of type class.
#define IS_CLASS(value) is_obj_type(value, OBJ_CLASS)

// Determines whether a value is of type closure.
#define IS_CLOSURE(value) is_obj_type(value, OBJ_CLOSURE)

// Determines whether a value is of type sequence.
#define IS_SEQ(value) is_obj_type(value, OBJ_SEQ)

// Determines whether a value is of type map.
#define IS_MAP(value) is_obj_type(value, OBJ_MAP)

// Determines whether a value is of type function.
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)

// Determines whether a value is of type instance.
#define IS_INSTANCE(value) is_obj_type(value, OBJ_INSTANCE)

// Determines whether a value is of type native function.
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)

// Determines whether a value is of type string.
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

// Converts a value into a bound method.
// Value must be of type bound method.
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))

// Converts a value into a class.
// Value must be of type class.
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))

// Converts a value into a closure.
// Value must be of type closure.
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))

// Converts a value into a sequence.
// Value must be of type sequence.
#define AS_SEQ(value) ((ObjSeq*)AS_OBJ(value))

// Converts a value into a map.
// Value must be of type map.
#define AS_MAP(value) ((ObjMap*)AS_OBJ(value))

// Converts a value into a function.
// Value must be of type function.
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))

// Converts a value into an instance.
// Value must be of type instance.
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))

// Converts a value into a native function.
// Value must be of type native function.
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)

// Converts a value into a string.
// Value must be of type string.
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))

// Converts a value into a C string.
// Value must be of type string.
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

// The type of an object.
typedef enum {
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_NATIVE,
  OBJ_SEQ,
  OBJ_MAP,
  OBJ_STRING,
  OBJ_UPVALUE,
  OBJ_BOUND_METHOD,
} ObjType;

// The base object construct.
struct Obj {
  ObjType type;
  bool is_marked;
  struct Obj* next;
};

struct ObjSeq {
  Obj obj;
  ValueArray items;
};

struct ObjMap {
  Obj obj;
  HashTable entries;
};

struct ObjInstance;

typedef struct {
  Obj obj;
  int arity;
  int upvalue_count;
  Chunk chunk;
  ObjString* name;
  ObjString* doc;
  struct ObjInstance* globals_context;
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
  ObjString* doc;
} ObjNative;

struct ObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
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

typedef struct ObjClass {
  Obj obj;
  ObjString* name;
  HashTable methods;
  struct ObjClass* base;
} ObjClass;

typedef struct ObjInstance {
  Obj obj;
  ObjClass* klass;
  HashTable fields;
} ObjInstance;

typedef struct {
  Obj obj;
  Value receiver;
  Obj* method;
} ObjBoundMethod;

// Creates, initializes and allocates a new bound method object. Might trigger
// garbage collection.
ObjBoundMethod* new_bound_method(Value receiver, Obj* method);

// Creates, initializes and allocates a new instance object. Might trigger
// garbage collection.
ObjInstance* new_instance(ObjClass* klass);

// Creates, initializes and allocates a new class object. Might trigger garbage
// collection.
ObjClass* new_class(ObjString* name, ObjClass* base);

// Creates, initializes and allocates a new closure object. Might trigger
// garbage collection.
ObjClosure* new_closure(ObjFunction* function);

// Creates, initializes and allocates a new function object. Might trigger
// garbage collection.
ObjFunction* new_function();

// Creates, initializes and allocates a new native function object. Might
// trigger garbage collection.
ObjNative* new_native(NativeFn function, ObjString* doc);

// Creates, initializes and allocates a new upvalue object. Might trigger
// garbage collection.
ObjUpvalue* new_upvalue(Value* slot);

// Copies a string literal into a heap-allocated string object.
// This does not take ownership of the string, but rather - as the name suggests - copies it. Might
// trigger garbage collection.
ObjString* copy_string(const char* chars, int length);

// Creates a string object from a C string.
// This takes ownership of the string. This means that the string will be freed
// when the object is freed. Might trigger garbage collection.
ObjString* take_string(char* chars, int length);

// Creates a new seq object from a value array.
// This takes ownership of the value array. This means that the value array will
// be freed when the object is freed. Might trigger garbage collection.
ObjSeq* take_seq(ValueArray* items);

// Creates a new map object from a hashtable.
// This takes ownership of the hashtable. This means that the hashtable will be
// freed when the object is freed. Might trigger garbage collection.
ObjMap* take_map(HashTable* entries);

static inline bool is_obj_type(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
