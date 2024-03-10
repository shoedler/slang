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

char* value_to_str(Value value) {
// TODO (optimize): Add an additional int ptr parameter to this function to allow the caller to be
// able to retrieve the length of the string.
//
// TODO (robust): Since we've added vm.pause_gc, we should use our own allocator instead of malloc
// to benefit from the linearity of the memory. We'd pause the gc, make the string and then resume
// the gc.
//
// TODO (cleanup): We use typenames in the following macros. We should define them in a single place
// and use them in type_name and value_to_str.
//
// TODO (cleanup): Remove the cases for which we have a base class. Define these cases as a separate
// function.
#define STR_TRUE "true"
#define STR_FALSE "false"
#define STR_NIL "nil"
#define STR_EMPTY_INTERNAL "EMPTY_INTERNAL"
#define STR_UNKNOWN "<Unknown>"
#define STR_OBJ_CLASS(fmt) "<Class "##fmt##">"
#define STR_OBJ_CLOSURE(fmt) "<Fn "##fmt##">"
#define STR_OBJ_FUNCTION(fmt) "<Fn "##fmt##">"
#define STR_OBJ_INSTANCE(fmt) "<Instance "##fmt##">"
#define STR_OBJ_SEQ(fmt) "<Seq ["##fmt##"]>"
#define STR_OBJ_FN_TOPLEVEL "<ToplevelFn>"
#define STR_OBJ_NATIVE "<NativeFn>"
#define STR_OBJ_UPVALUE "<Upvalue>"
#define STR_INT "%d"
#define STR_FLOAT "%f"
#define SEQ_SEPARATOR ", "
#define UNNAMED_CLASS "<UnnamedClass>"
#define UNNAMED_FN "<UnnamedFn>"

  switch (value.type) {
    case VAL_BOOL: {
      return _strdup(AS_BOOL(value) ? STR_TRUE : STR_FALSE);
    }
    case VAL_NIL: {
      return _strdup(STR_NIL);
    }
    case VAL_EMPTY_INTERNAL: {
      return _strdup(STR_EMPTY_INTERNAL);
    }
    case VAL_NUMBER: {
      double number = AS_NUMBER(value);
      char buffer[64];
      int integer;
      int len = 0;

      if (is_int(number, &integer)) {
        len = snprintf(buffer, sizeof(buffer), STR_INT, integer);
      } else {
        len = snprintf(buffer, sizeof(buffer), STR_FLOAT, number);
      }

      return _strdup(buffer);
    }
    case VAL_OBJ: {
      switch (OBJ_TYPE(value)) {
        case OBJ_CLASS: {
          ObjClass* klass = AS_CLASS(value);
          if (klass->name == NULL || klass->name->chars == NULL) {
            return _strdup(STR_OBJ_CLASS(UNNAMED_CLASS));
          }

          size_t buf_size = sizeof(STR_OBJ_CLASS("")) + klass->name->length;
          char* chars     = malloc(buf_size);
          if (chars == NULL) {
            INTERNAL_ERROR("Could not allocate memory for class string");
          }
          snprintf(chars, buf_size, STR_OBJ_CLASS("%s"), klass->name->chars);
          return chars;
        }
        case OBJ_BOUND_METHOD:
          value = OBJ_VAL(AS_BOUND_METHOD(value)->method->function);
          goto handle_as_function;
        case OBJ_CLOSURE: value = OBJ_VAL(AS_CLOSURE(value)->function); goto handle_as_function;
        case OBJ_FUNCTION: {
        handle_as_function:
          ObjFunction* function = AS_FUNCTION(value);
          if (function->name == NULL || function->name->chars == NULL) {
            return _strdup(UNNAMED_FN);
          }

          size_t buf_size = sizeof(STR_OBJ_FUNCTION("")) + function->name->length;
          char* chars     = malloc(buf_size);
          if (chars == NULL) {
            INTERNAL_ERROR("Could not allocate memory for function string");
          }
          snprintf(chars, buf_size, STR_OBJ_FUNCTION("%s"), function->name->chars);
          return chars;
        }
        case OBJ_INSTANCE: {
          ObjInstance* instance = AS_INSTANCE(value);
          if (instance->klass->name == NULL) {
            return _strdup(STR_OBJ_INSTANCE(UNNAMED_CLASS));
          }

          size_t buf_size = sizeof(STR_OBJ_INSTANCE("")) + instance->klass->name->length;
          char* chars     = malloc(buf_size);
          if (chars == NULL) {
            INTERNAL_ERROR("Could not allocate memory for instance string");
          }
          snprintf(chars, buf_size, STR_OBJ_INSTANCE("%s"), instance->klass->name->chars);
          return chars;
        }
        case OBJ_NATIVE: {
          return _strdup(STR_OBJ_NATIVE);
        }
        case OBJ_UPVALUE: {
          return _strdup(STR_OBJ_UPVALUE);
        }
        case OBJ_STRING: {
          ObjString* string = AS_STRING(value);
          char* chars       = malloc(string->length + 1);
          if (chars == NULL) {
            INTERNAL_ERROR("Could not allocate memory for string string");
          }
          strcpy(chars, string->chars);
          return chars;
        }
        case OBJ_SEQ: {
          ObjSeq* seq     = AS_SEQ(value);
          size_t buf_size = 64;  // Start with a reasonable size
          char* chars     = malloc(buf_size);
          if (chars == NULL) {
            INTERNAL_ERROR("Could not allocate memory for seq string");
          }

          strcpy(chars, "[");
          for (int i = 0; i < seq->items.count; i++) {
            char* item = value_to_str(seq->items.values[i]);

            // Expand chars to fit the separator plus the next item
            size_t new_buf_size = strlen(chars) + strlen(item) + sizeof(SEQ_SEPARATOR) +
                                  2;  // +2 so we are able to add the closing bracket if we're done
                                      // after this iteration.
            if (new_buf_size > buf_size) {
              buf_size = new_buf_size;
              chars    = realloc(chars, buf_size);
              if (chars == NULL) {
                INTERNAL_ERROR("Could not reallocate memory for value-of-seq string");
              }
            }

            strcat(chars, item);
            if (i < seq->items.count - 1) {
              strcat(chars, SEQ_SEPARATOR);
            }
          }

          strcat(chars, "]");
          return chars;
        }
      }
    }
    default: return _strdup(STR_UNKNOWN);
  }

#undef STR_TRUE
#undef STR_FALSE
#undef STR_NIL
#undef STR_EMPTY_INTERNAL
#undef STR_UNKNOWN
#undef STR_OBJ_CLASS
#undef STR_OBJ_CLOSURE
#undef STR_OBJ_FUNCTION
#undef STR_OBJ_INSTANCE
#undef STR_OBJ_SEQ
#undef STR_OBJ_NATIVE
#undef STR_OBJ_UPVALUE
#undef STR_INT
#undef STR_FLOAT
#undef SEQ_SEPARATOR
#undef UNNAMED_CLASS
#undef UNNAMED_FN
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