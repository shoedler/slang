#include "hashtable.h"
#include <memory.h>
#include "memory.h"
#include "value.h"
#include "vm.h"

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
  uint64_t index = hash_value(key) & (capacity - 1);

  // If we pass a tombstone and don't end up finding the key, its entry will be re-used for the insert.
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];
    if (IS_EMPTY_INTERNAL(entry->key)) {
      if (IS_NIL(entry->value)) {
        // Empty entry.
        return tombstone != NULL ? tombstone : entry;
      } else {
        // We found a tombstone. We need to keep looking in case the key is after it, but we'll use this entry as the insertion
        // point if the key ends up not being found.
        if (tombstone == NULL) {
          tombstone = entry;
        }
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
    entries[i].key   = empty_internal_value();
    entries[i].value = nil_value();
  }

  table->count = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (IS_EMPTY_INTERNAL(entry->key)) {
      continue;
    }

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
  if (table->count == 0) {
    return false;
  }

  Entry* entry = find_entry(table->entries, table->capacity, key);
  if (IS_EMPTY_INTERNAL(entry->key)) {
    return false;
  }

  *value = entry->value;
  return true;
}

bool hashtable_get_by_string(HashTable* table, ObjString* key, Value* value) {
  if (table->count == 0) {
    return false;
  }

  int capacity   = table->capacity;
  uint64_t index = key->obj.hash & (capacity - 1);
  for (;;) {
    Entry* entry = &table->entries[index];

    if (IS_EMPTY_INTERNAL(entry->key)) {
      return false;
    }

    if (IS_STRING(entry->key) && AS_STRING(entry->key) == key) {
      *value = entry->value;
      return true;
    }
    index = (index + 1) & (capacity - 1);
  }
}

void hashtable_preallocate(HashTable* table, int target_count) {
  // Grow the capacity using the default growth formula, starting at 0 until we have enough capacity to hold
  // count considering the max load factor. This ensures that hashtable_set will not need to resize the table
  // - at least until count is reached.
  int capacity = GROW_CAPACITY(0);
  while (target_count + 1 > capacity * TABLE_MAX_LOAD) {
    capacity = GROW_CAPACITY(capacity);
  }

  adjust_capacity(table, capacity);
}

bool hashtable_set(HashTable* table, Value key, Value value) {
  // This check/strategy needs to be in sync with init_hashtable_with_count.
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjust_capacity(table, capacity);
  }

  Entry* entry = find_entry(table->entries, table->capacity, key);

  bool is_new_key = IS_EMPTY_INTERNAL(entry->key);
  if (is_new_key && IS_NIL(entry->value)) {
    table->count++;
  }

  entry->key   = key;
  entry->value = value;
  return is_new_key;
}

bool hashtable_delete(HashTable* table, Value key) {
  if (table->count == 0) {
    return false;
  }

  // Find the entry.
  Entry* entry = find_entry(table->entries, table->capacity, key);
  if (IS_EMPTY_INTERNAL(entry->key)) {
    return false;
  }

  // Place a tombstone in the entry.
  entry->key   = empty_internal_value();
  entry->value = bool_value(true);
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

ObjString* hashtable_find_string(HashTable* table, const char* chars, int length, uint64_t hash) {
  if (table->count == 0) {
    return NULL;
  }

  uint64_t index = hash & (table->capacity - 1);
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
    if (string->length == length && string->obj.hash == hash && memcmp(string->chars, chars, length) == 0) {
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
    if (!IS_EMPTY_INTERNAL(entry->key) && !(entry->key.as.obj->is_marked)) {
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
