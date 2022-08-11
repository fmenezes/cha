#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <llvm-c/Core.h>

#include "nic/ast.h"

struct symbol_entry_list;
struct symbol_entry;
struct symbol_table;
struct symbol_value;

typedef struct symbol_table symbol_table;
typedef struct symbol_entry_list symbol_entry_list;
typedef struct symbol_entry symbol_entry;
typedef struct symbol_value symbol_value;

symbol_table *make_symbol_table(int size);
void insert_symbol_table(symbol_table *table, char *key, symbol_value *value);
symbol_value *get_symbol_table(symbol_table *table, char *key);
void free_symbol_table(symbol_table *table);

struct symbol_entry_list {
  symbol_entry *head;
  symbol_entry *tail;
};

struct symbol_entry {
  char *key;
  symbol_value *value;
  symbol_entry *next;
};

struct symbol_table {
  int size;
  symbol_entry_list *entries;
};

struct symbol_value {
  ni_ast_node *node;
  LLVMValueRef ref;
  LLVMTypeRef type;
};

#endif // SYMBOL_TABLE_H_
