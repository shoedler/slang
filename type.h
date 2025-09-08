#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include "object.h"
#include "value.h"

// Type system for static type checking
typedef enum {
  TYPE_UNKNOWN,   // Type not yet determined
  TYPE_ERROR,     // Error in typing
  TYPE_NIL,       // Nil type
  TYPE_BOOL,      // Boolean type  
  TYPE_INT,       // Integer type
  TYPE_FLOAT,     // Float type
  TYPE_STR,       // String type
  TYPE_SEQ,       // Sequence type
  TYPE_TUPLE,     // Tuple type
  TYPE_OBJ,       // Object type
  TYPE_FN,        // Function type
  TYPE_CLASS,     // Class type
  TYPE_INSTANCE,  // Instance type
} TypeKind;

typedef struct SlangType SlangType;

// Type representation
struct SlangType {
  TypeKind kind;
  ObjClass* class_ref;  // Reference to runtime class for TYPE_CLASS/TYPE_INSTANCE
  
  // For composite types
  union {
    struct {
      SlangType* element_type;  // For TYPE_SEQ
    } seq;
    
    struct {
      SlangType** element_types;  // For TYPE_TUPLE  
      int element_count;
    } tuple;
    
    struct {
      SlangType** param_types;   // For TYPE_FN
      int param_count;
      SlangType* return_type;
    } fn;
  } data;
};

// Type creation functions
SlangType* type_create_primitive(TypeKind kind);
SlangType* type_create_seq(SlangType* element_type);
SlangType* type_create_tuple(SlangType** element_types, int count);
SlangType* type_create_fn(SlangType** param_types, int param_count, SlangType* return_type);
SlangType* type_create_class(ObjClass* class_ref);
SlangType* type_create_instance(ObjClass* class_ref);

// Type utility functions
bool type_equals(SlangType* a, SlangType* b);
bool type_is_assignable(SlangType* target, SlangType* source);
SlangType* type_infer_binary_op(SlangType* left, SlangType* right, int op);
const char* type_to_string(SlangType* type);

// Memory management
void type_free(SlangType* type);
void type_mark(SlangType* type);

#endif // TYPE_H