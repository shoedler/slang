#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

#if defined(DEBUG_LOG_GC) || defined(DEBUG_LOG_GC_ALLOCATIONS) || defined(DEBUG_TRACE_EXECUTION)
#include "debug.h"
#endif

Vm vm;

static Value native_clock(int arg_count, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void reset_stack() {
  vm.pause_gc = 0;
  vm.stack_top = vm.stack;
  vm.frame_count = 0;
  vm.open_upvalues = NULL;
}

// Prints a runtime error message including the stacktrace
static void runtime_error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  // Stacktrace
  for (int i = vm.frame_count - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "at line %d ", function->chunk.lines[instruction]);
    if (function->name == NULL) {
      fprintf(stderr, "at the toplevel\n");
    } else {
      fprintf(stderr, "in \"%s\"\n", function->name->chars);
    }
  }

  reset_stack();
}

// This is just a placeholder to find where we actually throw rt errors.
// I want another way to do this, but I don't know how yet.
static Value exit_with_runtime_error() {
  free_vm();
  // TODO (recovery): Find a way to progpagate errors */
  exit(70);
  return NIL_VAL;
}

// This is just a plcaeholder to find where we actually throw compile errors.
// I want another way to do this, but I don't know how yet.
static void exit_with_compile_error() {
  free_vm();
  // TODO (recovery): Find a way to progpagate errors */
  exit(65);
}

// Defines a native function in the given table.
static void define_native(HashTable* table, const char* name, NativeFn function) {
  push(OBJ_VAL(copy_string(name, (int)strlen(name))));
  push(OBJ_VAL(new_native(function)));
  hashtable_set(table, vm.stack[0], vm.stack[1]);
  pop();
  pop();
}

// Defines an object in the given table.
static void define_obj(HashTable* table, const char* name, Obj* obj) {
  push(OBJ_VAL(copy_string(name, (int)strlen(name))));
  push(OBJ_VAL(obj));
  hashtable_set(table, vm.stack[0], vm.stack[1]);
  pop();
  pop();
}

// Defines a global variable in the globals table.
static void define_global(const char name[], Obj* obj) {
  define_obj(&vm.globals, name, obj);
}

// Creates a sequence of length "count" from the top "count" values on the stack.
// The resulting sequence is pushed onto the stack.
static void make_seq(int count) {
  // Since we know the count, we can preallocate the value array for the list. This avoids
  // using write_value_array within the loop, which can trigger a GC due to growing the array
  // and free items in the middle of the loop. Also, it lets us pop the list items on the
  // stack, instead of peeking and then having to pop them later (Requiring us to loop over
  // the array twice)
  ValueArray items;
  init_value_array(&items);

  items.values = GROW_ARRAY(Value, items.values, 0, count);
  items.capacity = count;
  items.count = count;

  for (int i = count - 1; i >= 0; i--) {
    items.values[i] = pop();
  }

  ObjSeq* seq = take_seq(&items);
  push(OBJ_VAL(seq));
}

static Value __builtin_to_str(int argc, Value argv[]);
static Value __builint_hash_value(int argc, Value argv[]);
static Value __builtin_get_type(int argc, Value argv[]);

void init_vm() {
  reset_stack();
  vm.objects = NULL;

  vm.bytes_allocated = 0;
  vm.next_gc = GC_DEFAULT_THRESHOLD;
  vm.gray_count = 0;
  vm.gray_capacity = 0;
  vm.gray_stack = NULL;
  vm.pause_gc = 1;  // Pause while we initialize the vm.

  init_hashtable(&vm.globals);
  init_hashtable(&vm.strings);
  init_hashtable(&vm.modules);

  // Create the reserved method names
  memset(vm.reserved_method_names, 0, sizeof(vm.reserved_method_names));
  vm.reserved_method_names[METHOD_CTOR] =
      OBJ_VAL(copy_string(KEYWORD_CONSTRUCTOR, KEYWORD_CONSTRUCTOR_LEN));
  vm.reserved_method_names[METHOD_NAME] = OBJ_VAL(copy_string(KEYWORD_NAME, KEYWORD_NAME_LEN));

  // Create the object class
  vm.object_class = new_class(copy_string("Object", 6), NULL);
  define_global("Object", (Obj*)vm.object_class);
  define_native(&vm.object_class->methods, "str", __builtin_to_str);
  define_native(&vm.object_class->methods, "hash", __builint_hash_value);
  define_native(&vm.object_class->methods, "type", __builtin_get_type);

  vm.module_class = new_class(copy_string("Module", 6), vm.object_class);
  define_global("Module", (Obj*)vm.module_class);

  // Create the builtin obj instance
  vm.builtin = new_instance(vm.object_class);
  define_global("__builtin", (Obj*)vm.builtin);
  define_native(&vm.builtin->fields, "clock", native_clock);

  vm.pause_gc = 0;
  reset_stack();
}

