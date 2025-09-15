#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"

int hash_function(const char *str) {
  int hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = hash * 33 + c;
  }
  return hash;
}

symbol_table *make_symbol_table(int size, symbol_table *parent) {
  symbol_table *table = malloc(sizeof(symbol_table));
  table->size = size;
  table->parent = parent;
  table->entries = malloc(sizeof(symbol_entry_list) * size);
  for (int i = 0; i < size; i++) {
    table->entries[i].head = NULL;
    table->entries[i].tail = NULL;
  }
  return table;
}

int insert_symbol_table(symbol_table *table, const char *key, cha_ast_node *node,
                        LLVMValueRef ref, LLVMTypeRef type) {
  int index = hash_function(key) % table->size;
  symbol_entry *entry = table->entries[index].head;
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return 1;
    }
    entry = entry->next;
  }

  entry = malloc(sizeof(symbol_entry));
  sprintf(entry->key, "%s", key);
  entry->next = NULL;
  entry->value = malloc(sizeof(symbol_value));
  entry->value->node = node;
  entry->value->ref = ref;
  entry->value->type = type;
  if (table->entries[index].head == NULL) {
    table->entries[index].head = entry;
    table->entries[index].tail = entry;
  } else {
    table->entries[index].tail->next = entry;
    table->entries[index].tail = entry;
  }
  return 0;
}

symbol_value *get_symbol_table(symbol_table *table, const char *key) {
  if (table == NULL) {
    return NULL;
  }
  int index = hash_function(key) % table->size;
  symbol_entry *entry = table->entries[index].head;
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry->value;
    }
    entry = entry->next;
  }
  return get_symbol_table(table->parent, key);
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

void free_all_symbol_tables(symbol_table *table) {
  if (table->parent != NULL) {
    free_all_symbol_tables(table->parent);
  }
  free_symbol_table(table);
}
