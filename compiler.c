#include "compiler.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "scope.h"
#include "vm.h"

FnCompiler* current_compiler = NULL;
AstFn* compiler_root         = NULL;
bool compiler_had_error      = false;

// Forward declarations
void compile_children(FnCompiler* compiler, AstNode* node);
static void compile_node(FnCompiler* compiler, AstNode* node);
static void emit_return(FnCompiler* compiler, AstNode* source);

static void compiler_init(FnCompiler* compiler, FnCompiler* enclosing, AstFn* function, ObjObject* globals_context) {
  compiler->enclosing = enclosing;
  current_compiler    = compiler;

  compiler->result = NULL;  // Required, because new_function() can trigger a GC

  compiler->function                = function;
  compiler->result                  = new_function();
  compiler->result->upvalue_count   = function->upvalue_count;
  compiler->result->globals_context = globals_context;
  compiler->result->name            = ((AstId*)function->base.children[0])->name;

  compiler->brakes_count    = 0;
  compiler->brakes_capacity = 0;
  compiler->brake_jumps     = NULL;
}

static ObjFunction* end_compiler(FnCompiler* compiler) {
  emit_return(compiler, (AstNode*)compiler->function);
  ObjFunction* function = compiler->result;
#ifdef DEBUG_PRINT_BYTECODE
  debug_disassemble_chunk(&function->chunk, function->name->chars);
#endif

  current_compiler = compiler->enclosing;

  return function;
}

static void compiler_free(FnCompiler* compiler) {
  FREE_ARRAY(int, compiler->brake_jumps, compiler->brakes_capacity);
}

static inline bool in_global_scope(FnCompiler* compiler) {
  return compiler->enclosing == NULL;
}

// Prints an error message at the offending node.
static void compiler_error(AstNode* offending_node, const char* format, ...) {
  compiler_had_error = true;

  fprintf(stderr, "Compiler error at line %d", offending_node->token_start.line);
  fprintf(stderr, ": " ANSI_COLOR_RED);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  fprintf(stderr, ANSI_COLOR_RESET "\n");
  va_end(args);

  SourceView source = chunk_make_source_view(offending_node->token_start, offending_node->token_end);
  report_error_location(source);
}

//
// Emission
//

// Adds a value to the current constant pool and returns its index.
static uint16_t make_constant(FnCompiler* compiler, Value value, AstNode* source) {
  int constant = chunk_add_constant(&compiler->result->chunk, value);
  if (constant > MAX_CONSTANTS) {
    compiler_error(source, "Too many constants in one chunk. Max is " STR(MAX_CONSTANTS));
    return 0;
  }

  return (uint16_t)constant;
}

// Adds the ids name to the constant pool and returns its index.
static uint16_t id_constant(FnCompiler* compiler, ObjString* id, AstNode* source) {
  return make_constant(compiler, str_value(id), source);
}

// Adds the synthetic name to the constant pool and returns its index.
static uint16_t synthetic_constant(FnCompiler* compiler, const char* name, AstNode* source) {
  return make_constant(compiler, str_value(copy_string(name, strlen(name))), source);
}

// Writes an opcode or operand to the current chunk. [source] is the node that generated the instruction and is
// used for error reporting in the vm via a lightweight source view.
static void emit_one(FnCompiler* compiler, uint16_t data, AstNode* source) {
  chunk_write(&compiler->result->chunk, data, source->token_start, source->token_end);
}
// See emit_one
static void emit_two(FnCompiler* compiler, uint16_t data1, uint16_t data2, AstNode* source) {
  emit_one(compiler, data1, source);
  emit_one(compiler, data2, source);
}
// See emit_one
static void emit_three(FnCompiler* compiler, uint16_t data1, uint16_t data2, uint16_t data3, AstNode* source) {
  emit_one(compiler, data1, source);
  emit_one(compiler, data2, source);
  emit_one(compiler, data3, source);
}

static void emit_return(FnCompiler* compiler, AstNode* source) {
  if (compiler->function->type == FN_TYPE_CONSTRUCTOR) {
    emit_two(compiler, OP_GET_LOCAL, 0, source);  // Return class instance, e.g. 'this'.
  } else {
    emit_one(compiler, OP_NIL, source);
  }

  emit_one(compiler, OP_RETURN, source);
}

static void emit_constant(FnCompiler* compiler, Value value, AstNode* source) {
  emit_two(compiler, OP_CONSTANT, make_constant(compiler, value, source), source);
}

// Emits a jump instruction and returns the offset of the jump instruction. Along with the emitted jump instruction, a 16-bit
// placeholder for the operand is emitted.
static int emit_jump(FnCompiler* compiler, uint16_t instruction, AstNode* source) {
  emit_one(compiler, instruction, source);
  emit_one(compiler, UINT16_MAX, source);
  return compiler->result->chunk.count - 1;
}

// Patches a previously emitted jump instruction with the actual jump offset. The offset is calculated by subtracting the current
// chunk's count from the offset of the jump instruction.
static void patch_jump(FnCompiler* compiler, int offset) {
  // -1 to adjust for the bytecode for the jump offset itself.
  int jump = compiler->result->chunk.count - offset - 1;

  if (jump > MAX_JUMP) {
    compiler_error((AstNode*)compiler->function, "Too much code to jump over, cannot jump over %d opcodes. Max is " STR(MAX_JUMP),
                   jump);
  }

  compiler->result->chunk.code[offset] = (uint16_t)jump;
}

// Patches a previously emitted jump instruction from a break statement.
static void patch_breaks(FnCompiler* compiler, int jump_start_offset) {
  while (compiler->brakes_count > 0 && compiler->brake_jumps[compiler->brakes_count - 1] > jump_start_offset) {
    patch_jump(compiler, compiler->brake_jumps[compiler->brakes_count - 1]);
    compiler->brakes_count--;
  }
}

// Emits a loop instruction. The operand is a 16-bit offset. It is calculated by subtracting the current chunk's count from the
// offset of the jump instruction.
static void emit_loop(FnCompiler* compiler, int loop_start, AstNode* source) {
  emit_one(compiler, OP_LOOP, source);

  int offset = compiler->result->chunk.count - loop_start + 1;
  if (offset > MAX_JUMP) {
    compiler_error(source, "Loop body too large, cannot jump over %d opcodes. Max is " STR(MAX_JUMP), offset);
  }

  emit_one(compiler, (uint16_t)offset, source);
}

