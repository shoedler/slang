#ifndef SCOPE_H
#define SCOPE_H

#include <stdbool.h>
#include <stdint.h>
#include "object.h"

struct AstNode;  // Forward declaration for circular dependency

typedef enum {
  SYMBOL_GLOBAL,
  SYMBOL_LOCAL,
  SYMBOL_UPVALUE,        // Whether the symbol is an upvalue (e.g. a var referencing a var in an outer scope)
  SYMBOL_UPVALUE_OUTER,  // Whether the symbol is an upvalue that is an upvalue of another upvalue
  SYMBOL_NATIVE,
} SymbolType;

typedef enum {
  SYMSTATE_DECLARED,
  SYMSTATE_INITIALIZED,
  SYMSTATE_USED,
} SymbolState;

// Symbol represents a variable or similar named entity during compilation
typedef struct {
  struct AstNode* source;  // Source node where the symbol was declared
  SymbolType type;         // Type of the symbol
  SymbolState state;       // State of the symbol
  int index;               // Internal index of the symbol this scopes locals. -1 if not applicable
  int function_index;  // Index in the functions locals, including its nested scopes. Or, index in the functions upvalue list. -1
                       // if not applicable
  bool is_const;       // Whether the symbol represents a constant
  bool is_captured;    // Whether the symbol is captured by an upvalue
  bool is_param;       // Whether the symbol is a function parameter
} Symbol;

// Entry in the scope's hashtable
typedef struct {
  ObjString* key;  // The symbol's name
  Symbol* value;   // The symbol's data
} SymbolEntry;

// The scope implements a hashtable optimized for ObjString* keys
typedef struct Scope {
  int count;                // Number of entries in use
  int capacity;             // Total size of entries array
  int depth;                // Depth of the this scope (scope depth)
  int local_count;          // Number of local variables in this scope
  struct Scope* enclosing;  // Enclosing scope
  SymbolEntry* entries;
} Scope;

// Initialize a new scope
void scope_init(Scope* scope, Scope* enclosing);

// Free resources used by scope
void scope_free(Scope* scope);

// Add a symbol in a scope. Returns true if the symbol was added, false if not, indicating that there is already a
// symbol with the same name. Does NOT update an existing symbol.
// [symbol] in an out parameter that will be set to the symbol entry in the scope.
bool scope_add_new(Scope* scope,
                   ObjString* key,
                   struct AstNode* source,
                   SymbolType type,
                   SymbolState state,
                   bool is_const,
                   bool is_param,
                   Symbol** symbol);

// Update a symbol in a scope. Returns true if the symbol was updated, false if not, indicating that there is no symbol with the
// provided [key].
bool scope_update(Scope* scope,
                  ObjString* key,
                  struct AstNode* source,
                  SymbolState state,
                  bool is_const,
                  bool is_param,
                  Symbol** symbol);

// Get a symbol from the scope. Returns NULL if not found.
Symbol* scope_get(Scope* scope, ObjString* key);

// Checks whether a symbol is an upvalue
bool symbol_is_upvalue(Symbol* symbol);

#endif  // SCOPE_H