void free_vm() {
  free_hashtable(&vm.globals);
  free_hashtable(&vm.strings);
  memset(vm.reserved_method_names, 0, sizeof(vm.reserved_method_names));
  free_objects();
}

void push(Value value) {
  *vm.stack_top = value;
  vm.stack_top++;
}

Value pop() {
  vm.stack_top--;
  return *vm.stack_top;
}

// Look at the value without popping it
static Value peek(int distance) {
  return vm.stack_top[-1 - distance];
}

// Executes a call to a function or method by creating a new call frame and
// pushing it onto the frame stack.
// Returns true if the call succeeded, false otherwise.
static bool call(ObjClosure* closure, int arg_count) {
  if (arg_count != closure->function->arity) {
    runtime_error("Expected %d arguments but got %d.", closure->function->arity, arg_count);
    return false;
  }

  if (vm.frame_count == FRAMES_MAX) {
    runtime_error("Stack overflow.");
    return false;
  }

  CallFrame* frame = &vm.frames[vm.frame_count++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stack_top - arg_count - 1;
  return true;
}

// Invokes a native function, as if it was a class method. Meaning, it assumes that before the
// arguments on the stack is the receiver (acting as an "instance") of the method.
static bool invoke_native(NativeFn native, int arg_count) {
  Value result = native(arg_count, vm.stack_top - arg_count - 1);  // -1 for the receiver
  vm.stack_top -= arg_count + 1;
  push(result);
  return true;
}

// Calls a callable value (function, method, class, etc.)
// Returns true if the call succeeded, false otherwise.
static bool call_value(Value callee, int arg_count) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_BOUND_METHOD: {
        ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
        vm.stack_top[-arg_count - 1] =
            bound->receiver;  // Put the receiver in slot 0, so we can access "this"
        return call(bound->method, arg_count);
      }
      case OBJ_CLASS: {
        ObjClass* klass = AS_CLASS(callee);
        vm.stack_top[-arg_count - 1] = OBJ_VAL(new_instance(klass));
        Value ctor;

        if (hashtable_get(&klass->methods, vm.reserved_method_names[METHOD_CTOR], &ctor)) {
          return call(AS_CLOSURE(ctor), arg_count);
        } else if (arg_count != 0) {
          runtime_error("Expected 0 arguments but got %d.", arg_count);
          return false;
        }
        return true;
      }
      case OBJ_CLOSURE:
        return call(AS_CLOSURE(callee), arg_count);
      case OBJ_FUNCTION:
        return call(AS_FUNCTION(callee), arg_count);
      case OBJ_NATIVE: {
        NativeFn native = AS_NATIVE(callee);
        Value result = native(arg_count, vm.stack_top - arg_count);
        vm.stack_top -= arg_count + 1;
        push(result);
        return true;
      }
      default:
        break;  // Non-callable object type.
    }
  }
  runtime_error("Attempted to call non-callable value of type %s.", type_name(callee));
  return false;
}