// Emits bytecode to assign the value at the top of the stack to the given [id].
static void emit_assign_id(FnCompiler* compiler, AstId* id) {
  Symbol* sym = id->symbol;
  switch (sym->type) {
    case SYMBOL_LOCAL: emit_two(compiler, OP_SET_LOCAL, sym->function_index, (AstNode*)id); break;
    case SYMBOL_UPVALUE:
    case SYMBOL_UPVALUE_OUTER: emit_two(compiler, OP_SET_UPVALUE, sym->function_index, (AstNode*)id); break;
    case SYMBOL_NATIVE: INTERNAL_ASSERT(false, "Fix resolver. Assigning to natives is forbidden."); break;
    case SYMBOL_GLOBAL: {
      uint16_t global_constant = id_constant(compiler, id->name, (AstNode*)id);
      emit_two(compiler, OP_SET_GLOBAL, global_constant, (AstNode*)id);
      break;
    }
    default: INTERNAL_ERROR("Unknown symbol type: %d", sym->type);
  }
}

// Emits bytecode to define the given [id] as a variable with the value at the top of the stack.
// If it's a global, [global_constant] is the index of the global constant in the constant pool.
// Note: This exists for defining globals - they are referenced by their acutal name. In contrast to emit_define_id, this exists
// because of import and destructuring declarations. In these cases we add a constant for some variable before actually defining
// any variable. We want to reuse the constant to not add unnecessary duplicate constants to the pool.
static void emit_define_id_explicit(FnCompiler* compiler, AstId* id, uint16_t global_constant) {
  Symbol* sym = id->symbol;
  switch (sym->type) {
    case SYMBOL_LOCAL: break;  // Locals are already defined.
    case SYMBOL_UPVALUE:
    case SYMBOL_UPVALUE_OUTER: INTERNAL_ASSERT(false, "Fix resolver. Cannot define upvalues."); break;
    case SYMBOL_NATIVE: INTERNAL_ASSERT(false, "Fix resolver. Cannot define natives."); break;
    case SYMBOL_GLOBAL: {
      emit_two(compiler, OP_DEFINE_GLOBAL, global_constant, (AstNode*)id);
      break;
    }
    default: INTERNAL_ERROR("Unknown symbol type: %d", sym->type);
  }
}

// Emits bytecode to define the given [id] as a variable with the value at the top of the stack.
static void emit_define_id(FnCompiler* compiler, AstId* id) {
  if (id->symbol->type == SYMBOL_GLOBAL) {
    uint16_t global_constant = id_constant(compiler, id->name, (AstNode*)id);
    emit_define_id_explicit(compiler, id, global_constant);
  } else {
    emit_define_id_explicit(compiler, id, 0 /* ignored */);
  }
}

// Emits bytecode to load the value of the given [id] onto the stack.
static void emit_load_id(FnCompiler* compiler, AstId* id) {
  Symbol* sym = id->symbol;
  switch (sym->type) {
    case SYMBOL_LOCAL: emit_two(compiler, OP_GET_LOCAL, sym->function_index, (AstNode*)id); break;
    case SYMBOL_UPVALUE:
    case SYMBOL_UPVALUE_OUTER: emit_two(compiler, OP_GET_UPVALUE, sym->function_index, (AstNode*)id); break;
    case SYMBOL_NATIVE:
    case SYMBOL_GLOBAL: {
      uint16_t global = id_constant(compiler, id->name, (AstNode*)id);
      emit_two(compiler, OP_GET_GLOBAL, global, (AstNode*)id);
      break;
    }
    default: INTERNAL_ERROR("Unknown symbol type: %d", sym->type);
  }
}

// Emits preliminary bytecode for any supported assignment target in a compound assignment case (++,--,%= etc.). These assignments
// require the target to be loaded onto the stack before the value to be assigned is computed. Used in combination with
// emit_assignment. Returns the name index if the target is a property access, 0 otherwise.
static uint16_t emit_compound_assignment_prelude(FnCompiler* compiler, AstExpression* target) {
  if (target->type == EXPR_VARIABLE) {
    AstId* id = (AstId*)target->base.children[0];
    emit_load_id(compiler, id);  // [value]
    return 0;
  } else if (target->type == EXPR_DOT) {
    AstNode* receiver = target->base.children[0];
    AstId* property   = (AstId*)target->base.children[1];
    uint16_t name     = id_constant(compiler, property->name, (AstNode*)property);
    compile_node(compiler, receiver);                               // [target]
    emit_two(compiler, OP_DUPE, 0, (AstNode*)target);               // Duplicate the receiver: [target][target]
    emit_two(compiler, OP_GET_PROPERTY, name, (AstNode*)property);  // [target][value]
    return name;
  } else if (target->type == EXPR_SUBS) {
    AstNode* receiver = target->base.children[0];
    AstNode* index    = target->base.children[1];
    compile_node(compiler, receiver);                        // [target]
    compile_node(compiler, index);                           // [target][idx]
    emit_two(compiler, OP_DUPE, 1, (AstNode*)target);        // Duplicate the receiver: [target][idx][target]
    emit_two(compiler, OP_DUPE, 1, (AstNode*)target);        // Duplicate the index:  [target][idx][target][idx]
    emit_one(compiler, OP_GET_SUBSCRIPT, (AstNode*)target);  // [target][idx][value]
    return 0;
  } else {
    INTERNAL_ERROR("Unsupported compound assignment target: %d", target->type);
    return 0;
  }
}

// Emits final bytecode for any supported assignment target in a compound assignment case (++,--,%= etc.). Used in combination
// with emit_compound_assignment_prelude. Assigns the value at the top of the stack to [target].
static void emit_compound_assignment(FnCompiler* compiler, AstExpression* target, uint16_t name) {
  if (target->type == EXPR_VARIABLE) {
    // Expects Stack: [value]
    AstId* id = (AstId*)target->base.children[0];
    emit_assign_id(compiler, id);
  } else if (target->type == EXPR_DOT) {
    // Expects Stack: [target][value]
    AstId* property = (AstId*)target->base.children[1];
    emit_two(compiler, OP_SET_PROPERTY, name, (AstNode*)property);
  } else if (target->type == EXPR_SUBS) {
    // Expects Stack: [target][idx][value]
    emit_one(compiler, OP_SET_SUBSCRIPT, (AstNode*)target);
  } else {
    INTERNAL_ERROR("Unsupported compound assignment target: %d", target->type);
  }
}

