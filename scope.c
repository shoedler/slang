#include "scope.h"
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "memory.h"

#define TOMBSTONE (Symbol*)1

void scope_init(Scope* scope, Scope* enclosing) {
  scope->count       = 0;
  scope->capacity    = 0;
  scope->local_count = 0;
  scope->entries     = NULL;
  scope->depth       = enclosing == NULL ? 0 : enclosing->depth + 1;
  scope->enclosing   = enclosing;
}

void scope_free(Scope* scope) {
  if (scope->entries == NULL) {
    goto CLEANUP;
  }

  for (int i = 0; i < scope->capacity; i++) {
    SymbolEntry* entry = &scope->entries[i];
    if (entry->value != NULL) {
      free(entry->value);
    }
  }
  free(scope->entries);

CLEANUP:
  // Explicitely don't call scope_init, as we don't want to reset the depth and enclosing scope
  scope->count    = 0;
  scope->capacity = 0;
}

Symbol* allocate_symbol(struct AstNode* source, SymbolType type, SymbolState state, bool is_const, bool is_param) {
  Symbol* value      = malloc(sizeof(Symbol));
  value->index       = -1;
  value->source      = source;
  value->type        = type;
  value->state       = state;
  value->is_const    = is_const;
  value->is_captured = false;
  value->is_param    = is_param;
  return value;
}

// Find entry for key. Returns a pointer to the entry if found, or a pointer to an empty entry if not found.
static SymbolEntry* find_entry(SymbolEntry* entries, int capacity, ObjString* key) {
  uint32_t index         = key->obj.hash & (capacity - 1);
  SymbolEntry* tombstone = NULL;

  for (;;) {
    SymbolEntry* entry = &entries[index];
    if (entry->key == NULL) {
      if (entry->value == NULL) {
        // Empty entry
        return tombstone != NULL ? tombstone : entry;
      }

      // We have a tombstone
      if (tombstone == NULL) {
        tombstone = entry;
      }
    } else if (entry->key == key) {
      // We found the key (using pointer comparison since strings are interned)
      return entry;
    }

    // Probe linearly
    index = (index + 1) & (capacity - 1);
  }
}

// Adjust scope capacity. Returns false if allocation fails.
static void adjust_capacity(Scope* scope, int new_capacity) {
  // Allocate new array of entries
  SymbolEntry* entries = malloc(sizeof(SymbolEntry) * new_capacity);
  if (entries == NULL) {
    INTERNAL_ERROR("Could not allocate memory for symbol scope.");
    return;
  }

  // Clear all entries
  memset(entries, 0, sizeof(SymbolEntry) * new_capacity);

  // Copy over existing entries
  scope->count = 0;
  for (int i = 0; i < scope->capacity; i++) {
    SymbolEntry* entry = &scope->entries[i];
    if (entry->key == NULL) {
      continue;
    }

    // Find new location for entry
    SymbolEntry* dest = find_entry(entries, new_capacity, entry->key);
    dest->key         = entry->key;
    dest->value       = entry->value;
    scope->count++;
  }

  // Free old array and update scope
  free(scope->entries);
  scope->entries  = entries;
  scope->capacity = new_capacity;
}

bool scope_add_new(Scope* scope,
                   ObjString* key,
                   struct AstNode* source,
                   SymbolType type,
                   SymbolState state,
                   bool is_const,
                   bool is_param,
                   Symbol** symbol) {
  // Grow scope if needed
  if (scope->count + 1 > scope->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(scope->capacity);
    adjust_capacity(scope, capacity);
  }

  // Find entry
  SymbolEntry* entry = find_entry(scope->entries, scope->capacity, key);

  // Not new
  if (entry->key != NULL) {
    *symbol = entry->value;
    return false;
  }

  // Store entry
  Symbol* value = allocate_symbol(source, type, state, is_const, is_param);
  if (type == SYMBOL_LOCAL) {
    value->index = scope->local_count++;
  }

  scope->count++;  // Increment symbol count, independent of symbol type
  entry->key   = key;
  entry->value = value;

  *symbol = value;
  return true;
}

bool scope_has(Scope* scope, ObjString* key) {
  return find_entry(scope->entries, scope->capacity, key)->key != NULL;
}

Symbol* scope_get(Scope* scope, ObjString* key) {
  if (scope->count == 0) {
    return NULL;
  }

  // Find entry
  SymbolEntry* entry = find_entry(scope->entries, scope->capacity, key);
  if (entry->key == NULL) {
    return NULL;
  }

  return entry->value;
}

bool scope_delete(Scope* scope, ObjString* key) {
  if (scope->count == 0)
    return false;

  // Find entry
  SymbolEntry* entry = find_entry(scope->entries, scope->capacity, key);
  if (entry->key == NULL)
    return false;

  // Place a tombstone
  entry->key   = NULL;
  entry->value = TOMBSTONE;  // Marks as tombstone
  return true;
}
