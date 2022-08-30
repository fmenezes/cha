#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"

int hash_function(char *str) {
  int hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = hash * 33 + c;
  }
  return hash;
}

symbol_table *make_symbol_table(int size) {
  symbol_table *table = malloc(sizeof(symbol_table));
  table->size = size;
  table->entries = malloc(sizeof(symbol_entry_list) * size);
  for (int i = 0; i < size; i++) {
    table->entries[i].head = NULL;
    table->entries[i].tail = NULL;
  }
  return table;
}

void insert_symbol_table(symbol_table *table, char *key, symbol_value *value) {
  int index = hash_function(key) % table->size;
  symbol_entry *entry = malloc(sizeof(symbol_entry));
  entry->key = key;
  entry->value = value;
  entry->next = NULL;
  if (table->entries[index].head == NULL) {
    table->entries[index].head = entry;
    table->entries[index].tail = entry;
  } else {
    table->entries[index].tail->next = entry;
    table->entries[index].tail = entry;
  }
}

symbol_value *get_symbol_table(symbol_table *table, char *key) {
  int index = hash_function(key) % table->size;
  symbol_entry *entry = table->entries[index].head;
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry->value;
    }
    entry = entry->next;
  }
  return NULL;
}

void free_symbol_table(symbol_table *table) {
  if (table == NULL) {
    return;
  }
  for (int i = 0; i < table->size; i++) {
    symbol_entry *entry = table->entries[i].head;
    while (entry != NULL) {
      symbol_entry *next = entry->next;
      free(entry->value);
      free(entry);
      entry = next;
    }
  }
  free(table->entries);
  free(table);
}