// Emits preliminary bytecode for any supported assignment target in a normal assignment case (=). Some targets require the
// to be emitted before the value to be assigned is computed. Used in combination with emit_assignment. Returns the name index if
// the target is a property access, 0 otherwise.
static uint16_t emit_assignment_prelude(FnCompiler* compiler, AstExpression* target) {
  if (target->type == EXPR_VARIABLE) {
    return 0;
  } else if (target->type == EXPR_DOT) {
    AstNode* receiver = target->base.children[0];
    AstId* property   = (AstId*)target->base.children[1];
    uint16_t name     = id_constant(compiler, property->name, (AstNode*)property);
    compile_node(compiler, receiver);  // [target]
    return name;
  } else if (target->type == EXPR_SUBS) {
    AstNode* receiver = target->base.children[0];
    AstNode* index    = target->base.children[1];
    compile_node(compiler, receiver);  // [target]
    compile_node(compiler, index);     // [target][idx]
    return 0;
  } else {
    INTERNAL_ERROR("Unsupported assignment target: %d", target->type);
    return 0;
  }
}

// Emits final bytecode for any supported assignment target in a normal assignment case (=). Used in combination with
// emit_assignment_prelude. Assigns the value at the top of the stack to [target].
static void emit_assignment(FnCompiler* compiler, AstExpression* target, uint16_t name) {
  if (target->type == EXPR_VARIABLE) {
    // Expects Stack: [value]
    AstId* id = (AstId*)target->base.children[0];
    emit_assign_id(compiler, id);
  } else if (target->type == EXPR_DOT) {
    // Expects Stack: [target][value]
    AstId* property = (AstId*)target->base.children[1];
    emit_two(compiler, OP_SET_PROPERTY, name, (AstNode*)property);
  } else if (target->type == EXPR_SUBS) {
    // Expects Stack: [target][idx][value]
    emit_one(compiler, OP_SET_SUBSCRIPT, (AstNode*)target);
  } else {
    INTERNAL_ERROR("Unsupported assignment target: %d", target->type);
  }
}

// Emits bytecode for defining variables using destructuring. Assings the value at the top of the stack to the pattern.
static void emit_define_pattern(FnCompiler* compiler, AstPattern* pattern) {
  for (int i = 0; i < pattern->base.count; i++) {
    AstPattern* child = (AstPattern*)pattern->base.children[i];
    AstId* child_id   = (AstId*)child->base.children[0];

    emit_two(compiler, OP_DUPE, 0, (AstNode*)child);  // Duplicate the rhs value: [rhs] -> [rhs][rhs]

    Value payload = pattern->type == PAT_OBJ ? str_value(child_id->name) : int_value(i);
    if (child->type == PAT_REST) {
      emit_constant(compiler, payload, (AstNode*)child);  // [rhs][rhs][payload]
      emit_one(compiler, OP_NIL, (AstNode*)child);        // [rhs][rhs][payload][nil]
      emit_one(compiler, OP_GET_SLICE, (AstNode*)child);  // [rhs][slice]
    } else if (child->type == PAT_BINDING) {
      emit_constant(compiler, payload, (AstNode*)child);      // [rhs][rhs][payload]
      emit_one(compiler, OP_GET_SUBSCRIPT, (AstNode*)child);  // [rhs][value]
    } else {
      INTERNAL_ERROR("Unsupported pattern child-type: %d", child->type);
    }

    if (child_id->symbol->type == SYMBOL_GLOBAL) {
      emit_define_id(compiler, child_id);
    } else {
      // In local scope, we have already emitted a placeholder (OP_NIL) for the value, so we need to use assign, not define.
      emit_assign_id(compiler, child_id);
      if (!in_global_scope(compiler)) {
        emit_one(compiler, OP_POP, (AstNode*)child);  // Discard the value.
      }
    }
  }

  emit_one(compiler, OP_POP, (AstNode*)pattern);  // Discard the rhs value left on the stack.
}

// Emits preliminary bytecode for defining a pattern. Used in combination with emit_define_pattern.
static void emit_define_pattern_prelude(FnCompiler* compiler, AstPattern* pattern) {
  // Emit placeholder values when in local scope. Not needed for globals, as they are declared with OP_DEFINE_GLOBAL.
  // We only get here when the pattern is preceded by a 'let' or 'const' keyword, so we can safely assume all of the bindings are
  // locals.
  if (!in_global_scope(compiler)) {
    for (int i = 0; i < pattern->base.count; i++) {
      emit_one(compiler, OP_NIL, (AstNode*)pattern);
    }
  }
}

//
// Important stuff
//

// Discards all local variables in the scope. The ones that are captured by closures are closed over.
// Used for exiting a scope, or for loop control flow when hitting a skip or break.
static void discard_locals(FnCompiler* compiler, Scope* scope, AstNode* source) {
  int current_local = scope->local_count - 1;
  while (current_local >= 0) {  // Only scopes that have locals are handled.
#ifdef DEBUG_COMPILER
    int before = current_local;
#endif
    for (int i = 0; i < scope->capacity; i++) {
      SymbolEntry* entry = &scope->entries[i];
      if (entry->key == NULL || entry->value->type != SYMBOL_LOCAL || entry->value->index != current_local) {
        continue;
      }
      if (entry->value->is_captured) {
        emit_one(compiler, OP_CLOSE_UPVALUE, source);
      } else {
        emit_one(compiler, OP_POP, source);
      }
      current_local--;
      break;
    }
#ifdef DEBUG_COMPILER
    if (before == current_local) {
      INTERNAL_ERROR("Could not find local variable. Something's wrong with indexing locals in the scope.");
    }
#endif
  }
}

