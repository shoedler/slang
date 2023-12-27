#ifndef table_h
#define table_h

#include "common.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

// A hash table entry.
typedef struct {
  ObjString* key;
  Value value;
} Entry;

// A hash table.
typedef struct {
  int count;
  int capacity;
  Entry* entries;
} HashTable;

// Initializes a hashtable.
void init_hashtable(HashTable* table);

// Frees a hashtable.
void free_hashtable(HashTable* table);

// Gets a value from a hashtable.
bool hashtable_get(HashTable* table, ObjString* key, Value* value);

// Adds a value to a hashtable.
bool hashtable_set(HashTable* table, ObjString* key, Value value);

// Deletes a value from a hashtable.
bool hashtable_delete(HashTable* table, ObjString* key);

// Copies all entries from one hashtable to another.
void hashtable_add_all(HashTable* from, HashTable* to);

// Finds a string in the hashtable by a c string.
// Compares strings by value
ObjString* hashtable_find_string(HashTable* table,
                                 const char* chars,
                                 int length,
                                 uint32_t hash);

// Removes all white entries from a hashtable.
// This is intended to be used for the vm's interned strings, as white entries
// are swept and would result in dangling pointers within the hashtable.
void hashtable_remove_white(HashTable* table);

// Marks a hashtable by marking all its entries.
void mark_hashtable(HashTable* table);

#endif