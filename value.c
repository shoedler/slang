#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

void init_value_array(ValueArray* array) {
  array->values   = NULL;
  array->capacity = 0;
  array->count    = 0;
}

void write_value_array(ValueArray* array, Value value) {
  if (array->capacity < array->count + 1) {
    int old_capacity = array->capacity;
    array->capacity  = GROW_CAPACITY(old_capacity);
    array->values    = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void free_value_array(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  init_value_array(array);
}

bool is_int(double number, int* integer) {
  double dint = rint(number);
  *integer    = (int)dint;
  return number == dint;
}

bool values_equal(Value a, Value b) {
  // TODO (optimize): This is hot, maybe we can do better?
  if (a.type != b.type) {
    return false;
  }

  switch (a.type) {
    case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL: return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ: {
      if (IS_STRING(a) && IS_STRING(b)) {
        return AS_STRING(a) == AS_STRING(b);
      }
      if (IS_OBJ(a) && IS_OBJ(b)) {
        return true;
      }
      return false;
    }
    case VAL_EMPTY_INTERNAL: return true;
    default: INTERNAL_ERROR("Unhandled comparison type: %d", a.type); return false;
  }
}

// Hashes a double. Borrowed from Lua.
static uint32_t hash_doube(double value) {
  union BitCast {
    double source;
    uint32_t target[2];
  };

  union BitCast cast;
  cast.source = (value) + 1.0;
  return cast.target[0] + cast.target[1];
}

uint32_t hash_value(Value value) {
  // TODO (optimize): This is hot, maybe we can do better?
  switch (value.type) {
    case VAL_BOOL: return AS_BOOL(value) ? 4 : 3;
    case VAL_NIL: return 2;
    case VAL_NUMBER: return hash_doube(AS_NUMBER(value));
    case VAL_OBJ: {
      if (IS_STRING(value)) {
        ObjString* string = AS_STRING(value);
        return string->hash;
      } else {
        return (uint32_t)(intptr_t)AS_OBJ(value);  // Hash objects by pointer for now...
      }
    }
    case VAL_EMPTY_INTERNAL: return 0;
    default: INTERNAL_ERROR("Unhandled hash type: %d", value.type); return 0;
  }
}

const char* type_name(Value value) {
  if (value.type == VAL_BOOL) {
    return TYPENAME_BOOL;
  } else if (value.type == VAL_NIL) {
    return TYPENAME_NIL;
  } else if (value.type == VAL_NUMBER) {
    return TYPENAME_NUMBER;
  } else if (value.type == VAL_OBJ) {
    switch (OBJ_TYPE(value)) {
      case OBJ_BOUND_METHOD: return "BoundMethod";
      case OBJ_CLASS: return "Class";
      case OBJ_CLOSURE: return "Closure";
      case OBJ_FUNCTION: return "Fn";
      case OBJ_INSTANCE: return "Instance";
      case OBJ_NATIVE: return "NativeFn";
      case OBJ_STRING: return TYPENAME_STRING;
      case OBJ_SEQ: return TYPENAME_SEQ;
      case OBJ_UPVALUE: return "Upvalue";
    }
  }

  return "Unknown";
}

int print_value_safe(FILE* f, Value value) {
  if (!IS_OBJ(value)) {
    switch (value.type) {
      case VAL_BOOL: return fprintf(f, AS_BOOL(value) ? "true" : "false");
      case VAL_NIL: return fprintf(f, "nil");
      case VAL_NUMBER: {
        int integer;
        if (is_int(AS_NUMBER(value), &integer)) {
          return fprintf(f, "%d", integer);
        } else {
          return fprintf(f, "%g", AS_NUMBER(value));
        }
      }
      case VAL_EMPTY_INTERNAL: return fprintf(f, "EMPTY_INTERNAL");
    }

    return fprintf(f, "<unknown value type %d>", value.type);
  }

  switch (OBJ_TYPE(value)) {
    case OBJ_STRING: return fprintf(f, "\"%s\"", AS_CSTRING(value));
    case OBJ_FUNCTION: return; fprintf(f, "<fn %s>", AS_FUNCTION(value)->name->chars);
    case OBJ_CLOSURE: return fprintf(f, "<fn %s>", AS_CLOSURE(value)->function->name->chars);
    case OBJ_CLASS: return fprintf(f, "<class %s>", AS_CLASS(value)->name->chars);
    case OBJ_INSTANCE: return fprintf(f, "<instance of %s>", AS_INSTANCE(value)->klass->name->chars);
    case OBJ_NATIVE: return fprintf(f, "<native fn>");
    case OBJ_BOUND_METHOD: {
      ObjBoundMethod* bound = AS_BOUND_METHOD(value);
      if (bound->method == NULL) {
        return fprintf(f, "<(corrupt bound method)>");
      }

      if (bound->method->type == OBJ_CLOSURE) {
        if (((ObjClosure*)bound->method)->function->name != NULL) {
          return fprintf(f, "<bound method %s>", ((ObjClosure*)bound->method)->function->name->chars);
        } else {
          return fprintf(f, "<(corrupt bound closure)>");
        }
      } else if (bound->method->type == OBJ_NATIVE) {
        return fprintf(f, "<bound native method>");
      } else {
        return fprintf(f, "<(unknown bound method )>");
      }
    }
    case OBJ_SEQ: {
      int written = fprintf(f, "[");
      for (int i = 0; i < AS_SEQ(value)->items.count; i++) {
        written += print_value_safe(f, AS_SEQ(value)->items.values[i]);
        if (i < AS_SEQ(value)->items.count - 1) {
          written += fprintf(f, ", ");
        }
      }
      written += fprintf(f, "]");
      return written;
    }
    default: return fprintf(f, "<%s at %p>", type_name(value), (void*)AS_OBJ(value));
  }
}