// Compiles a function.
static ObjFunction* compile_function(FnCompiler* compiler, AstFn* fn) {
  FnCompiler subcompiler;
  compiler_init(&subcompiler, compiler, fn, compiler->result->globals_context);

  // Compile the function in the subcompiler
  AstId* name            = (AstId*)fn->base.children[0];
  AstDeclaration* params = (AstDeclaration*)fn->base.children[1];
  AstNode* body          = fn->base.children[2];

  // Parameters
  if (params != NULL) {
    subcompiler.result->arity = params->base.count;
    for (int i = 0; i < params->base.count; i++) {
      AstId* id = (AstId*)params->base.children[i];
      emit_define_id(&subcompiler, id);
    }
  }

  // Body / expression
  compile_node(&subcompiler, body);
  if (fn->is_lambda) {
    INTERNAL_ASSERT(body->type == NODE_EXPR, "Lambda body should be an expression.");
    emit_one(&subcompiler, OP_RETURN, (AstNode*)fn);  // Implicit return for lambdas.
  }

  // Done. Now emit the produced function and its upvalues in the parent compiler.
  ObjFunction* function = end_compiler(&subcompiler);
  emit_two(compiler, OP_CLOSURE, make_constant(compiler, fn_value((Obj*)function), (AstNode*)fn), (AstNode*)fn);
  for (int i = 0; i < function->upvalue_count; i++) {
    emit_one(compiler, fn->upvalues[i].is_local ? 1 : 0, (AstNode*)fn);
    emit_one(compiler, fn->upvalues[i].index, (AstNode*)fn);
  }

  compiler_free(&subcompiler);

  // Class functions register themselves in the class scope. Named functions register themselves in enclosing scope.
  if (fn->type == FN_TYPE_CONSTRUCTOR) {
    uint16_t ctor = synthetic_constant(compiler, STR(SP_METHOD_CTOR), (AstNode*)fn);
    emit_two(compiler, OP_METHOD, ctor, (AstNode*)fn);
    emit_one(compiler, (uint16_t)FN_TYPE_CONSTRUCTOR, (AstNode*)fn);
  } else if (fn->type == FN_TYPE_METHOD) {
    uint16_t method = id_constant(compiler, name->name, (AstNode*)name);
    emit_two(compiler, OP_METHOD, method, (AstNode*)fn);
    emit_one(compiler, (uint16_t)FN_TYPE_METHOD, (AstNode*)fn);
  } else if (fn->type == FN_TYPE_METHOD_STATIC) {
    uint16_t method = id_constant(compiler, name->name, (AstNode*)name);
    emit_two(compiler, OP_METHOD, method, (AstNode*)fn);
    emit_one(compiler, (uint16_t)FN_TYPE_METHOD_STATIC, (AstNode*)fn);
  } else if (fn->type == FN_TYPE_NAMED_FUNCTION) {
    emit_define_id(compiler, name);
  } else {
    INTERNAL_ASSERT(fn->type == FN_TYPE_ANONYMOUS_FUNCTION || fn->type == FN_TYPE_MODULE, "Unknown function type: %d", fn->type);
  }

  return function;
}

//
// Declarations
//

static void compile_declare_function(FnCompiler* compiler, AstDeclaration* decl) {
  AstFn* fn = (AstFn*)decl->base.children[0];
  compile_function(compiler, fn);
}

static void compile_declare_class(FnCompiler* compiler, AstDeclaration* decl) {
  AstId* class          = (AstId*)decl->base.children[0];
  AstId* baseclass_name = (AstId*)decl->base.children[1];

  // Declare and define the class name
  uint16_t class_name = id_constant(compiler, class->name, (AstNode*)class);
  emit_two(compiler, OP_CLASS, class_name, (AstNode*)decl);
  emit_define_id_explicit(compiler, class, class_name);

  if (baseclass_name != NULL) {
    emit_load_id(compiler, baseclass_name);
    emit_load_id(compiler, class);
    emit_one(compiler, OP_INHERIT, (AstNode*)decl);
  }

  emit_load_id(compiler, class);
  // Body
  for (int i = 2; i < decl->base.count; i++) {
    AstNode* member = decl->base.children[i];
    compile_node(compiler, member);
  }

  emit_one(compiler, OP_FINALIZE, (AstNode*)decl);
}

static void compile_declare_variable(FnCompiler* compiler, AstDeclaration* decl) {
  AstNode* id_or_pattern = decl->base.children[0];
  AstNode* initializer   = decl->base.children[1];

  if (id_or_pattern->type == NODE_PATTERN) {
    emit_define_pattern_prelude(compiler, (AstPattern*)id_or_pattern);
  }

  if (initializer != NULL) {
    compile_node(compiler, initializer);
  } else {
    emit_one(compiler, OP_NIL, (AstNode*)decl);
  }

  if (id_or_pattern->type == NODE_PATTERN) {
    emit_define_pattern(compiler, (AstPattern*)id_or_pattern);
  } else if (id_or_pattern->type == NODE_ID) {
    emit_define_id(compiler, (AstId*)id_or_pattern);
  } else {
    INTERNAL_ERROR("Unknown declaration target: %d", id_or_pattern->type);
  }
}

//
// Statements
//

static void compile_statement_import(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* name_or_pattern = stmt->base.children[0];

  if (name_or_pattern->type == NODE_ID) {
    AstId* module_name = (AstId*)name_or_pattern;
    uint16_t name      = id_constant(compiler, module_name->name, (AstNode*)module_name);

    if (stmt->path != NULL) {
      uint16_t path = make_constant(compiler, str_value(stmt->path), (AstNode*)stmt);
      emit_three(compiler, OP_IMPORT_FROM, name, path, (AstNode*)stmt);
    } else {
      emit_two(compiler, OP_IMPORT, name, (AstNode*)module_name);
    }

    emit_define_id_explicit(compiler, module_name, name);
  } else if (name_or_pattern->type == NODE_PATTERN) {
    AstPattern* pattern = (AstPattern*)name_or_pattern;

    // Since we don't have a module name, we calculate the absolute path of the module here and use it as the module name.
    // TODO: When loading a module from cache, we should compare their absolute paths instead of the module name to avoid
    // loading the same module multiple times.
    Value cwd;
    if (!hashtable_get_by_string(&vm.module->fields, vm.special_prop_names[SPECIAL_PROP_FILE_PATH], &cwd)) {
      INTERNAL_ERROR("Module file path not found in the fields of the active module (module." STR(SP_PROP_FILE_PATH) ").");
      cwd = str_value(copy_string("?", 1));
    }

    char* absolute_path = vm_resolve_module_path((ObjString*)cwd.as.obj, NULL, stmt->path);
    uint16_t absolute_file_path_constant =
        make_constant(compiler, str_value(copy_string(absolute_path, strlen(absolute_path))), (AstNode*)stmt);
    free(absolute_path);  // since we copied to make sure it's allocated on our managed heap, we need to free it.

    emit_define_pattern_prelude(compiler, pattern);

    emit_three(compiler, OP_IMPORT_FROM, absolute_file_path_constant, absolute_file_path_constant,
               (AstNode*)stmt);  // Use the path as the module name.

    emit_define_pattern(compiler, pattern);
  } else {
    INTERNAL_ERROR("Unknown import target.");
  }
}

static void compile_statement_block(FnCompiler* compiler, AstStatement* stmt) {
  compile_children(compiler, (AstNode*)stmt);
}

static void compile_statement_if(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* condition   = stmt->base.children[0];
  AstNode* then_branch = stmt->base.children[1];
  AstNode* else_branch = stmt->base.children[2];

  compile_node(compiler, condition);
  int then_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, (AstNode*)stmt);
  emit_one(compiler, OP_POP, condition);  // Discard the condition value.

  compile_node(compiler, then_branch);
  int else_jump = emit_jump(compiler, OP_JUMP, (AstNode*)stmt);

  patch_jump(compiler, then_jump);
  emit_one(compiler, OP_POP, (AstNode*)stmt);

  if (else_branch != NULL) {
    compile_node(compiler, else_branch);
  }

  patch_jump(compiler, else_jump);
}

