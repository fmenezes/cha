#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.tab.h"

extern cha_ast_node_list *parsed_ast;

cha_ast_node *make_cha_ast_node_constant_integer(cha_ast_location loc,
                                                 char * value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_CONSTANT_INT;
  node->location = loc;
  node->_result_type = make_cha_ast_primitive_type(loc, CHA_AST_PRIMITIVE_TYPE_CONST_INT);
  node->const_value = value;
  return node;
}

cha_ast_node *make_cha_ast_node_constant_unsigned_integer(cha_ast_location loc,
                                                          char * value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_CONSTANT_UINT;
  node->location = loc;
  node->_result_type = make_cha_ast_primitive_type(loc, CHA_AST_PRIMITIVE_TYPE_CONST_UINT);
  node->const_value = value;
  return node;
}

cha_ast_node *make_cha_ast_node_constant_float(cha_ast_location loc,
                                               double value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_CONSTANT_FLOAT;
  node->location = loc;
  node->_result_type =
      make_cha_ast_primitive_type(loc, CHA_AST_PRIMITIVE_TYPE_CONST_FLOAT);
  node->const_float = value;
  return node;
}

cha_ast_node *make_cha_ast_node_constant_true(cha_ast_location loc) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_CONSTANT_BOOL;
  node->location = loc;
  node->_result_type = make_cha_ast_primitive_type(loc, CHA_AST_PRIMITIVE_TYPE_BOOL);
  node->const_bool = 1;
  return node;
}

cha_ast_node *make_cha_ast_node_constant_false(cha_ast_location loc) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_CONSTANT_BOOL;
  node->location = loc;
  node->_result_type = make_cha_ast_primitive_type(loc, CHA_AST_PRIMITIVE_TYPE_BOOL);
  node->const_bool = 0;
  return node;
}

