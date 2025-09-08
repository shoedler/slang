#include "type.h"
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

// Type creation functions

SlangType* type_create_primitive(TypeKind kind) {
  SlangType* type = malloc(sizeof(SlangType));
  type->kind = kind;
  type->class_ref = NULL;
  return type;
}

SlangType* type_create_seq(SlangType* element_type) {
  SlangType* type = malloc(sizeof(SlangType));
  type->kind = TYPE_SEQ;
  type->class_ref = NULL;
  type->data.seq.element_type = element_type;
  return type;
}

SlangType* type_create_tuple(SlangType** element_types, int count) {
  SlangType* type = malloc(sizeof(SlangType));
  type->kind = TYPE_TUPLE;
  type->class_ref = NULL;
  type->data.tuple.element_types = element_types;
  type->data.tuple.element_count = count;
  return type;
}

SlangType* type_create_fn(SlangType** param_types, int param_count, SlangType* return_type) {
  SlangType* type = malloc(sizeof(SlangType));
  type->kind = TYPE_FN;
  type->class_ref = NULL;
  type->data.fn.param_types = param_types;
  type->data.fn.param_count = param_count;
  type->data.fn.return_type = return_type;
  return type;
}

SlangType* type_create_class(ObjClass* class_ref) {
  SlangType* type = malloc(sizeof(SlangType));
  type->kind = TYPE_CLASS;
  type->class_ref = class_ref;
  return type;
}

SlangType* type_create_instance(ObjClass* class_ref) {
  SlangType* type = malloc(sizeof(SlangType));
  type->kind = TYPE_INSTANCE;
  type->class_ref = class_ref;
  return type;
}

// Type utility functions

bool type_equals(SlangType* a, SlangType* b) {
  if (a == NULL && b == NULL) return true;
  if (a == NULL || b == NULL) return false;
  if (a->kind != b->kind) return false;
  
  switch (a->kind) {
    case TYPE_CLASS:
    case TYPE_INSTANCE:
      return a->class_ref == b->class_ref;
    case TYPE_SEQ:
      return type_equals(a->data.seq.element_type, b->data.seq.element_type);
    case TYPE_TUPLE:
      if (a->data.tuple.element_count != b->data.tuple.element_count) return false;
      for (int i = 0; i < a->data.tuple.element_count; i++) {
        if (!type_equals(a->data.tuple.element_types[i], b->data.tuple.element_types[i])) {
          return false;
        }
      }
      return true;
    case TYPE_FN:
      if (a->data.fn.param_count != b->data.fn.param_count) return false;
      if (!type_equals(a->data.fn.return_type, b->data.fn.return_type)) return false;
      for (int i = 0; i < a->data.fn.param_count; i++) {
        if (!type_equals(a->data.fn.param_types[i], b->data.fn.param_types[i])) {
          return false;
        }
      }
      return true;
    default:
      return true;  // Primitive types with same kind are equal
  }
}

bool type_is_assignable(SlangType* target, SlangType* source) {
  if (type_equals(target, source)) return true;
  
  // Allow assignment from int to float
  if (target->kind == TYPE_FLOAT && source->kind == TYPE_INT) {
    return true;
  }
  
  // TODO: Add inheritance checking for classes
  
  return false;
}

const char* type_to_string(SlangType* type) {
  if (type == NULL) return "unknown";
  
  switch (type->kind) {
    case TYPE_UNKNOWN: return "unknown";
    case TYPE_ERROR: return "error";
    case TYPE_NIL: return "nil";
    case TYPE_BOOL: return "bool";
    case TYPE_INT: return "int";
    case TYPE_FLOAT: return "flt";
    case TYPE_STR: return "str";
    case TYPE_SEQ: return "seq";  // TODO: Include element type
    case TYPE_TUPLE: return "tuple";  // TODO: Include element types
    case TYPE_OBJ: return "obj";
    case TYPE_FN: return "fn";  // TODO: Include signature
    case TYPE_CLASS: return "class";
    case TYPE_INSTANCE: return "instance";
  }
  
  return "unknown";
}

// Memory management

void type_free(SlangType* type) {
  if (type == NULL) return;
  
  switch (type->kind) {
    case TYPE_SEQ:
      type_free(type->data.seq.element_type);
      break;
    case TYPE_TUPLE:
      for (int i = 0; i < type->data.tuple.element_count; i++) {
        type_free(type->data.tuple.element_types[i]);
      }
      free(type->data.tuple.element_types);
      break;
    case TYPE_FN:
      for (int i = 0; i < type->data.fn.param_count; i++) {
        type_free(type->data.fn.param_types[i]);
      }
      free(type->data.fn.param_types);
      type_free(type->data.fn.return_type);
      break;
    default:
      break;
  }
  
  free(type);
}

void type_mark(SlangType* type) {
  if (type == NULL) return;
  
  // Note: For now, we don't mark the class_ref since types are not part of the GC system
  // In a full implementation, this would need to integrate with the GC
  // TODO: Consider making types part of the GC system or handling class references differently
  
  switch (type->kind) {
    case TYPE_SEQ:
      type_mark(type->data.seq.element_type);
      break;
    case TYPE_TUPLE:
      for (int i = 0; i < type->data.tuple.element_count; i++) {
        type_mark(type->data.tuple.element_types[i]);
      }
      break;
    case TYPE_FN:
      for (int i = 0; i < type->data.fn.param_count; i++) {
        type_mark(type->data.fn.param_types[i]);
      }
      type_mark(type->data.fn.return_type);
      break;
    default:
      break;
  }
}