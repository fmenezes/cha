#ifndef __NIC_AST_H__
#define __NIC_AST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct ni_ast_location {
  char *file;
  int line_begin;
  int column_begin;
  int line_end;
  int column_end;
} ni_ast_location;

typedef enum ni_ast_type {
  NI_AST_TYPE_INT_TYPE,
  NI_AST_TYPE_INT_CONSTANT,
  NI_AST_TYPE_BIN_OP,
  NI_AST_TYPE_VARIABLE_DECLARATION,
  NI_AST_TYPE_VARIABLE_ASSIGNMENT,
  NI_AST_TYPE_VARIABLE_LOOKUP,
  NI_AST_TYPE_ARGUMENT,
  NI_AST_TYPE_BLOCK,
  NI_AST_TYPE_FUNCTION_DECLARATION,
  NI_AST_TYPE_FUNCTION_CALL,
  NI_AST_TYPE_FUNCTION_RETURN,
} ni_ast_type;

typedef enum ni_ast_operator {
  NI_AST_OPERATOR_PLUS,
  NI_AST_OPERATOR_MINUS,
  NI_AST_OPERATOR_MULTIPLY,
} ni_ast_operator;

struct ni_ast_node_list;
struct ni_ast_node;
struct ni_ast_node_list_entry;

typedef struct ni_ast_node ni_ast_node;
typedef struct ni_ast_node_list ni_ast_node_list;
typedef struct ni_ast_node_list_entry ni_ast_node_list_entry;

struct ni_ast_node {
  ni_ast_type type;
  ni_ast_location location;
  union {
    struct {
      char *value;
    } int_const;
    struct {
      ni_ast_operator op;
      ni_ast_node *left;
      ni_ast_node *right;
    } bin_op;
    struct {
      char *identifier;
      ni_ast_node *type;
    } variable_declaration;
    struct {
      char *identifier;
      ni_ast_node *value;
    } variable_assignment;
    struct {
      char *identifier;
    } variable_lookup;
    struct {
      char *identifier;
      ni_ast_node *type;
    } argument;
    ni_ast_node_list *block;
    struct {
      char *identifier;
      ni_ast_node *return_type;
      ni_ast_node_list *argument_list;
      ni_ast_node_list *block;
    } function_declaration;
    struct {
      char *identifier;
      ni_ast_node_list *argument_list;
    } function_call;
    struct {
      ni_ast_node *value;
    } function_return;
  };
};

struct ni_ast_node_list_entry {
  ni_ast_node *node;
  ni_ast_node_list_entry *next;
};

struct ni_ast_node_list {
  unsigned int count;
  ni_ast_node_list_entry *head;
  ni_ast_node_list_entry *tail;
};

ni_ast_node *make_ni_ast_node_int_const(ni_ast_location loc, const char *value);
ni_ast_node *make_ni_ast_node_int_type(ni_ast_location loc);
ni_ast_node *make_ni_ast_node_bin_op(ni_ast_location loc, ni_ast_operator op,
                                     ni_ast_node *left, ni_ast_node *right);
ni_ast_node *make_ni_ast_node_variable_declaration(ni_ast_location loc,
                                                   const char *identifier,
                                                   ni_ast_node *type);
ni_ast_node *make_ni_ast_node_variable_assignment(ni_ast_location loc,
                                                  const char *identifier,
                                                  ni_ast_node *value);
ni_ast_node *make_ni_ast_node_variable_lookup(ni_ast_location loc,
                                              const char *identifier);
ni_ast_node *make_ni_ast_node_argument(ni_ast_location loc,
                                       const char *identifier,
                                       ni_ast_node *type);
ni_ast_node *make_ni_ast_node_block(ni_ast_location loc,
                                    ni_ast_node_list *block);
ni_ast_node *make_ni_ast_node_function_declaration(
    ni_ast_location loc, const char *identifier, ni_ast_node *return_type,
    ni_ast_node_list *argument_list, ni_ast_node_list *block);
ni_ast_node *make_ni_ast_node_function_call(ni_ast_location loc,
                                            const char *identifier,
                                            ni_ast_node_list *argument_list);
ni_ast_node *make_ni_ast_node_function_return(ni_ast_location loc,
                                              ni_ast_node *value);
ni_ast_node_list *make_ni_ast_node_list(ni_ast_node *head);
void ni_ast_node_list_append(ni_ast_node_list *list, ni_ast_node *next);

void free_ni_ast_node(ni_ast_node *node);
void free_ni_ast_node_list(ni_ast_node_list *list);

int ni_ast_parse(FILE *file, ni_ast_node_list **out);

#ifdef __cplusplus
}
#endif

#endif // __NIC_AST_H__