#define NEW_LOOP()                                                             \
  int surrounding_loop_start           = compiler->innermost_loop_start;       \
  int surrounding_loop_scope_depth     = compiler->innermost_loop_scope_depth; \
  compiler->innermost_loop_start       = compiler->result->chunk.count;        \
  compiler->innermost_loop_scope_depth = compiler->function->base.scope->depth;

#define END_LOOP()                                               \
  compiler->innermost_loop_start       = surrounding_loop_start; \
  compiler->innermost_loop_scope_depth = surrounding_loop_scope_depth;

static void compile_statement_while(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* condition = stmt->base.children[0];
  AstNode* body      = stmt->base.children[1];

  // Save the loop state for continue(skip)/break statements, which might occur in the loop body.
  NEW_LOOP();

  compile_node(compiler, condition);
  int exit_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, condition);  // Jump out of the loop if the condition is false.
  emit_one(compiler, OP_POP, condition);                             // Discard the result of the condition expression.

  compile_node(compiler, body);
  emit_loop(compiler, compiler->innermost_loop_start, (AstNode*)stmt);
  patch_jump(compiler, exit_jump);
  emit_one(compiler, OP_POP, condition);

  patch_breaks(compiler, compiler->innermost_loop_start);

  // Restore the surrounding loop state.
  END_LOOP();
}

static void compile_statement_for(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* initializer = stmt->base.children[0];
  AstNode* condition   = stmt->base.children[1];
  AstNode* increment   = stmt->base.children[2];
  AstNode* body        = stmt->base.children[3];

  // Initializer
  if (initializer != NULL) {
    compile_node(compiler, initializer);
  }

  // Save the loop state for continue(skip)/break statements, which might occur in the loop body.
  NEW_LOOP();

  // Loop condition
  int exit_jump = -1;
  if (condition != NULL) {
    compile_node(compiler, condition);
    exit_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, condition);  // Jump out of the loop if the condition is false.
    emit_one(compiler, OP_POP, condition);                         // Discard the result of the condition expression.
  }

  // Loop increment
  if (increment != NULL) {
    int body_jump       = emit_jump(compiler, OP_JUMP, increment);
    int increment_start = compiler->result->chunk.count;
    compile_node(compiler, increment);
    emit_one(compiler, OP_POP, increment);  // Discard the result of the increment expression.
    emit_loop(compiler, compiler->innermost_loop_start, (AstNode*)stmt);
    compiler->innermost_loop_start = increment_start;
    patch_jump(compiler, body_jump);
  }

  // Loop body
  compile_node(compiler, body);
  emit_loop(compiler, compiler->innermost_loop_start, (AstNode*)stmt);

  if (exit_jump != -1) {
    patch_jump(compiler, exit_jump);
    emit_one(compiler, OP_POP, condition);  // Discard the result of the condition expression.
  }

  patch_breaks(compiler, compiler->innermost_loop_start);

  // Restore the surrounding loop state.
  END_LOOP();
}

#undef NEW_LOOP
#undef END_LOOP

static void compile_statement_return(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  if (expr != NULL) {
    compile_node(compiler, expr);
    emit_one(compiler, OP_RETURN, expr);
  } else {
    emit_return(compiler, (AstNode*)stmt);
  }
}

static void compile_statement_print(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  compile_node(compiler, expr);
  emit_one(compiler, OP_PRINT, expr);
}

static void compile_statement_expr(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  compile_node(compiler, expr);
  emit_one(compiler, OP_POP, expr);
}

static void compile_statement_break(FnCompiler* compiler, AstStatement* stmt) {
  INTERNAL_ASSERT(compiler->innermost_loop_start != -1, "Should have been caught by the resolver.");

  // Grow the brakes array if necessary
  if (SHOULD_GROW(compiler->brakes_count + 1, compiler->brakes_capacity)) {
    int old_capacity          = compiler->brakes_capacity;
    compiler->brakes_capacity = GROW_CAPACITY(old_capacity);
    compiler->brake_jumps     = RESIZE_ARRAY(int, compiler->brake_jumps, compiler->brakes_count, compiler->brakes_capacity);
  }

  // Discard any locals created in the loop body, then jump to the end of the loop.
  discard_locals(compiler, stmt->loop->base.scope, (AstNode*)stmt);
  compiler->brake_jumps[compiler->brakes_count++] = emit_jump(compiler, OP_JUMP, (AstNode*)stmt);
}

static void compile_statement_skip(FnCompiler* compiler, AstStatement* stmt) {
  INTERNAL_ASSERT(compiler->innermost_loop_start != -1, "Should have been caught by the resolver.");
  discard_locals(compiler, stmt->loop->base.scope, (AstNode*)stmt);
  emit_loop(compiler, compiler->innermost_loop_start, (AstNode*)stmt);
}

static void compile_statement_throw(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* expr = stmt->base.children[0];
  compile_node(compiler, expr);
  emit_one(compiler, OP_THROW, expr);
}

static void compile_statement_try(FnCompiler* compiler, AstStatement* stmt) {
  AstNode* try_stmt   = stmt->base.children[0];
  AstNode* catch_stmt = stmt->base.children[1];

  int try_jump = emit_jump(compiler, OP_TRY, (AstNode*)stmt);
  compile_node(compiler, try_stmt);  // Try statement

  // If the try stmt was successful, skip the catch stmt.
  int success_jump = emit_jump(compiler, OP_JUMP, try_stmt);
  patch_jump(compiler, try_jump);

  if (catch_stmt != NULL) {
    compile_node(compiler, catch_stmt);  // Catch statement
  }
  patch_jump(compiler, success_jump);  // Skip the catch block if the try block was successful.
}

//
// Expressions
//

static void compile_expr_binary(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  compile_node(compiler, right);
  switch (expr->operator_.type) {
    case TOKEN_NEQ: emit_one(compiler, OP_NEQ, (AstNode*)expr); break;
    case TOKEN_EQ: emit_one(compiler, OP_EQ, (AstNode*)expr); break;
    case TOKEN_GT: emit_one(compiler, OP_GT, (AstNode*)expr); break;
    case TOKEN_GTEQ: emit_one(compiler, OP_GTEQ, (AstNode*)expr); break;
    case TOKEN_LT: emit_one(compiler, OP_LT, (AstNode*)expr); break;
    case TOKEN_LTEQ: emit_one(compiler, OP_LTEQ, (AstNode*)expr); break;
    case TOKEN_PLUS: emit_one(compiler, OP_ADD, (AstNode*)expr); break;
    case TOKEN_MINUS: emit_one(compiler, OP_SUBTRACT, (AstNode*)expr); break;
    case TOKEN_MULT: emit_one(compiler, OP_MULTIPLY, (AstNode*)expr); break;
    case TOKEN_DIV: emit_one(compiler, OP_DIVIDE, (AstNode*)expr); break;
    case TOKEN_MOD: emit_one(compiler, OP_MODULO, (AstNode*)expr); break;
    default: INTERNAL_ERROR("Unhandled binary operator type: %d", expr->operator_.type); break;
  }
}