// Invokes a method on a class by looking up the method in the class' method
// table and calling it. (Combines "get-property" and "call" opcodes)
// Scans upwards through the class hierarchy to find the method.
static bool invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) {
  while (klass != NULL) {
    Value method;
    if (hashtable_get(&klass->methods, OBJ_VAL(name), &method)) {
      switch (AS_OBJ(method)->type) {
        case OBJ_CLOSURE:
          return call(AS_CLOSURE(method), arg_count);
        case OBJ_NATIVE:
          return invoke_native(AS_NATIVE(method), arg_count);
        default: {
          runtime_error("Cannot invoke method of type '%s' on class", type_name(method));
          return false;
        }
      }
    }
    klass = klass->base;
  }
  runtime_error("Undefined property '%s' in '%s'", name->chars, klass->name->chars);
  return false;
}

// Invokes a method on the top of the stack.
static bool invoke(ObjString* name, int arg_count) {
  Value receiver = peek(arg_count);

  if (!IS_INSTANCE(receiver)) {
    // TODO (unfinished): We should check the base-class of the type of the receiver. They are
    // currently not implemented.
    return invoke_from_class(vm.object_class, name, arg_count);
  }

  ObjInstance* instance = AS_INSTANCE(receiver);

  Value value;
  // It could be a field which is a function, we need to check that first
  if (hashtable_get(&instance->fields, OBJ_VAL(name), &value)) {
    vm.stack_top[-arg_count - 1] = value;
    return call_value(value, arg_count);
  }

  return invoke_from_class(instance->klass, name, arg_count);
}

// This is an example builtin function. It takes a single argument and returns its hash value
static Value __builint_hash_value(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 argument but got %d.", argc);
    return exit_with_runtime_error();
  }

  return NUMBER_VAL(hash_value(argv[0]));
}

// Takes a value and returns its type (string)
static Value __builtin_get_type(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 argument but got %d.", argc);
    return exit_with_runtime_error();
  }

  const char* type_name_ = type_name(argv[0]);
  int length = (int)strlen(type_name_);
  ObjString* str_obj = copy_string(type_name_, strlen(type_name_));
  return OBJ_VAL(str_obj);
}

// Takes a value and returns a string representation of its value
static Value __builtin_to_str(int argc, Value argv[]) {
  if (argc != 0) {
    runtime_error("Expected 0 arguments but got %d.", argc);
    return exit_with_runtime_error();
  }

  char* str_val = value_to_str(argv[0]);
  ObjString* str_obj = take_string(str_val, strlen(str_val));
  return OBJ_VAL(str_obj);
}

// Binds a method to an instance by creating a new bound method object and
// pushing it onto the stack.
static bool bind_method(ObjClass* klass, ObjString* name) {
  Value method;
  if (!hashtable_get(&klass->methods, OBJ_VAL(name), &method)) {
    return false;
  }

  ObjBoundMethod* bound = new_bound_method(peek(0), AS_CLOSURE(method));
  pop();
  push(OBJ_VAL(bound));
  return true;
}

// Creates a new upvalue and inserts it into the linked list of open upvalues.
// Before inserting, it checks whether there is already an upvalue for the local
// variable. If there is, it returns that one instead.
static ObjUpvalue* capture_upvalue(Value* local) {
  ObjUpvalue* prev_upvalue = NULL;
  ObjUpvalue* upvalue = vm.open_upvalues;

  // Are there any open upvalues from start (actually, start = the top of the
  // list, e.g. "last" in terms of reading a file with your eyes) to local?
  while (upvalue != NULL && upvalue->location > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next;
  }

  // If there is one, return it
  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  // Otherwise, create one and insert it into the list
  ObjUpvalue* created_upvalue = new_upvalue(local);
  created_upvalue->next = upvalue;

  if (prev_upvalue == NULL) {
    vm.open_upvalues = created_upvalue;
  } else {
    prev_upvalue->next = created_upvalue;
  }

  return created_upvalue;
}

// Closes every upvalue until the given stack slot is reached.
// Closing upvalues moves them from the stack to the heap.
static void close_upvalues(Value* last) {
  while (vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
    ObjUpvalue* upvalue = vm.open_upvalues;
    upvalue->closed = *upvalue->location;  // Move the value (via location pointer) from the
                                           // stack to the heap (closed field)
    upvalue->location = &upvalue->closed;  // Point to ourselves for the value
    vm.open_upvalues = upvalue->next;
  }
}

