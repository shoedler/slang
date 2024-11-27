#ifndef table_h
#define table_h

#include <stdbool.h>
#include <stdint.h>
#include "value.h"

#define TABLE_MAX_LOAD 0.75

// A hash table entry.
typedef struct {
  Value key;
  Value value;
} Entry;

// A hash table.
typedef struct {
  int count;
  int capacity;
  Entry* entries;
} HashTable;

// Initializes a hashtable.
void hashtable_init(HashTable* table);

// Expands and allocates a hashtables capacity / entries to be able to hold at least target_count items
// without resizing. Inferrs the capacity from the count using the default growth formula. Use this when you
// know the number of entries you will be adding to the table with hashtable_set.
void hashtable_init_of_size(HashTable* table, int target_count);

// Frees a hashtable.
void hashtable_free(HashTable* table);

// Gets a value from a hashtable.
bool hashtable_get(HashTable* table, Value key, Value* value);

// Gets a value from a hashtable by a string key. This is much faster than using a Value key.
// Whenever possible, use this function instead of hashtable_get.
bool hashtable_get_by_string(HashTable* table, ObjString* key, Value* value);

// Adds a value to a hashtable. Returns true if the value was added, false if
// it was an update.
bool hashtable_set(HashTable* table, Value key, Value value);

// Deletes a value from a hashtable.
bool hashtable_delete(HashTable* table, Value key);

// Copies all entries from one hashtable to another.
void hashtable_add_all(HashTable* from, HashTable* to);

// Finds a string in the hashtable by a c string.
// Compares strings by value
ObjString* hashtable_find_string(HashTable* table, const char* chars, int length, uint64_t hash);

// Removes all white entries from a hashtable.
// This is intended to be used for the vm's interned strings, as white entries
// are swept and would result in dangling pointers within the hashtable.
void hashtable_remove_white(HashTable* table);

#endif