static void compile_expr_postfix(FnCompiler* compiler, AstExpression* expr) {
  AstExpression* inner = (AstExpression*)expr->base.children[0];
  OpCode op;

  switch (expr->operator_.type) {
    case TOKEN_PLUS_PLUS: op = OP_ADD; break;
    case TOKEN_MINUS_MINUS: op = OP_SUBTRACT; break;
    default: INTERNAL_ERROR("Unhandled postfix operator type: %d", expr->operator_.type); break;
  }

  // TODO (optimize): That's a lot of bytecode for a simple operation.
  compile_node(compiler, (AstNode*)inner);  // Load itself before the operation

  uint16_t name = emit_compound_assignment_prelude(compiler, inner);
  emit_constant(compiler, int_value(1), (AstNode*)expr);  // Load the increment/decrement value
  emit_one(compiler, op, (AstNode*)inner);
  emit_compound_assignment(compiler, inner, name);  // Leaves the result on the stack

  emit_one(compiler, OP_POP, (AstNode*)expr);  // Discard the result, leaving the original value on the stack.
}

static void compile_expr_unary(FnCompiler* compiler, AstExpression* expr) {
  if (expr->operator_.type == TOKEN_NOT) {
    compile_node(compiler, expr->base.children[0]);
    emit_one(compiler, OP_NOT, (AstNode*)expr);
  } else if (expr->operator_.type == TOKEN_MINUS) {
    compile_node(compiler, expr->base.children[0]);
    emit_one(compiler, OP_NEGATE, (AstNode*)expr);
  } else {
    AstExpression* inner = (AstExpression*)expr->base.children[0];
    OpCode op;

    switch (expr->operator_.type) {
      case TOKEN_PLUS_PLUS: op = OP_ADD; break;
      case TOKEN_MINUS_MINUS: op = OP_SUBTRACT; break;
      default: INTERNAL_ERROR("Unhandled unary operator type: %d", expr->operator_.type); break;
    }

    // TODO (optimize): That's a lot of bytecode for a simple operation.
    uint16_t name = emit_compound_assignment_prelude(compiler, inner);
    emit_constant(compiler, int_value(1), (AstNode*)expr);  // Load the increment/decrement value
    emit_one(compiler, op, (AstNode*)inner);
    emit_compound_assignment(compiler, inner, name);  // Leaves the result on the stack, e.g. itself after the operation.
  }
}

static void compile_expr_grouping(FnCompiler* compiler, AstExpression* expr) {
  compile_children(compiler, (AstNode*)expr);
}

static void compile_expr_literal(FnCompiler* compiler, AstExpression* expr) {
  compile_children(compiler, (AstNode*)expr);
}

static void compile_expr_variable(FnCompiler* compiler, AstExpression* expr) {
  AstId* var = (AstId*)expr->base.children[0];
  emit_load_id(compiler, var);
}

static void compile_expr_assign(FnCompiler* compiler, AstExpression* expr) {
  AstExpression* left  = (AstExpression*)expr->base.children[0];
  AstExpression* right = (AstExpression*)expr->base.children[1];
  OpCode op;

  switch (expr->operator_.type) {
    case TOKEN_ASSIGN: {
      uint16_t name = emit_assignment_prelude(compiler, left);
      compile_node(compiler, (AstNode*)right);
      emit_assignment(compiler, left, name);
      return;
    }
    case TOKEN_PLUS_ASSIGN: op = OP_ADD; break;
    case TOKEN_MINUS_ASSIGN: op = OP_SUBTRACT; break;
    case TOKEN_MULT_ASSIGN: op = OP_MULTIPLY; break;
    case TOKEN_DIV_ASSIGN: op = OP_DIVIDE; break;
    case TOKEN_MOD_ASSIGN: op = OP_MODULO; break;
    default: INTERNAL_ERROR("Unhandled compound assignment operator type: %d", expr->operator_.type); break;
  }

  // TODO (optimize): That's a lot of bytecode for a simple operation.
  uint16_t name = emit_compound_assignment_prelude(compiler, left);
  compile_node(compiler, (AstNode*)right);
  emit_one(compiler, op, (AstNode*)expr);
  emit_compound_assignment(compiler, left, name);
}

static void compile_expr_and(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  int end_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, left);
  emit_one(compiler, OP_POP, left);  // Discard the left value.

  compile_node(compiler, right);
  patch_jump(compiler, end_jump);
}

static void compile_expr_or(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  int else_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, left);
  int end_jump  = emit_jump(compiler, OP_JUMP, left);

  patch_jump(compiler, else_jump);
  emit_one(compiler, OP_POP, left);  // Discard the left value.

  compile_node(compiler, right);
  patch_jump(compiler, end_jump);
}

static void compile_expr_is(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  compile_node(compiler, right);
  emit_one(compiler, OP_IS, (AstNode*)expr);
}

static void compile_expr_in(FnCompiler* compiler, AstExpression* expr) {
  AstNode* left  = expr->base.children[0];
  AstNode* right = expr->base.children[1];

  compile_node(compiler, left);
  compile_node(compiler, right);
  emit_one(compiler, OP_IN, (AstNode*)expr);
}

static void compile_expr_call(FnCompiler* compiler, AstExpression* expr) {
  AstExpression* target = (AstExpression*)expr->base.children[0];
  uint16_t argc         = (uint16_t)expr->base.count - 1;

  // Calling "base" is a special case
  if (target->type == EXPR_BASE) {
    AstId* this_  = (AstId*)target->base.children[0];
    AstId* base_  = (AstId*)target->base.children[1];
    uint16_t ctor = synthetic_constant(compiler, STR(SP_METHOD_CTOR), (AstNode*)target);

    emit_load_id(compiler, (AstId*)this_);  // This
    for (int i = 1; i < expr->base.count; i++) {
      compile_node(compiler, expr->base.children[i]);
    }
    emit_load_id(compiler, (AstId*)base_);  // Base
    emit_three(compiler, OP_BASE_INVOKE, ctor, argc, (AstNode*)target);
  } else {
    // Could just use compile_children here, but this is more explicit:
    compile_node(compiler, (AstNode*)target);
    for (int i = 1; i < expr->base.count; i++) {
      compile_node(compiler, expr->base.children[i]);
    }
    emit_two(compiler, OP_CALL, argc, (AstNode*)expr);
  }
}

