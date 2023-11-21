#ifndef __NI_AST_H__
#define __NI_AST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct ni_ast_location {
  char file[500];
  int line_begin;
  int column_begin;
  int line_end;
  int column_end;
} ni_ast_location;

typedef enum ni_ast_internal_type {
  NI_AST_INTERNAL_TYPE_UNDEF = -1,
  NI_AST_INTERNAL_TYPE_CONST_INT,
  NI_AST_INTERNAL_TYPE_INT,
  NI_AST_INTERNAL_TYPE_INT8,
  NI_AST_INTERNAL_TYPE_INT16,
  NI_AST_INTERNAL_TYPE_INT32,
  NI_AST_INTERNAL_TYPE_INT64,
  NI_AST_INTERNAL_TYPE_INT128,
  NI_AST_INTERNAL_TYPE_CONST_UINT,
  NI_AST_INTERNAL_TYPE_UINT,
  NI_AST_INTERNAL_TYPE_UINT8,
  NI_AST_INTERNAL_TYPE_UINT16,
  NI_AST_INTERNAL_TYPE_UINT32,
  NI_AST_INTERNAL_TYPE_UINT64,
  NI_AST_INTERNAL_TYPE_UINT128,
  NI_AST_INTERNAL_TYPE_CONST_FLOAT,
  NI_AST_INTERNAL_TYPE_FLOAT16,
  NI_AST_INTERNAL_TYPE_FLOAT32,
  NI_AST_INTERNAL_TYPE_FLOAT64,
  NI_AST_INTERNAL_TYPE_BOOL,
} ni_ast_internal_type;

typedef enum ni_ast_node_type {
  NI_AST_NODE_TYPE_CONSTANT_INT,
  NI_AST_NODE_TYPE_CONSTANT_UINT,
  NI_AST_NODE_TYPE_CONSTANT_FLOAT,
  NI_AST_NODE_TYPE_CONSTANT_BOOL,
  NI_AST_NODE_TYPE_BIN_OP,
  NI_AST_NODE_TYPE_CONSTANT_DECLARATION,
  NI_AST_NODE_TYPE_VARIABLE_DECLARATION,
  NI_AST_NODE_TYPE_VARIABLE_ASSIGNMENT,
  NI_AST_NODE_TYPE_VARIABLE_LOOKUP,
  NI_AST_NODE_TYPE_ARGUMENT,
  NI_AST_NODE_TYPE_BLOCK,
  NI_AST_NODE_TYPE_FUNCTION_DECLARATION,
  NI_AST_NODE_TYPE_FUNCTION_CALL,
  NI_AST_NODE_TYPE_FUNCTION_RETURN,
} ni_ast_node_type;

typedef enum ni_ast_operator {
  NI_AST_OPERATOR_ADD,
  NI_AST_OPERATOR_SUBTRACT,
  NI_AST_OPERATOR_MULTIPLY,
  NI_AST_OPERATOR_EQUALS_EQUALS,
  NI_AST_OPERATOR_NOT_EQUALS,
  NI_AST_OPERATOR_GREATER_THAN,
  NI_AST_OPERATOR_GREATER_THAN_OR_EQUALS,
  NI_AST_OPERATOR_LESS_THAN,
  NI_AST_OPERATOR_LESS_THAN_OR_EQUALS,
  NI_AST_OPERATOR_AND,
  NI_AST_OPERATOR_OR,
} ni_ast_operator;

typedef struct ni_ast_node_list ni_ast_node_list;
typedef struct ni_ast_node ni_ast_node;
typedef struct ni_ast_node_list_entry ni_ast_node_list_entry;
typedef struct ni_ast_type ni_ast_type;

struct ni_ast_node {
  ni_ast_node_type node_type;
  ni_ast_location location;
  ni_ast_type *_result_type;
  union {
    char *const_value;
    short const_bool;
    struct {
      ni_ast_operator op;
      ni_ast_node *left;
      ni_ast_node *right;
    } bin_op;
    struct {
      char *identifier;
      ni_ast_node *value;
    } constant_declaration;
    struct {
      char *identifier;
      ni_ast_type *type;
      ni_ast_node *value;
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
      ni_ast_type *type;
    } argument;
    ni_ast_node_list *block;
    struct {
      char *identifier;
      ni_ast_type *return_type;
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

struct ni_ast_type {
  ni_ast_internal_type internal_type;
  ni_ast_location location;
};

ni_ast_node *make_ni_ast_node_constant_integer(ni_ast_location loc,
                                               const char *value);
ni_ast_node *make_ni_ast_node_constant_unsigned_integer(ni_ast_location loc,
                                                        const char *value);
ni_ast_node *make_ni_ast_node_constant_float(ni_ast_location loc,
                                             const char *value);
ni_ast_node *make_ni_ast_node_constant_true(ni_ast_location loc);
ni_ast_node *make_ni_ast_node_constant_false(ni_ast_location loc);
ni_ast_type *make_ni_ast_type(ni_ast_location loc,
                              ni_ast_internal_type internal_type);
ni_ast_type *make_ni_ast_type_int(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_uint(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_int8(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_uint8(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_int16(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_uint16(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_int32(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_uint32(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_int64(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_uint64(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_int128(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_uint128(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_float16(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_float32(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_float64(ni_ast_location loc);
ni_ast_type *make_ni_ast_type_bool(ni_ast_location loc);
ni_ast_node *make_ni_ast_node_bin_op(ni_ast_location loc, ni_ast_operator op,
                                     ni_ast_node *left, ni_ast_node *right);
ni_ast_node *make_ni_ast_node_variable_declaration(ni_ast_location loc,
                                                   const char *identifier,
                                                   ni_ast_type *type,
                                                   ni_ast_node *value);
ni_ast_node *make_ni_ast_node_variable_assignment(ni_ast_location loc,
                                                  const char *identifier,
                                                  ni_ast_node *value);
ni_ast_node *make_ni_ast_node_variable_lookup(ni_ast_location loc,
                                              const char *identifier);
ni_ast_node *make_ni_ast_node_argument(ni_ast_location loc,
                                       const char *identifier,
                                       ni_ast_type *type);
ni_ast_node *make_ni_ast_node_block(ni_ast_location loc,
                                    ni_ast_node_list *block);
ni_ast_node *make_ni_ast_node_function_declaration(
    ni_ast_location loc, const char *identifier, ni_ast_type *return_type,
    ni_ast_node_list *argument_list, ni_ast_node_list *block);
ni_ast_node *make_ni_ast_node_constant_declaration(ni_ast_location loc,
                                                   const char *identifier,
                                                   ni_ast_node *value);
ni_ast_node *make_ni_ast_node_function_call(ni_ast_location loc,
                                            const char *identifier,
                                            ni_ast_node_list *argument_list);
ni_ast_node *make_ni_ast_node_function_return(ni_ast_location loc,
                                              ni_ast_node *value);
ni_ast_node_list *make_ni_ast_node_list(ni_ast_node *head);
void ni_ast_node_list_append(ni_ast_node_list *list, ni_ast_node *next);

void free_ni_ast_node(ni_ast_node *node);
void free_ni_ast_node_list(ni_ast_node_list *list);

#ifdef __cplusplus
}
#endif

#endif // __NI_AST_H__
