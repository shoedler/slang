#ifndef SCOPE_H
#define SCOPE_H

#include <stdbool.h>
#include <stdint.h>
#include "object.h"

struct AstNode;  // Forward declaration for circular dependency

typedef enum {
  SYMBOL_GLOBAL,
  SYMBOL_LOCAL,
  SYMBOL_PARAM,
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
  uint16_t index;          // Index/slot of the symbol, used for locals and upvalues
  bool is_const;           // Whether the symbol represents a constant
  bool is_upvalue;         // Whether the symbol is an upvalue
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
  struct Scope* enclosing;  // Enclosing scope
  SymbolEntry* entries;
} Scope;

// Initialize a new scope
void scope_init(Scope* scope, Scope* enclosing);

// Free resources used by scope
void scope_free(Scope* scope);

// Add or update a symbol in a scope. Returns true if the symbol was added, false if not, indicating that there is already a
// symbol with the same name. Does NOT update an existing symbol.
bool scope_add_new(Scope* scope, ObjString* key, struct AstNode* source, SymbolType type, SymbolState state, bool is_const);

// Check if a symbol is in the scope
bool scope_has(Scope* scope, ObjString* key);

// Get a symbol from the scope. Returns NULL if not found.
Symbol* scope_get(Scope* scope, ObjString* key);

// Allocates a new symbol
Symbol* allocate_symbol(struct AstNode* source, SymbolType type, SymbolState state, bool is_const);

// Delete a symbol from the scope. Returns true if found and deleted.
bool scope_delete(Scope* scope, ObjString* key);

#endif  // SCOPE_H