static void compile_expr_dot(FnCompiler* compiler, AstExpression* expr) {
  // If we get here, it's always a property get access.
  AstNode* target = expr->base.children[0];
  AstId* property = (AstId*)expr->base.children[1];
  uint16_t name   = id_constant(compiler, property->name, (AstNode*)property);

  // Getting a property from "base" is a special case
  if (((AstExpression*)target)->type == EXPR_BASE) {
    AstId* this_ = (AstId*)target->children[0];
    AstId* base_ = (AstId*)target->children[1];

    emit_load_id(compiler, this_);  // This
    emit_load_id(compiler, base_);  // Base
    emit_two(compiler, OP_GET_BASE_METHOD, name, (AstNode*)expr);
  } else {
    compile_node(compiler, target);
    emit_two(compiler, OP_GET_PROPERTY, name, (AstNode*)expr);
  }
}

static void compile_expr_invoke(FnCompiler* compiler, AstExpression* expr) {
  AstNode* target = expr->base.children[0];
  AstId* property = (AstId*)expr->base.children[1];
  uint16_t name   = id_constant(compiler, property->name, (AstNode*)property);
  uint16_t argc   = (uint16_t)expr->base.count - 2;

  // Invoking a method on "base" is a special case
  if (((AstExpression*)target)->type == EXPR_BASE) {
    AstId* this_  = (AstId*)target->children[0];
    AstId* base_  = (AstId*)target->children[1];
    uint16_t ctor = synthetic_constant(compiler, STR(SP_METHOD_CTOR), (AstNode*)target);

    emit_load_id(compiler, this_);  // This
    for (int i = 2; i < expr->base.count; i++) {
      compile_node(compiler, expr->base.children[i]);
    }
    emit_load_id(compiler, base_);  // Base
    emit_three(compiler, OP_BASE_INVOKE, ctor, argc, (AstNode*)expr);
  } else {
    compile_node(compiler, target);
    for (int i = 2; i < expr->base.count; i++) {
      compile_node(compiler, expr->base.children[i]);
    }
    emit_three(compiler, OP_INVOKE, name, argc, (AstNode*)expr);
  }
}

static void compile_expr_subs(FnCompiler* compiler, AstExpression* expr) {
  AstExpression* target = (AstExpression*)expr->base.children[0];
  AstNode* index        = expr->base.children[1];

  compile_node(compiler, (AstNode*)target);
  compile_node(compiler, index);
  emit_one(compiler, OP_GET_SUBSCRIPT, (AstNode*)expr);
}

static void compile_expr_slice(FnCompiler* compiler, AstExpression* expr) {
  AstExpression* target = (AstExpression*)expr->base.children[0];
  AstNode* start        = expr->base.children[1];
  AstNode* end          = expr->base.children[2];

  compile_node(compiler, (AstNode*)target);
  if (start == NULL) {
    emit_constant(compiler, int_value(0), (AstNode*)expr);  // Default start index is 0
  } else {
    compile_node(compiler, start);
  }

  if (end == NULL) {
    emit_one(compiler, OP_NIL, (AstNode*)expr);  // Default end index is nil
  } else {
    compile_node(compiler, end);
  }

  emit_one(compiler, OP_GET_SLICE, (AstNode*)expr);
}

static void compile_expr_this(FnCompiler* compiler, AstExpression* expr) {
  AstId* this_ = (AstId*)expr->base.children[0];
  emit_load_id(compiler, this_);
}

static void compile_expr_anon_fn(FnCompiler* compiler, AstExpression* expr) {
  compile_function(compiler, (AstFn*)expr->base.children[0]);
}

static void compile_expr_ternary(FnCompiler* compiler, AstExpression* expr) {
  AstNode* condition    = expr->base.children[0];
  AstNode* true_branch  = expr->base.children[1];
  AstNode* false_branch = expr->base.children[2];

  compile_node(compiler, condition);  // Condition
  int else_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, condition);
  emit_one(compiler, OP_POP, condition);  // Discard the condition.

  compile_node(compiler, true_branch);  // True branch
  int end_jump = emit_jump(compiler, OP_JUMP, true_branch);

  patch_jump(compiler, else_jump);
  emit_one(compiler, OP_POP, true_branch);  // Discard the true branch.

  compile_node(compiler, false_branch);  // False branch
  patch_jump(compiler, end_jump);
}

static void compile_expr_try(FnCompiler* compiler, AstExpression* expr) {
  AstId* error        = (AstId*)expr->base.children[0];
  AstNode* try_expr   = expr->base.children[1];
  AstNode* catch_expr = expr->base.children[2];

  int try_jump = emit_jump(compiler, OP_TRY, (AstNode*)expr);
  compile_node(compiler, try_expr);  // Try expression
  emit_assign_id(compiler, error);

  // If the try block was successful, skip the catch block.
  int success_jump = emit_jump(compiler, OP_JUMP, try_expr);
  patch_jump(compiler, try_jump);

  if (catch_expr == NULL) {
    emit_one(compiler, OP_NIL, try_expr);  // Push nil as the "else" value.
  } else {
    compile_node(compiler, catch_expr);  // Catch expression
  }
  emit_assign_id(compiler, error);     // Assign the error variable.
  patch_jump(compiler, success_jump);  // Skip the catch block if the try block was successful.
}

//
// Literals
//

static void compile_lit_number(FnCompiler* compiler, AstLiteral* lit) {
  emit_constant(compiler, lit->value, (AstNode*)lit);
}

static void compile_lit_string(FnCompiler* compiler, AstLiteral* lit) {
  emit_constant(compiler, lit->value, (AstNode*)lit);
}

static void compile_lit_bool(FnCompiler* compiler, AstLiteral* lit) {
  emit_one(compiler, lit->value.as.boolean ? OP_TRUE : OP_FALSE, (AstNode*)lit);
}

static void compile_lit_nil(FnCompiler* compiler, AstLiteral* lit) {
  emit_one(compiler, OP_NIL, (AstNode*)lit);
}

static void compile_lit_tuple(FnCompiler* compiler, AstLiteral* lit) {
  compile_children(compiler, (AstNode*)lit);
  emit_two(compiler, OP_TUPLE_LITERAL, (uint16_t)lit->base.count, (AstNode*)lit);
}

