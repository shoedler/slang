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

// Determines whether a value is of type function.
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)

// Determines whether a value is of type object.
#define IS_OBJECT(value) is_obj_type(value, OBJ_OBJECT)

// Determines whether a value is of type native function.
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)

// Determines whether a value is of type string.
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

// Determines whether a value is callable.
#define IS_CALLABLE(value) is_callable(value)

// Determines whether an object is an instance of a class.
#define OBJECT_IS_INSTANCE(object) (object->klass != vm.__builtin_Obj_class)

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

// Converts a value into a function.
// Value must be of type function.
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))

// Converts a value into an object.
// Value must be of type object.
#define AS_OBJECT(value) ((ObjObject*)AS_OBJ(value))

// Converts a value into a native function.
// Value must be of type native function.
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value)))

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
  OBJ_OBJECT,
  OBJ_NATIVE,
  OBJ_SEQ,
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

struct ObjObject;

typedef struct {
  Obj obj;
  int arity;
  int upvalue_count;
  Chunk chunk;
  ObjString* name;
  ObjString* doc;
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
  ObjString* doc;
  int arity;
} ObjNative;

struct ObjString {
  Obj obj;
  int length;
  uint32_t hash;
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

typedef struct ObjClass {
  Obj obj;
  ObjString* name;
  HashTable methods;
  struct ObjClass* base;
} ObjClass;

typedef struct ObjObject {
  Obj obj;
  ObjClass* klass;
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
// collection.
ObjClass* new_class(ObjString* name, ObjClass* base);

// Creates, initializes and allocates a new closure object. Might trigger
// garbage collection.
ObjClosure* new_closure(ObjFunction* function);

// Creates, initializes and allocates a new function object. Might trigger
// garbage collection.
ObjFunction* new_function();

// Creates, initializes and allocates a new seq object. Might trigger garbage
// collection.
ObjSeq* new_seq();

// Creates, initializes and allocates a new native function object. Might
// trigger garbage collection.
ObjNative* new_native(NativeFn function, ObjString* doc, int arity);

// Creates, initializes and allocates a new upvalue object. Might trigger
// garbage collection.
ObjUpvalue* new_upvalue(Value* slot);

// Copies a string literal into a heap-allocated string object.
// This does not take ownership of the string, but rather - as the name suggests - copies it. Might
// trigger garbage collection.
ObjString* copy_string(const char* chars, int length);

// Creates, initializes and allocates a new seq object. Initializes the
// value array to add 'count' values without resizing. It's intended to add items directly to
// items.values[idx], do not use write_value_array. Might trigger garbage collection.
ObjSeq* prealloc_seq(int count);

// Creates a string object from a C string.
// This takes ownership of the string. This means that the string will be freed
// when the object is freed. Might trigger garbage collection.
ObjString* take_string(char* chars, int length);

// Creates a new seq object from a value array.
// This takes ownership of the value array. This means that the value array will
// be freed when the object is freed. Might trigger garbage collection.
ObjSeq* take_seq(ValueArray* items);

// Creates a new object from a hashtable.
// This takes ownership of the hashtable. This means that the hashtable will be
// freed when the object is freed. Might trigger garbage collection.
ObjObject* take_object(HashTable* entries);

static inline bool is_obj_type(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Determines if a value is callable. This is used to check if a value can be called as a function.
static inline bool is_callable(Value value) {
  if (IS_OBJ(value)) {
    switch (OBJ_TYPE(value)) {
      case OBJ_CLOSURE:
      case OBJ_BOUND_METHOD:
      case OBJ_NATIVE: return true;
      default: break;
    }
  }
  return false;
}

// Determines the arity of a callable obj. This is used to check how many arguments a function expects.
static inline int get_arity(Obj* callable) {
again:
  switch (callable->type) {
    case OBJ_CLOSURE: return ((ObjClosure*)callable)->function->arity;
    case OBJ_NATIVE: return ((ObjNative*)callable)->arity;
    case OBJ_BOUND_METHOD: {
      callable = ((ObjBoundMethod*)callable)->method;
      goto again;
    };
    default: break;
  }

  INTERNAL_ERROR("Value is not callable.");
  return -999;
}

#endif
