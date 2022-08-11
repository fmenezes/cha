#include <stdlib.h>
#include <string.h>

#include "nic/ast.h"
#include "parser.tab.h"

extern ni_ast_node_list *parsed_ast;
extern FILE *yyin;

ni_ast_node *make_ni_ast_node_int_const(ni_ast_location loc,
                                        char *value) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_INT_CONSTANT;
  node->location = loc;
  node->int_const.value = value;
  return node;
}

ni_ast_node *make_ni_ast_node_int_type(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_INT_TYPE;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_bin_op(ni_ast_location loc, char *op,
                                     ni_ast_node *left,
                                     ni_ast_node *right) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_BIN_OP;
  node->location = loc;
  node->bin_op.op = strdup(op);
  node->bin_op.left = left;
  node->bin_op.right = right;
  return node;
}

ni_ast_node *make_ni_ast_node_variable_declaration(ni_ast_location loc,
                                                   char *identifier,
                                                   ni_ast_node *type) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_VARIABLE_DECLARATION;
  node->location = loc;
  node->variable_declaration.identifier = strdup(identifier);
  node->variable_declaration.type = type;
  return node;
}

ni_ast_node *make_ni_ast_node_variable_assignment(ni_ast_location loc,
                                                  char *identifier,
                                                  ni_ast_node *value) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_VARIABLE_ASSIGNMENT;
  node->location = loc;
  node->variable_assignment.identifier = strdup(identifier);
  node->variable_assignment.value = value;
  return node;
}

ni_ast_node *make_ni_ast_node_variable_lookup(ni_ast_location loc,
                                              char *identifier) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_VARIABLE_LOOKUP;
  node->location = loc;
  node->variable_lookup.identifier = strdup(identifier);
  return node;
}

ni_ast_node *make_ni_ast_node_argument(ni_ast_location loc,
                                       char *identifier,
                                       ni_ast_node *type) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_ARGUMENT;
  node->location = loc;
  node->argument.identifier = strdup(identifier);
  node->argument.type = type;
  return node;
}

ni_ast_node *make_ni_ast_node_block(ni_ast_location loc,
                                    ni_ast_node_list *block) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_BLOCK;
  node->location = loc;
  node->block = block;
  return node;
}

ni_ast_node *make_ni_ast_node_function_declaration(
    ni_ast_location loc, char *identifier,
    ni_ast_node *return_type, ni_ast_node_list *argument_list,
    ni_ast_node_list *block) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_FUNCTION_DECLARATION;
  node->location = loc;
  node->function_declaration.identifier = strdup(identifier);
  node->function_declaration.return_type = return_type;
  node->function_declaration.argument_list = argument_list;
  node->function_declaration.block = block;
  return node;
}

ni_ast_node *
make_ni_ast_node_function_call(ni_ast_location loc,
                               char *identifier,
                               ni_ast_node_list *argument_list) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_FUNCTION_CALL;
  node->location = loc;
  node->function_call.identifier = strdup(identifier);
  node->function_call.argument_list = argument_list;
  return node;
}

ni_ast_node *make_ni_ast_node_function_return(ni_ast_location loc,
                                              ni_ast_node *value) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_FUNCTION_RETURN;
  node->location = loc;
  node->function_return.value = value;
  return node;
}

void free_ni_ast_node(ni_ast_node *node) {
  switch (node->type) {
  case NI_AST_TYPE_INT_CONSTANT:
    free(node->int_const.value);
    break;
  case NI_AST_TYPE_BIN_OP:
    free_ni_ast_node(node->bin_op.left);
    free_ni_ast_node(node->bin_op.right);
    free(node->bin_op.op);
    break;
  case NI_AST_TYPE_VARIABLE_DECLARATION:
    free_ni_ast_node(node->variable_declaration.type);
    free(node->variable_declaration.identifier);
    break;
  case NI_AST_TYPE_VARIABLE_ASSIGNMENT:
    free_ni_ast_node(node->variable_assignment.value);
    free(node->variable_assignment.identifier);
    break;
  case NI_AST_TYPE_VARIABLE_LOOKUP:
    free(node->variable_lookup.identifier);
    break;
  case NI_AST_TYPE_ARGUMENT:
    free_ni_ast_node(node->argument.type);
    free(node->argument.identifier);
    break;
  case NI_AST_TYPE_BLOCK:
    free_ni_ast_node_list(node->block);
    break;
  case NI_AST_TYPE_FUNCTION_DECLARATION:
    free_ni_ast_node_list(node->function_declaration.argument_list);
    free_ni_ast_node_list(node->function_declaration.block);
    free(node->function_declaration.identifier);
    break;
  case NI_AST_TYPE_FUNCTION_CALL:
    free_ni_ast_node_list(node->function_call.argument_list);
    free(node->function_call.identifier);
    break;
  case NI_AST_TYPE_FUNCTION_RETURN:
    free_ni_ast_node(node->function_return.value);
    break;
  case NI_AST_TYPE_INT_TYPE:
    /* no additional fields to clear */
    break;
  }

  free(node);
}

ni_ast_node_list_entry *make_ni_ast_node_list_entry(ni_ast_node *node) {
  ni_ast_node_list_entry *entry = malloc(sizeof(ni_ast_node_list_entry));
  entry->node = node;
  entry->next = NULL;
  return entry;
}

ni_ast_node_list *make_ni_ast_node_list(ni_ast_node *head){
  ni_ast_node_list *ret = malloc(sizeof(ni_ast_node_list));
  ret->count = 1;
  ret->head = make_ni_ast_node_list_entry(head);
  ret->tail = ret->head;
  return ret;
}

void ni_ast_node_list_append(ni_ast_node_list *list, ni_ast_node *next) {
  list->count++;
  list->tail->next = make_ni_ast_node_list_entry(next);
  list->tail = list->tail->next;
}

void free_ni_ast_node_list(ni_ast_node_list *list) {
  if (list == NULL) {
    return;
  }

  ni_ast_node_list *cur = list;
  while (cur->head != NULL) {
    ni_ast_node_list_entry *next = cur->head->next;

    free_ni_ast_node(cur->head->node);
    free(cur->head);
    cur->head = next;
  }
  free(list);
}

ni_ast_node_list *ni_ast_parse(FILE *file) {
  yyin = file;
  if (yyparse() != 0) {
    return NULL;
  }
  return parsed_ast;
}