static void compile_lit_seq(FnCompiler* compiler, AstLiteral* lit) {
  compile_children(compiler, (AstNode*)lit);
  emit_two(compiler, OP_SEQ_LITERAL, (uint16_t)lit->base.count, (AstNode*)lit);
}

static void compile_lit_obj(FnCompiler* compiler, AstLiteral* lit) {
  compile_children(compiler, (AstNode*)lit);
  emit_two(compiler, OP_OBJECT_LITERAL, (uint16_t)lit->base.count / 2, (AstNode*)lit);
}

static void compile_node(FnCompiler* compiler, AstNode* node) {
  switch (node->type) {
    case NODE_BLOCK: compile_children(compiler, node); break;
    case NODE_FN: compile_function(compiler, (AstFn*)node); break;
    case NODE_ID: INTERNAL_ERROR("Should not compile " STR(NODE_ID) " directly."); break;
    case NODE_DECL: {
      AstDeclaration* decl = (AstDeclaration*)node;
      switch (decl->type) {
        case DECL_FN: compile_declare_function(compiler, decl); break;
        case DECL_FN_PARAMS: INTERNAL_ERROR("Should not compile " STR(DECL_FN_PARAMS) " directly."); break;
        case DECL_CLASS: compile_declare_class(compiler, decl); break;
        case DECL_VARIABLE: compile_declare_variable(compiler, decl); break;
        default: INTERNAL_ERROR("Unhandled declaration type."); break;
      }
      break;
    }
    case NODE_STMT: {
      AstStatement* stmt = (AstStatement*)node;
      switch (stmt->type) {
        case STMT_IMPORT: compile_statement_import(compiler, stmt); break;
        case STMT_BLOCK: compile_statement_block(compiler, stmt); break;
        case STMT_IF: compile_statement_if(compiler, stmt); break;
        case STMT_WHILE: compile_statement_while(compiler, stmt); break;
        case STMT_FOR: compile_statement_for(compiler, stmt); break;
        case STMT_RETURN: compile_statement_return(compiler, stmt); break;
        case STMT_PRINT: compile_statement_print(compiler, stmt); break;
        case STMT_EXPR: compile_statement_expr(compiler, stmt); break;
        case STMT_BREAK: compile_statement_break(compiler, stmt); break;
        case STMT_SKIP: compile_statement_skip(compiler, stmt); break;
        case STMT_THROW: compile_statement_throw(compiler, stmt); break;
        case STMT_TRY: compile_statement_try(compiler, stmt); break;
        default: INTERNAL_ERROR("Unhandled statement type."); break;
      }
      break;
    }
    case NODE_EXPR: {
      AstExpression* expr = (AstExpression*)node;
      switch (expr->type) {
        case EXPR_BINARY: compile_expr_binary(compiler, expr); break;
        case EXPR_POSTFIX: compile_expr_postfix(compiler, expr); break;
        case EXPR_UNARY: compile_expr_unary(compiler, expr); break;
        case EXPR_GROUPING: compile_expr_grouping(compiler, expr); break;
        case EXPR_LITERAL: compile_expr_literal(compiler, expr); break;
        case EXPR_VARIABLE: compile_expr_variable(compiler, expr); break;
        case EXPR_ASSIGN: compile_expr_assign(compiler, expr); break;
        case EXPR_AND: compile_expr_and(compiler, expr); break;
        case EXPR_OR: compile_expr_or(compiler, expr); break;
        case EXPR_IS: compile_expr_is(compiler, expr); break;
        case EXPR_IN: compile_expr_in(compiler, expr); break;
        case EXPR_CALL: compile_expr_call(compiler, expr); break;
        case EXPR_DOT: compile_expr_dot(compiler, expr); break;
        case EXPR_INVOKE: compile_expr_invoke(compiler, expr); break;
        case EXPR_SUBS: compile_expr_subs(compiler, expr); break;
        case EXPR_SLICE: compile_expr_slice(compiler, expr); break;
        case EXPR_THIS: compile_expr_this(compiler, expr); break;
        case EXPR_BASE: INTERNAL_ERROR("Should not compile " STR(EXPR_BASE) " directly."); break;
        case EXPR_ANONYMOUS_FN: compile_expr_anon_fn(compiler, expr); break;
        case EXPR_TERNARY: compile_expr_ternary(compiler, expr); break;
        case EXPR_TRY: compile_expr_try(compiler, expr); break;
        default: INTERNAL_ERROR("Unhandled expression type."); break;
      }
      break;
    }
    case NODE_LIT: {
      AstLiteral* lit = (AstLiteral*)node;
      switch (lit->type) {
        case LIT_NUMBER: compile_lit_number(compiler, lit); break;
        case LIT_STRING: compile_lit_string(compiler, lit); break;
        case LIT_BOOL: compile_lit_bool(compiler, lit); break;
        case LIT_NIL: compile_lit_nil(compiler, lit); break;
        case LIT_TUPLE: compile_lit_tuple(compiler, lit); break;
        case LIT_SEQ: compile_lit_seq(compiler, lit); break;
        case LIT_OBJ: compile_lit_obj(compiler, lit); break;
        default: INTERNAL_ERROR("Unhandled literal type."); break;
      }
      break;
    }
    case NODE_PATTERN: INTERNAL_ERROR("Should not compile " STR(NODE_PATTERN) " directly."); break;
  }

  // Exit the scope, if we entered one - no need to do that for functions though, since their locals are popped when the function
  // returns.
  if (node->scope != NULL && node->type != NODE_FN) {
    discard_locals(compiler, node->scope, node);
  }

#ifdef DEBUG_COMPILER
  if (in_global_scope(compiler)) {  // We're at the top-level
    printf("\n== End of compilation ==\n\n");
  }
#endif
}

void compile_children(FnCompiler* compiler, AstNode* node) {
  for (int i = 0; i < node->count; i++) {
    AstNode* child = node->children[i];
    if (child != NULL) {
      compile_node(compiler, child);
    }
  }
}

bool compile(AstFn* ast, ObjObject* globals_context, ObjFunction** result) {
  compiler_had_error = false;
  compiler_root      = ast;

  FnCompiler compiler;
  compiler_init(&compiler, NULL, ast, globals_context);
#ifdef DEBUG_COMPILER
  printf("\n\n\n === COMPILE ===\n\n");
#endif
  *result = compile_function(&compiler, ast);

  compiler_root = NULL;
  return !compiler_had_error;
}

void compiler_mark_roots() {
  if (current_compiler == NULL) {
    return;
  }

  ast_mark((AstNode*)compiler_root);

  FnCompiler* compiler = current_compiler;
  while (compiler != NULL) {
    mark_obj((Obj*)compiler->result);
    compiler = compiler->enclosing;
  }
}