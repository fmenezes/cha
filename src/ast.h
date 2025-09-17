#ifndef __CHA_AST_H__
#define __CHA_AST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct cha_ast_location {
  char file[500];
  int line_begin;
  int column_begin;
  int line_end;
  int column_end;
} cha_ast_location;

typedef enum cha_ast_primitive_type {
  CHA_AST_PRIMITIVE_TYPE_UNDEF = -1,
  CHA_AST_PRIMITIVE_TYPE_CONST_INT,
  CHA_AST_PRIMITIVE_TYPE_INT,
  CHA_AST_PRIMITIVE_TYPE_INT8,
  CHA_AST_PRIMITIVE_TYPE_INT16,
  CHA_AST_PRIMITIVE_TYPE_INT32,
  CHA_AST_PRIMITIVE_TYPE_INT64,
  CHA_AST_PRIMITIVE_TYPE_CONST_UINT,
  CHA_AST_PRIMITIVE_TYPE_UINT,
  CHA_AST_PRIMITIVE_TYPE_UINT8,
  CHA_AST_PRIMITIVE_TYPE_UINT16,
  CHA_AST_PRIMITIVE_TYPE_UINT32,
  CHA_AST_PRIMITIVE_TYPE_UINT64,
  CHA_AST_PRIMITIVE_TYPE_CONST_FLOAT,
  CHA_AST_PRIMITIVE_TYPE_FLOAT16,
  CHA_AST_PRIMITIVE_TYPE_FLOAT32,
  CHA_AST_PRIMITIVE_TYPE_FLOAT64,
  CHA_AST_PRIMITIVE_TYPE_BOOL,
} cha_ast_primitive_type;

typedef enum cha_ast_node_type {
  CHA_AST_NODE_TYPE_CONSTANT_INT,
  CHA_AST_NODE_TYPE_CONSTANT_UINT,
  CHA_AST_NODE_TYPE_CONSTANT_FLOAT,
  CHA_AST_NODE_TYPE_CONSTANT_BOOL,
  CHA_AST_NODE_TYPE_BIN_OP,
  CHA_AST_NODE_TYPE_CONSTANT_DECLARATION,
  CHA_AST_NODE_TYPE_VARIABLE_DECLARATION,
  CHA_AST_NODE_TYPE_VARIABLE_ASSIGNMENT,
  CHA_AST_NODE_TYPE_VARIABLE_LOOKUP,
  CHA_AST_NODE_TYPE_ARGUMENT,
  CHA_AST_NODE_TYPE_BLOCK,
  CHA_AST_NODE_TYPE_FUNCTION_DECLARATION,
  CHA_AST_NODE_TYPE_FUNCTION_CALL,
  CHA_AST_NODE_TYPE_FUNCTION_RETURN,
  CHA_AST_NODE_TYPE_IF,
} cha_ast_node_type;

typedef enum cha_ast_operator {
  CHA_AST_OPERATOR_ADD,
  CHA_AST_OPERATOR_SUBTRACT,
  CHA_AST_OPERATOR_MULTIPLY,
  CHA_AST_OPERATOR_DIVIDE,
  CHA_AST_OPERATOR_EQUALS_EQUALS,
  CHA_AST_OPERATOR_NOT_EQUALS,
  CHA_AST_OPERATOR_GREATER_THAN,
  CHA_AST_OPERATOR_GREATER_THAN_OR_EQUALS,
  CHA_AST_OPERATOR_LESS_THAN,
  CHA_AST_OPERATOR_LESS_THAN_OR_EQUALS,
  CHA_AST_OPERATOR_AND,
  CHA_AST_OPERATOR_OR,
} cha_ast_operator;

typedef struct cha_ast_node_list cha_ast_node_list;
typedef struct cha_ast_node cha_ast_node;
typedef struct cha_ast_node_list_entry cha_ast_node_list_entry;
typedef struct cha_ast_type cha_ast_type;

