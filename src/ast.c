#include <stdlib.h>
#include <string.h>

#include "ni/ast.h"
#include "parser.tab.h"

extern ni_ast_node_list *parsed_ast;

ni_ast_node *make_ni_ast_node_constant_number(ni_ast_location loc,
                                              const char *value) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_CONSTANT_NUMBER;
  node->location = loc;
  node->const_value = strdup(value);
  return node;
}

ni_ast_node *make_ni_ast_node_constant_float(ni_ast_location loc,
                                             const char *value) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_CONSTANT_FLOAT;
  node->location = loc;
  node->const_value = strdup(value);
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_byte(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_BYTE;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_sbyte(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_SBYTE;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_short(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_SHORT;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_ushort(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_USHORT;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_int(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_INT;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_uint(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_UINT;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_long(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_LONG;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_ulong(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_ULONG;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_large(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_LARGE;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_ularge(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_ULARGE;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_sfloat(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_SFLOAT;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_float(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_FLOAT;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_reftype_double(ni_ast_location loc) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_REFTYPE_DOUBLE;
  node->location = loc;
  return node;
}

ni_ast_node *make_ni_ast_node_bin_op(ni_ast_location loc, ni_ast_operator op,
                                     ni_ast_node *left, ni_ast_node *right) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_BIN_OP;
  node->location = loc;
  node->bin_op.op = op;
  node->bin_op.left = left;
  node->bin_op.right = right;
  return node;
}

ni_ast_node *make_ni_ast_node_variable_declaration(ni_ast_location loc,
                                                   const char *identifier,
                                                   ni_ast_node *type) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_VARIABLE_DECLARATION;
  node->location = loc;
  node->variable_declaration.identifier = strdup(identifier);
  node->variable_declaration.type = type;
  return node;
}

ni_ast_node *make_ni_ast_node_variable_assignment(ni_ast_location loc,
                                                  const char *identifier,
                                                  ni_ast_node *value) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_VARIABLE_ASSIGNMENT;
  node->location = loc;
  node->variable_assignment.identifier = strdup(identifier);
  node->variable_assignment.value = value;
  return node;
}

ni_ast_node *make_ni_ast_node_variable_lookup(ni_ast_location loc,
                                              const char *identifier) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_VARIABLE_LOOKUP;
  node->location = loc;
  node->variable_lookup.identifier = strdup(identifier);
  return node;
}

ni_ast_node *make_ni_ast_node_argument(ni_ast_location loc,
                                       const char *identifier,
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
    ni_ast_location loc, const char *identifier, ni_ast_node *return_type,
    ni_ast_node_list *argument_list, ni_ast_node_list *block) {
  ni_ast_node *node = malloc(sizeof(ni_ast_node));
  node->type = NI_AST_TYPE_FUNCTION_DECLARATION;
  node->location = loc;
  node->function_declaration.identifier = strdup(identifier);
  node->function_declaration.return_type = return_type;
  node->function_declaration.argument_list = argument_list;
  node->function_declaration.block = block;
  return node;
}

ni_ast_node *make_ni_ast_node_function_call(ni_ast_location loc,
                                            const char *identifier,
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
  if (node == NULL) {
    return;
  }

  switch (node->type) {
  case NI_AST_TYPE_CONSTANT_NUMBER:
  case NI_AST_TYPE_CONSTANT_FLOAT:
    free(node->const_value);
    break;
  case NI_AST_TYPE_BIN_OP:
    free_ni_ast_node(node->bin_op.left);
    free_ni_ast_node(node->bin_op.right);
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
    free_ni_ast_node(node->function_declaration.return_type);
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
  case NI_AST_TYPE_REFTYPE_BYTE:
  case NI_AST_TYPE_REFTYPE_SBYTE:
  case NI_AST_TYPE_REFTYPE_SHORT:
  case NI_AST_TYPE_REFTYPE_USHORT:
  case NI_AST_TYPE_REFTYPE_INT:
  case NI_AST_TYPE_REFTYPE_UINT:
  case NI_AST_TYPE_REFTYPE_LONG:
  case NI_AST_TYPE_REFTYPE_ULONG:
  case NI_AST_TYPE_REFTYPE_LARGE:
  case NI_AST_TYPE_REFTYPE_ULARGE:
  case NI_AST_TYPE_REFTYPE_FLOAT:
  case NI_AST_TYPE_REFTYPE_SFLOAT:
  case NI_AST_TYPE_REFTYPE_DOUBLE:
    /* no additional fields to clear */
    break;
  }

  free(node->location.file);
  free(node);
}

ni_ast_node_list_entry *make_ni_ast_node_list_entry(ni_ast_node *node) {
  ni_ast_node_list_entry *entry = malloc(sizeof(ni_ast_node_list_entry));
  entry->node = node;
  entry->next = NULL;
  return entry;
}

ni_ast_node_list *make_ni_ast_node_list(ni_ast_node *head) {
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