// Adds a method to the class on top of the stack.
// The methods closure is on top of the stack, the class is one below that.
static void define_method(ObjString* name) {
  Value method = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));  // We trust the compiler that this value
                                        // is actually a class
  hashtable_set(&klass->methods, OBJ_VAL(name), method);
  pop();
}

// Determines whether a value is falsey. We consider nil and false to be falsey,
// and everything else to be truthy.
static bool is_falsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

// Concatenates two strings on the stack into a new string and pushes it onto the stack
static void concatenate() {
  ObjString* b = AS_STRING(peek(0));  // Peek, so it doesn't get freed by the GC
  ObjString* a = AS_STRING(peek(1));  // Peek, so it doesn't get freed by the GC

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = take_string(chars, length);
  pop();  // Pop, because we only peeked
  pop();  // Pop, because we only peeked
  push(OBJ_VAL(result));
}

// This function represents the main loop of the virtual machine.
// It fetches the next instruction, decodes it, and dispatches it
static Value run() {
  CallFrame* frame = &vm.frames[vm.frame_count - 1];

// Read a single piece of data from the current instruction pointer and advance
// it
#define READ_ONE() (*frame->ip++)

// Read a constant from the constant pool. This consumes one piece of data
// on the stack, which is the index of the constant to read
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_ONE()])

// Read a string from the constant pool.
#define READ_STRING() AS_STRING(READ_CONSTANT())

// Perform a binary operation on the top two values on the stack.
// This consumes two pieces of data from the stack, and pushes the result
// value_type is the type of the result value, op is the operator to use
#define BINARY_OP(value_type, op)                                                               \
  do {                                                                                          \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                                           \
      runtime_error("Operands must be numbers. Left was %s, right was %s.", type_name(peek(1)), \
                    type_name(peek(0)));                                                        \
      return exit_with_runtime_error();                                                         \
    }                                                                                           \
    double b = AS_NUMBER(pop());                                                                \
    double a = AS_NUMBER(pop());                                                                \
    push(value_type(a op b));                                                                   \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    disassemble_instruction(&frame->closure->function->chunk,
                            (int)(frame->ip - frame->closure->function->chunk.code));

    printf(ANSI_CYAN_STR(" Stack "));
    for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
      printf(ANSI_CYAN_STR("["));
      char* str = value_to_str(*slot);
      printf("%s", str);
      free(str);
      printf(ANSI_CYAN_STR("]"));
    }
    printf("\n");
