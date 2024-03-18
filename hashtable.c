#include "hashtable.h"
#include <memory.h>
#include "memory.h"
#include "value.h"

void init_hashtable(HashTable* table) {
  table->count    = 0;
  table->capacity = 0;
  table->entries  = NULL;
}

void free_hashtable(HashTable* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  init_hashtable(table);
}

// Find the entry for key. Returns NULL if no entry is found.
static Entry* find_entry(Entry* entries, int capacity, Value key) {
  uint32_t index   = hash_value(key) & (capacity - 1);
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];
    if (IS_EMPTY_INTERNAL(entry->key)) {
      if (IS_NIL(entry->value)) {
        // Empty entry.
        return tombstone != NULL ? tombstone : entry;
      } else {
        // We found a tombstone.
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (values_equal(entry->key, key)) {
      // We found the key.
      return entry;
    }

    index = (index + 1) & (capacity - 1);
  }
}

// Adjusts the capacity of the hashtable to be at least of size capacity.
// Creates a new hashtable and copies the entries over.
static void adjust_capacity(HashTable* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key   = EMPTY_INTERNAL_VAL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (IS_EMPTY_INTERNAL(entry->key))
      continue;

    Entry* dest = find_entry(entries, capacity, entry->key);
    dest->key   = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries  = entries;
  table->capacity = capacity;
}

bool hashtable_get(HashTable* table, Value key, Value* value) {
  if (table->count == 0)
    return false;

  Entry* entry = find_entry(table->entries, table->capacity, key);
  if (IS_EMPTY_INTERNAL(entry->key))
    return false;

  *value = entry->value;
  return true;
}

bool hashtable_set(HashTable* table, Value key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjust_capacity(table, capacity);
  }

  Entry* entry    = find_entry(table->entries, table->capacity, key);
  bool is_new_key = IS_EMPTY_INTERNAL(entry->key);
  if (is_new_key && IS_NIL(entry->value))
    table->count++;

  entry->key   = key;
  entry->value = value;
  return is_new_key;
}

bool hashtable_delete(HashTable* table, Value key) {
  if (table->count == 0)
    return false;

  // Find the entry.
  Entry* entry = find_entry(table->entries, table->capacity, key);
  if (IS_EMPTY_INTERNAL(entry->key))
    return false;

  // Place a tombstone in the entry.
  entry->key   = EMPTY_INTERNAL_VAL;
  entry->value = BOOL_VAL(true);
  return true;
}

void hashtable_add_all(HashTable* from, HashTable* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key)) {
      hashtable_set(to, entry->key, entry->value);
    }
  }
}

ObjString* hashtable_find_string(HashTable* table, const char* chars, int length, uint32_t hash) {
  if (table->count == 0) {
    return NULL;
  }

  uint32_t index = hash & (table->capacity - 1);
  for (;;) {
    Entry* entry = &table->entries[index];
    if (IS_EMPTY_INTERNAL(entry->key)) {
      // Stop if we find an empty non-tombstone entry.
      if (IS_NIL(entry->value)) {
        return NULL;
      }

      // We found a tombstone. No need to do further checks and slow down the search.
      goto finish_find_string_iteration;
    }

    // Check if we found the string.
    ObjString* string = AS_STRING(entry->key);
    if (string->length == length && string->hash == hash && memcmp(string->chars, chars, length) == 0) {
      // We found it.
      return string;
    }

  finish_find_string_iteration:
    index = (index + 1) & (table->capacity - 1);
  }
}

void hashtable_remove_white(HashTable* table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (!IS_EMPTY_INTERNAL(entry->key) && !(AS_OBJ(entry->key))->is_marked) {
      hashtable_delete(table, entry->key);
    }
  }
}

void mark_hashtable(HashTable* table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    mark_value(entry->key);
    mark_value(entry->value);
  }
}