cha_ast_type *make_cha_ast_primitive_type_int(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_INT;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_uint(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_UINT;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_uint8(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_UINT8;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_int8(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_INT8;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_int16(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_INT16;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_uint16(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_UINT16;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_int32(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_INT32;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_uint32(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_UINT32;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_int64(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_INT64;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_uint64(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_UINT64;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_float16(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_FLOAT16;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_float32(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_FLOAT32;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_float64(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_FLOAT64;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_primitive_type_bool(cha_ast_location loc) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->primitive_type = CHA_AST_PRIMITIVE_TYPE_BOOL;
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->location = loc;
  return t;
}

cha_ast_type *make_cha_ast_array_type(cha_ast_location loc, cha_ast_type* element_type, int size) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->type_definition = CHA_AST_TYPE_DEFINITION_ARRAY;
  t->array_type.element_type = element_type;
  t->array_type.size = size;
  t->location = loc;
  return t;
}

cha_ast_node *make_cha_ast_node_bin_op(cha_ast_location loc,
                                       cha_ast_operator op, cha_ast_node *left,
                                       cha_ast_node *right) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_BIN_OP;
  node->location = loc;
  node->_result_type = NULL;
  node->bin_op.op = op;
  node->bin_op.left = left;
  node->bin_op.right = right;
  return node;
}

cha_ast_node *make_cha_ast_node_variable_declaration(cha_ast_location loc,
                                                     char *identifier,
                                                     cha_ast_type *type,
                                                     cha_ast_node *value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_VARIABLE_DECLARATION;
  node->location = loc;
  node->_result_type = NULL;
  node->variable_declaration.identifier = identifier;
  node->variable_declaration.type = type;
  node->variable_declaration.value = value;
  return node;
}

cha_ast_node *make_cha_ast_node_variable_assignment(cha_ast_location loc,
                                                    char *identifier,
                                                    cha_ast_node *value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_VARIABLE_ASSIGNMENT;
  node->location = loc;
  node->_result_type = NULL;
  node->variable_assignment.identifier = identifier;
  node->variable_assignment.value = value;
  return node;
}

cha_ast_node *make_cha_ast_node_variable_lookup(cha_ast_location loc,
                                                char *identifier) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_VARIABLE_LOOKUP;
  node->location = loc;
  node->_result_type = NULL;
  node->variable_lookup.identifier = identifier;
  return node;
}

cha_ast_node *make_cha_ast_node_argument(cha_ast_location loc, char *identifier,
                                         cha_ast_type *type) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_ARGUMENT;
  node->location = loc;
  node->_result_type = NULL;
  node->argument.identifier = identifier;
  node->argument.type = type;
  return node;
}

cha_ast_node *make_cha_ast_node_block(cha_ast_location loc,
                                      cha_ast_node_list *block) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_BLOCK;
  node->location = loc;
  node->_result_type = NULL;
  node->block = block;
  return node;
}

cha_ast_node *make_cha_ast_node_function_declaration(
    cha_ast_location loc, char *identifier, cha_ast_type *return_type,
    cha_ast_node_list *argument_list, cha_ast_node_list *block) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_FUNCTION_DECLARATION;
  node->location = loc;
  node->_result_type = NULL;
  node->function_declaration.identifier = identifier;
  node->function_declaration.return_type = return_type;
  node->function_declaration.argument_list = argument_list;
  node->function_declaration.block = block;
  return node;
}

cha_ast_node *
make_cha_ast_node_function_call(cha_ast_location loc, char *identifier,
                                cha_ast_node_list *argument_list) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_FUNCTION_CALL;
  node->location = loc;
  node->_result_type = NULL;
  node->function_call.identifier = identifier;
  node->function_call.argument_list = argument_list;
  return node;
}

cha_ast_node *make_cha_ast_node_function_return(cha_ast_location loc,
                                                cha_ast_node *value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_FUNCTION_RETURN;
  node->location = loc;
  node->_result_type = NULL;
  node->function_return.value = value;
  return node;
}

cha_ast_node *make_cha_ast_node_if(cha_ast_location loc,
                                   cha_ast_node *condition,
                                   cha_ast_node_list *block,
                                   cha_ast_node_list *else_block) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_IF;
  node->location = loc;
  node->_result_type = NULL;
  node->if_block.condition = condition;
  node->if_block.block = block;
  node->if_block.else_block = else_block;
  return node;
}

cha_ast_type *make_cha_ast_primitive_type(cha_ast_location loc,
                                cha_ast_primitive_type primitive_type) {
  cha_ast_type *t = malloc(sizeof(cha_ast_type));
  t->type_definition = CHA_AST_TYPE_DEFINITION_PRIMITIVE;
  t->primitive_type = primitive_type;
  t->location = loc;
  return t;
}

void free_cha_ast_type(cha_ast_type *type) {
  if (type == NULL) {
    return;
  }

  switch (type->type_definition)
  {
  case CHA_AST_TYPE_DEFINITION_PRIMITIVE:
    // nothing to free
    break;
  case CHA_AST_TYPE_DEFINITION_ARRAY:
    free_cha_ast_type(type->array_type.element_type);
    break;
  case CHA_AST_TYPE_DEFINITION_IDENTIFIER:
    free(type->identifier);
    break;
  }
  
  free(type);
}

void free_cha_ast_node(cha_ast_node *node) {
  if (node == NULL) {
    return;
  }

  switch (node->node_type) {
  case CHA_AST_NODE_TYPE_CONSTANT_INT:
  case CHA_AST_NODE_TYPE_CONSTANT_UINT:
    free(node->const_value);
    break;
  case CHA_AST_NODE_TYPE_BIN_OP:
    free_cha_ast_node(node->bin_op.left);
    free_cha_ast_node(node->bin_op.right);
    break;
  case CHA_AST_NODE_TYPE_CONSTANT_DECLARATION:
    free_cha_ast_node(node->constant_declaration.value);
    free(node->constant_declaration.identifier);
    break;
  case CHA_AST_NODE_TYPE_VARIABLE_DECLARATION:
    free_cha_ast_type(node->variable_declaration.type);
    free(node->variable_declaration.identifier);
    break;
  case CHA_AST_NODE_TYPE_VARIABLE_ASSIGNMENT:
    free_cha_ast_node(node->variable_assignment.value);
    free(node->variable_assignment.identifier);
    break;
  case CHA_AST_NODE_TYPE_VARIABLE_LOOKUP:
    free(node->variable_lookup.identifier);
    break;
  case CHA_AST_NODE_TYPE_ARGUMENT:
    free_cha_ast_type(node->argument.type);
    free(node->argument.identifier);
    break;
  case CHA_AST_NODE_TYPE_BLOCK:
    free_cha_ast_node_list(node->block);
    break;
  case CHA_AST_NODE_TYPE_FUNCTION_DECLARATION:
    free_cha_ast_type(node->function_declaration.return_type);
    free_cha_ast_node_list(node->function_declaration.argument_list);
    free_cha_ast_node_list(node->function_declaration.block);
    free(node->function_declaration.identifier);
    break;
  case CHA_AST_NODE_TYPE_FUNCTION_CALL:
    free_cha_ast_node_list(node->function_call.argument_list);
    free(node->function_call.identifier);
    break;
  case CHA_AST_NODE_TYPE_FUNCTION_RETURN:
    free_cha_ast_node(node->function_return.value);
    break;
  case CHA_AST_NODE_TYPE_CONSTANT_BOOL:
  case CHA_AST_NODE_TYPE_CONSTANT_FLOAT:
    // nothing to free
  case CHA_AST_NODE_TYPE_IF:
    free_cha_ast_node_list(node->if_block.block);
    free_cha_ast_node_list(node->if_block.else_block);
    free_cha_ast_node(node->if_block.condition);
    break;
  }

  free_cha_ast_type(node->_result_type);
  free(node);
}

cha_ast_node_list_entry *make_cha_ast_node_list_entry(cha_ast_node *node) {
  cha_ast_node_list_entry *entry = malloc(sizeof(cha_ast_node_list_entry));
  entry->node = node;
  entry->next = NULL;
  return entry;
}

cha_ast_node_list *make_cha_ast_node_list(cha_ast_node *head) {
  cha_ast_node_list *ret = malloc(sizeof(cha_ast_node_list));
  ret->count = 1;
  ret->head = make_cha_ast_node_list_entry(head);
  ret->tail = ret->head;
  return ret;
}

void cha_ast_node_list_append(cha_ast_node_list *list, cha_ast_node *next) {
  list->count++;
  list->tail->next = make_cha_ast_node_list_entry(next);
  list->tail = list->tail->next;
}

void free_cha_ast_node_list(cha_ast_node_list *list) {
  if (list == NULL) {
    return;
  }

  while (list->head != NULL) {
    cha_ast_node_list_entry *next = list->head->next;

    free_cha_ast_node(list->head->node);
    free(list->head);
    list->head = next;
  }
  free(list);
}

cha_ast_node *make_cha_ast_node_constant_declaration(cha_ast_location loc,
                                                     char *identifier,
                                                     cha_ast_node *value) {
  cha_ast_node *node = malloc(sizeof(cha_ast_node));
  node->node_type = CHA_AST_NODE_TYPE_CONSTANT_DECLARATION;
  node->location = loc;
  node->_result_type = NULL;
  node->constant_declaration.identifier = identifier;
  node->constant_declaration.value = value;
  return node;
}