#endif

    uint16_t instruction;
    switch (instruction = READ_ONE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_NIL:
        push(NIL_VAL);
        break;
      case OP_TRUE:
        push(BOOL_VAL(true));
        break;
      case OP_FALSE:
        push(BOOL_VAL(false));
        break;
      case OP_POP:
        pop();
        break;
      case OP_GET_LOCAL: {
        uint16_t slot = READ_ONE();
        push(frame->slots[slot]);
        break;
      }
      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!hashtable_get(&vm.globals, OBJ_VAL(name), &value)) {
          runtime_error("Undefined variable '%s'.", name->chars);
          return exit_with_runtime_error();
        }
        push(value);
        break;
      }
      case OP_GET_UPVALUE: {
        uint16_t slot = READ_ONE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }
      case OP_EXPORT:  // This is currently the same as OP_DEFINE_GLOBAL, but it will be different
                       // when we (correct) modules
      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        hashtable_set(&vm.globals, OBJ_VAL(name),
                      peek(0));  // peek, because assignment is an expression!
        pop();
        break;
      }
      case OP_SET_LOCAL: {
        uint16_t slot = READ_ONE();
        frame->slots[slot] = peek(0);  // peek, because assignment is an expression!
        break;
      }
      case OP_SET_GLOBAL: {
        ObjString* name = READ_STRING();
        if (hashtable_set(&vm.globals, OBJ_VAL(name),
                          peek(0))) {  // peek, because assignment is an expression!
          hashtable_delete(&vm.globals, OBJ_VAL(name));
          runtime_error("Undefined variable '%s'.", name->chars);
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SET_UPVALUE: {
        uint16_t slot = READ_ONE();
        *frame->closure->upvalues[slot]->location =
            peek(0);  // peek, because assignment is an expression!
        break;
      }
      case OP_GET_INDEX: {
        Value index = pop();
        Value assignee = pop();

        if (!IS_NUMBER(index)) {
          push(NIL_VAL);
          break;
        }

        double i_raw = AS_NUMBER(index);
        int i;
        if (!is_int(i_raw, &i)) {
          push(NIL_VAL);
          break;
        }

        if (IS_STRING(assignee)) {
          ObjString* string = AS_STRING(assignee);
          if (i < 0 || i >= string->length) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          ObjString* char_str = copy_string(AS_CSTRING(assignee) + i, 1);
          push(OBJ_VAL(char_str));
          break;
        } else if (IS_SEQ(assignee)) {
          ObjSeq* seq = AS_SEQ(assignee);
          if (i < 0 || i >= seq->items.count) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          push(seq->items.values[i]);
        } else {
          runtime_error("%s cannot be get-indexed.", type_name(assignee));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SET_INDEX: {
        Value value = pop();  // We should peek, because assignment is an expression! But,
                              // the order is wrong so we push it back on the stack later.
                              // We are being careful not to trigger a GC in this block,
                              // because value might get collected if we do.
        Value index = pop();
        Value assignee = pop();

        if (!IS_NUMBER(index)) {
          push(NIL_VAL);
          break;
        }

        double i_raw = AS_NUMBER(index);
        int i;
        if (!is_int(i_raw, &i)) {
          push(NIL_VAL);
          break;
        }

        if (IS_SEQ(assignee)) {
          ObjSeq* seq = AS_SEQ(assignee);

          if (i < 0 || i >= seq->items.count) {
            runtime_error("Index out of bounds.");
            return exit_with_runtime_error();
          }

          seq->items.values[i] = value;
          push(value);
        } else {
          runtime_error("%s cannot be set-indexed.", type_name(assignee));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_GET_PROPERTY: {
        ObjString* name = READ_STRING();
        switch (peek(0).type) {
          case VAL_OBJ: {
            switch (OBJ_TYPE(peek(0))) {
              case OBJ_STRING: {
                if (strcmp("len", name->chars) ==
                    0) {  // TODO (robust): We should check such "syntheticFields" in
                          // OP_SET_PROPERTY for robustness. At this point, you can only set fields
                          // on instances anyway - but when we add internal base-classes for
                          // primitive types this can get ugly really fast
                  Value value = pop();  // Value to get the length of
                  int length = AS_STRING(value)->length;
                  push(NUMBER_VAL(length));
                  goto done_getting_property;
                }
                break;
              }
              case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(peek(0));
                if (values_equal(OBJ_VAL(name), vm.reserved_method_names[METHOD_NAME])) {
                  pop();  // Pop the class
                  push(OBJ_VAL(klass->name));
                  goto done_getting_property;
                } else if (values_equal(OBJ_VAL(name), vm.reserved_method_names[METHOD_CTOR])) {
                  pop();  // Pop the class, should be fine since we don't trigger the GC in this
                          // block
                  Value ctor;
                  hashtable_get(&klass->methods, OBJ_VAL(name), &ctor);
                  push(ctor);
                  goto done_getting_property;
                }
                break;
              }
              case OBJ_SEQ: {
                if (strcmp("len", name->chars) ==
                    0) {  // TODO (robust): We should check such "syntheticFields" in
                          // OP_SET_PROPERTY for robustness. At this point, you can only set fields
                          // on instances anyway - but when we add internal base-classes for
                          // primitive types this can get ugly really fast
                  Value value = pop();  // Value to get the length of
                  int length = AS_SEQ(value)->items.count;
                  push(NUMBER_VAL(length));
                  goto done_getting_property;
                }
                break;
              }
              case OBJ_INSTANCE: {
                ObjInstance* instance = AS_INSTANCE(peek(0));
                Value value;
                if (hashtable_get(&instance->fields, OBJ_VAL(name), &value)) {
                  pop();  // Instance.
                  push(value);
                  goto done_getting_property;
                } else if (bind_method(instance->klass, name)) {
                  goto done_getting_property;
                }
                break;
              }
            }
            break;
          }
        }

        // If we get here, check if it is a method on the origin object class
        Value value;
        if (hashtable_get(&vm.object_class->methods, OBJ_VAL(name), &value)) {
          pop();  // Pop the object
          push(value);
          goto done_getting_property;
        }

        runtime_error("Property '%s' does not exist on type %s.", name->chars, type_name(peek(0)));
        return exit_with_runtime_error();

      done_getting_property:
        break;
      }
      case OP_SET_PROPERTY: {
        if (!IS_INSTANCE(peek(1))) {
          runtime_error("%s cannot have fields.", type_name(peek(1)));
          return exit_with_runtime_error();
        }

        ObjInstance* instance = AS_INSTANCE(peek(1));
        ObjString* name = READ_STRING();
        hashtable_set(&instance->fields, OBJ_VAL(name), peek(0));  // Create or update
        Value value = pop();
        pop();
        push(value);
        break;
      }
      case OP_IMPORT: {
        ObjString* name = READ_STRING();
        Value module;

        if (!hashtable_get(&vm.modules, OBJ_VAL(name), &module)) {
          // Try to open the module instead
          char tmp[256];
          if (sprintf(tmp, "C:\\Projects\\slang\\modules\\%s.sl", name->chars) < 0) {
            runtime_error("Could not import module '%s.sl'. Could not format string", name->chars);
            exit_with_runtime_error();
            return NIL_VAL;
          }

          vm.exit_on_frame = vm.frame_count;
          module = run_file(tmp, 1 /* new scope */);
          vm.exit_on_frame = -1;

          if (!IS_OBJ(module)) {
            runtime_error("Could not import module '%s'. Expected object type", name->chars);
            exit_with_runtime_error();
            return NIL_VAL;
          }
          push(module);  // Show ourselves to the GC before we put it in the hashtable
          hashtable_set(&vm.modules, OBJ_VAL(name), module);
          break;
        }
        push(module);
        break;
      }
      case OP_GET_BASE_METHOD: {
        ObjString* name = READ_STRING();
        ObjClass* baseclass = AS_CLASS(pop());

        if (!bind_method(baseclass, name)) {
          runtime_error("Property '%s' does not exist in '%s'.", name->chars,
                        baseclass->name->chars);
          exit_with_runtime_error();
          return NIL_VAL;
        }
        break;
      }
      case OP_EQ: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(values_equal(a, b)));
        break;
      }
      case OP_NEQ: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(!values_equal(a, b)));
        break;
      }
      case OP_GT:
        BINARY_OP(BOOL_VAL, >);
        break;
      case OP_LT:
        BINARY_OP(BOOL_VAL, <);
        break;
      case OP_GTEQ:
        BINARY_OP(BOOL_VAL, >=);
        break;
      case OP_LTEQ:
        BINARY_OP(BOOL_VAL, <=);
        break;
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtime_error(
              "Operands must be two numbers or two strings. Left was "
              "%s, right was %s.",
              type_name(peek(1)), type_name(peek(0)));
          return exit_with_runtime_error();
        }
        break;
      }
      case OP_SUBTRACT:
        BINARY_OP(NUMBER_VAL, -);
        break;
      case OP_MULTIPLY:
        BINARY_OP(NUMBER_VAL, *);
        break;
      case OP_DIVIDE:
        BINARY_OP(NUMBER_VAL, /);
        break;
      case OP_NOT:
        push(BOOL_VAL(is_falsey(pop())));
        break;
      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          runtime_error("Operand must be a number. Was %s.", type_name(peek(0)));
          return exit_with_runtime_error();
        }
        push(NUMBER_VAL(-AS_NUMBER(pop())));
        break;
      case OP_PRINT: {
        char* str = value_to_str(peek(0));
        printf("%s\n", str);
        free(str);
        pop();
        break;
      }
      case OP_LIST_LITERAL: {
        int count = READ_ONE();
        make_seq(count);
        break;
      }
      case OP_JUMP: {
        uint16_t offset = READ_ONE();
        frame->ip += offset;
        break;
      }
      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_ONE();
        if (is_falsey(peek(0)))
          frame->ip += offset;
        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_ONE();
        frame->ip -= offset;
        break;
      }
      case OP_CALL: {
        int arg_count = READ_ONE();
        if (!call_value(peek(arg_count), arg_count)) {
          return exit_with_runtime_error();
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_INVOKE: {
        ObjString* method = READ_STRING();
        int arg_count = READ_ONE();
        if (!invoke(method, arg_count)) {
          return exit_with_runtime_error();
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_BASE_INVOKE: {
        ObjString* method = READ_STRING();
        int arg_count = READ_ONE();
        ObjClass* baseclass = AS_CLASS(pop());
        if (!invoke_from_class(baseclass, method, arg_count)) {
          return exit_with_runtime_error();
        }
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = new_closure(function);
        push(OBJ_VAL(closure));

        // Bring closure to life
        for (int i = 0; i < closure->upvalue_count; i++) {
          uint16_t is_local = READ_ONE();
          uint16_t index = READ_ONE();
          if (is_local) {
            closure->upvalues[i] = capture_upvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }
      case OP_CLOSE_UPVALUE:
        close_upvalues(vm.stack_top - 1);
        pop();
        break;
      case OP_RETURN: {
        Value result = pop();
        close_upvalues(frame->slots);
        vm.frame_count--;
        if (vm.frame_count == 0) {
          return pop();
        }

        vm.stack_top = frame->slots;
        if (vm.exit_on_frame == vm.frame_count) {
          return result;
        }
        push(result);
        frame = &vm.frames[vm.frame_count - 1];
        break;
      }
      case OP_CLASS:
        // Initially, a class always inherits from Object
        push(OBJ_VAL(new_class(READ_STRING(), vm.object_class)));
        break;
      case OP_INHERIT: {
        Value baseclass = peek(1);
        ObjClass* subclass = AS_CLASS(peek(0));
        if (!IS_CLASS(baseclass)) {
          runtime_error("Base class must be a class. Was %s.", type_name(baseclass));
          return exit_with_runtime_error();
        }
        hashtable_add_all(&AS_CLASS(baseclass)->methods, &subclass->methods);
        subclass->base = AS_CLASS(baseclass);
        pop();  // Subclass.
        break;
      }
      case OP_METHOD:
        define_method(READ_STRING());
        break;
    }
  }

#undef READ_ONE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

Value interpret(const char* source, bool local_scope) {
  ObjFunction* function = compile(source, local_scope);
  if (function == NULL) {
    exit_with_compile_error();
    return NIL_VAL;
  }

  push(OBJ_VAL(function));
  ObjClosure* closure = new_closure(function);
  pop();
  push(OBJ_VAL(closure));
  call(closure, 0);

  return run();
}

static char* read_file(const char* path) {
  if (path == NULL) {
    INTERNAL_ERROR("Cannot open NULL path \"%s\"", path);
    exit(74);
  }

  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    INTERNAL_ERROR("Could not open file \"%s\"", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    INTERNAL_ERROR("Not enough memory to read \"%s\"\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    INTERNAL_ERROR("Could not read file \"%s\"\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

Value run_file(const char* path, bool local_scope) {
#ifdef DEBUG_TRACE_EXECUTION
  printf(ANSI_MAGENTA_STR("===== "));
  printf(ANSI_CYAN_STR("Running file: %s, New local scope: %s\n"), path,
         local_scope ? "true" : "false");
#endif

  char* source = read_file(path);

  if (source == NULL) {
    free(source);
    return NIL_VAL;
  }

  Value result = interpret(source, local_scope);
  free(source);

  return result;
}