struct cha_ast_node {
  cha_ast_node_type node_type;
  cha_ast_location location;
  cha_ast_type *_result_type;
  union {
    char *const_value;
    double const_float;
    short const_bool;
    struct {
      cha_ast_operator op;
      cha_ast_node *left;
      cha_ast_node *right;
    } bin_op;
    struct {
      char *identifier;
      cha_ast_node *value;
    } constant_declaration;
    struct {
      char *identifier;
      cha_ast_type *type;
      cha_ast_node *value;
    } variable_declaration;
    struct {
      char *identifier;
      cha_ast_node *value;
    } variable_assignment;
    struct {
      char *identifier;
    } variable_lookup;
    struct {
      char *identifier;
      cha_ast_type *type;
    } argument;
    cha_ast_node_list *block;
    struct {
      char *identifier;
      cha_ast_type *return_type;
      cha_ast_node_list *argument_list;
      cha_ast_node_list *block;
    } function_declaration;
    struct {
      char *identifier;
      cha_ast_node_list *argument_list;
    } function_call;
    struct {
      cha_ast_node *value;
    } function_return;
    struct {
      cha_ast_node *condition;
      cha_ast_node_list *block;
      cha_ast_node_list *else_block;
    } if_block;
  };
};

struct cha_ast_node_list_entry {
  cha_ast_node *node;
  cha_ast_node_list_entry *next;
};

struct cha_ast_node_list {
  unsigned int count;
  cha_ast_node_list_entry *head;
  cha_ast_node_list_entry *tail;
};

struct cha_ast_type {
  cha_ast_primitive_type primitive_type;
  cha_ast_location location;
};

cha_ast_node *make_cha_ast_node_constant_integer(cha_ast_location loc,
                                                 char *value);
cha_ast_node *make_cha_ast_node_constant_unsigned_integer(cha_ast_location loc,
                                                          char *value);
cha_ast_node *make_cha_ast_node_constant_float(cha_ast_location loc,
                                               double value);
cha_ast_node *make_cha_ast_node_constant_true(cha_ast_location loc);
cha_ast_node *make_cha_ast_node_constant_false(cha_ast_location loc);
cha_ast_type *make_cha_ast_type(cha_ast_location loc,
                                cha_ast_primitive_type primitive_type);
cha_ast_type *make_cha_ast_type_int(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_uint(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_int8(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_uint8(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_int16(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_uint16(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_int32(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_uint32(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_int64(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_uint64(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_float16(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_float32(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_float64(cha_ast_location loc);
cha_ast_type *make_cha_ast_type_bool(cha_ast_location loc);
cha_ast_node *make_cha_ast_node_bin_op(cha_ast_location loc,
                                       cha_ast_operator op, cha_ast_node *left,
                                       cha_ast_node *right);
cha_ast_node *make_cha_ast_node_variable_declaration(cha_ast_location loc,
                                                     char *identifier,
                                                     cha_ast_type *type,
                                                     cha_ast_node *value);
cha_ast_node *make_cha_ast_node_variable_assignment(cha_ast_location loc,
                                                    char *identifier,
                                                    cha_ast_node *value);
cha_ast_node *make_cha_ast_node_variable_lookup(cha_ast_location loc,
                                                char *identifier);
cha_ast_node *make_cha_ast_node_argument(cha_ast_location loc, char *identifier,
                                         cha_ast_type *type);
cha_ast_node *make_cha_ast_node_block(cha_ast_location loc,
                                      cha_ast_node_list *block);
cha_ast_node *make_cha_ast_node_function_declaration(
    cha_ast_location loc, char *identifier, cha_ast_type *return_type,
    cha_ast_node_list *argument_list, cha_ast_node_list *block);
cha_ast_node *make_cha_ast_node_constant_declaration(cha_ast_location loc,
                                                     char *identifier,
                                                     cha_ast_node *value);
cha_ast_node *make_cha_ast_node_function_call(cha_ast_location loc,
                                              char *identifier,
                                              cha_ast_node_list *argument_list);
cha_ast_node *make_cha_ast_node_function_return(cha_ast_location loc,
                                                cha_ast_node *value);
cha_ast_node *make_cha_ast_node_if(cha_ast_location loc,
                                   cha_ast_node *condition,
                                   cha_ast_node_list *block,
                                   cha_ast_node_list *else_block);
cha_ast_node_list *make_cha_ast_node_list(cha_ast_node *head);
void cha_ast_node_list_append(cha_ast_node_list *list, cha_ast_node *next);

void free_cha_ast_node(cha_ast_node *node);
void free_cha_ast_node_list(cha_ast_node_list *list);

#ifdef __cplusplus
}
#endif

#endif // __CHA_AST_H__
