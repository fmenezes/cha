#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "log.h"
#include "symbol_table.h"
#include "validate.h"

int ni_validate_function_list(ni_ast_node_list *ast);
int ni_validate_node_list(ni_ast_node_list *ast);
int ni_validate_node(ni_ast_node *ast_node);
int ni_validate_node_fun(ni_ast_node *ast_node);
int ni_validate_node_var(ni_ast_node *ast_node);
int ni_validate_node_arg(ni_ast_node *ast_node);
int ni_validate_node_var_assign(ni_ast_node *ast_node);
int ni_validate_node_var_lookup(ni_ast_node *ast_node);
int ni_validate_node_bin_op(ni_ast_node *ast_node);
int ni_validate_node_call(ni_ast_node *ast_node);
int ni_validate_node_ret(ni_ast_node *ast_node);
int ni_validate_const_num(ni_ast_node *ast_node);
int ni_validate_const_float(ni_ast_node *ast_node);
int type_cmp(ni_ast_type *ast_type_1, ni_ast_type *ast_type_2);

symbol_table *fn_validate_table = NULL;
symbol_table *var_validate_table = NULL;
ni_ast_node *fun = NULL;

int ni_validate_const_num(ni_ast_node *ast_node) {
  ast_node->_result_type =
      make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_INT32);
  return 0;
}

int ni_validate_const_float(ni_ast_node *ast_node) {
  ast_node->_result_type =
      make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_FLOAT64);
  return 0;
}

int ni_validate(ni_ast_node_list *ast) {
  fn_validate_table = make_symbol_table(SYMBOL_TABLE_SIZE);

  int ret = ni_validate_function_list(ast);
  if (ret != 0) {
    free_symbol_table(fn_validate_table);
    return ret;
  }

  ret = ni_validate_node_list(ast);

  free_symbol_table(fn_validate_table);
  return ret;
}

int ni_validate_function_list(ni_ast_node_list *ast) {
  int ret = 0;
  ni_ast_node_list_entry *ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    if (ast_current_node->node->node_type !=
        NI_AST_NODE_TYPE_FUNCTION_DECLARATION) {
      log_validation_error(ast_current_node->node->location,
                           "expected function declaration");
      ret = 1;
    } else {
      ret = insert_symbol_table(
          fn_validate_table,
          ast_current_node->node->function_declaration.identifier,
          ast_current_node->node, NULL, NULL);

      if (ret != 0) {
        log_validation_error(
            ast_current_node->node->location, "function '%s' already defined",
            ast_current_node->node->function_declaration.identifier);
      }
    }
    ast_current_node = ast_current_node->next;
  }
  return ret;
}

int ni_validate_node_list(ni_ast_node_list *ast) {
  if (ast == NULL) {
    return 0;
  }

  int ret = 0;
  ni_ast_node_list_entry *ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    ret = ni_validate_node(ast_current_node->node);
    if (ret != 0) {
      return ret;
    }
    ast_current_node = ast_current_node->next;
  }
  return ret;
}

int ni_validate_node(ni_ast_node *ast_node) {
  if (ast_node == NULL) {
    return 0;
  }

  switch (ast_node->node_type) {
  case NI_AST_NODE_TYPE_FUNCTION_DECLARATION:
    return ni_validate_node_fun(ast_node);
  case NI_AST_NODE_TYPE_BLOCK:
    return ni_validate_node_list(ast_node->block);
  case NI_AST_NODE_TYPE_VARIABLE_DECLARATION:
    return ni_validate_node_var(ast_node);
  case NI_AST_NODE_TYPE_BIN_OP:
    return ni_validate_node_bin_op(ast_node);
  case NI_AST_NODE_TYPE_VARIABLE_ASSIGNMENT:
    return ni_validate_node_var_assign(ast_node);
  case NI_AST_NODE_TYPE_VARIABLE_LOOKUP:
    return ni_validate_node_var_lookup(ast_node);
  case NI_AST_NODE_TYPE_FUNCTION_CALL:
    return ni_validate_node_call(ast_node);
  case NI_AST_NODE_TYPE_FUNCTION_RETURN:
    return ni_validate_node_ret(ast_node);
  case NI_AST_NODE_TYPE_ARGUMENT:
    return ni_validate_node_arg(ast_node);
  case NI_AST_NODE_TYPE_CONSTANT_NUMBER:
    return ni_validate_const_num(ast_node);
  case NI_AST_NODE_TYPE_CONSTANT_FLOAT:
    return ni_validate_const_float(ast_node);
  default:
    return 0; // no validation
  }
}

int ni_validate_node_fun(ni_ast_node *ast_node) {
  var_validate_table = make_symbol_table(SYMBOL_TABLE_SIZE);

  fun = ast_node;

  int ret = ni_validate_node_list(ast_node->function_declaration.argument_list);

  int ret_block = ni_validate_node_list(ast_node->function_declaration.block);
  if (ret_block != 0) {
    ret = ret_block;
  }

  free_symbol_table(var_validate_table);
  return ret;
}

int ni_validate_node_var(ni_ast_node *ast_node) {
  int ret = insert_symbol_table(var_validate_table,
                                ast_node->variable_declaration.identifier,
                                ast_node, NULL, NULL);
  if (ret != 0) {
    log_validation_error(ast_node->location, "variable '%s' already defined",
                         ast_node->variable_declaration.identifier);
  }
  return ret;
}

int ni_validate_node_arg(ni_ast_node *ast_node) {
  int ret = insert_symbol_table(
      var_validate_table, ast_node->argument.identifier, ast_node, NULL, NULL);
  if (ret != 0) {
    log_validation_error(ast_node->location, "argument '%s' already defined",
                         ast_node->argument.identifier);
  }
  return ret;
}

int ni_validate_node_var_assign(ni_ast_node *ast_node) {
  symbol_value *v = get_symbol_table(var_validate_table,
                                     ast_node->variable_assignment.identifier);
  if (v == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_assignment.identifier);
    return 1;
  }

  if (ni_validate_node(ast_node->variable_assignment.value) != 0) {
    return 1;
  }

  if (type_cmp(ast_node->variable_assignment.value->_result_type,
               v->node->variable_declaration.type) != 0) {
    char *expected_type = NULL;
    char *got_type = NULL;
    type_str(v->node->variable_declaration.type, &expected_type);
    type_str(ast_node->variable_assignment.value->_result_type, &got_type);
    log_validation_error(ast_node->location,
                         "type mismatch expects '%s' passed '%s'",
                         expected_type, got_type);
    free(expected_type);
    free(got_type);
    return 1;
  }

  return 0;
}

int ni_validate_node_var_lookup(ni_ast_node *ast_node) {
  symbol_value *v = get_symbol_table(var_validate_table,
                                     ast_node->variable_lookup.identifier);
  if (v == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_lookup.identifier);
    return 1;
  }
  ast_node->_result_type = make_ni_ast_type(
      ast_node->location, v->node->variable_declaration.type->internal_type);
  return 0;
}

int ni_validate_node_bin_op(ni_ast_node *ast_node) {
  int ret_left = ni_validate_node(ast_node->bin_op.left);
  int ret_right = ni_validate_node(ast_node->bin_op.right);
  if (ret_left != 0 || ret_right != 0) {
    return 1;
  }
  if (type_cmp(ast_node->bin_op.left->_result_type,
               ast_node->bin_op.right->_result_type) != 0) {
    char op[5];
    switch (ast_node->bin_op.op) {
    case NI_AST_OPERATOR_PLUS:
      sprintf(op, "+");
      break;
    case NI_AST_OPERATOR_MINUS:
      sprintf(op, "-");
      break;
    case NI_AST_OPERATOR_MULTIPLY:
      sprintf(op, "*");
      break;
    }

    char *ltype = NULL;
    char *rtype = NULL;
    type_str(ast_node->bin_op.left->_result_type, &ltype);
    type_str(ast_node->bin_op.right->_result_type, &rtype);

    log_validation_error(ast_node->location,
                         "incompatible types found: %s %s %s", ltype, op,
                         rtype);

    free(ltype);
    free(rtype);

    return 1;
  }
  ast_node->_result_type = make_ni_ast_type(
      ast_node->location, ast_node->bin_op.left->_result_type->internal_type);
  return 0;
}

int ni_validate_node_call(ni_ast_node *ast_node) {
  symbol_value *callee_fun =
      get_symbol_table(fn_validate_table, ast_node->function_call.identifier);
  if (callee_fun == NULL) {
    log_validation_error(ast_node->location, "function '%s' not found",
                         ast_node->function_call.identifier);
    return 1;
  }
  if (callee_fun->node->function_declaration.return_type != NULL) {
    ast_node->_result_type = make_ni_ast_type(
        ast_node->location,
        callee_fun->node->function_declaration.return_type->internal_type);
  }
  int ret = 0;
  if ((ast_node->function_call.argument_list == NULL ||
       ast_node->function_call.argument_list->count == 0) &&
      callee_fun->node->function_declaration.argument_list != NULL &&
      callee_fun->node->function_declaration.argument_list->count > 0) {
    log_validation_error(ast_node->location, "function '%s' expects arguments",
                         ast_node->function_call.identifier);
  } else if (ast_node->function_call.argument_list != NULL &&
             ast_node->function_call.argument_list->count > 0) {
    if (callee_fun->node->function_declaration.argument_list == NULL) {
      log_validation_error(ast_node->location,
                           "function '%s' expects no arguments",
                           ast_node->function_call.identifier);
      ret = 1;
    } else {
      if (ast_node->function_call.argument_list->count !=
          callee_fun->node->function_declaration.argument_list->count) {
        log_validation_error(
            ast_node->location,
            "function '%s' expects %d arguments but %d were passed",
            ast_node->function_call.identifier,
            fun->function_declaration.argument_list->count,
            ast_node->function_call.argument_list->count);
        ret = 1;
      } else {
        ni_ast_node_list_entry *arg =
            ast_node->function_call.argument_list->head;
        ni_ast_node_list_entry *def_arg =
            callee_fun->node->function_declaration.argument_list->head;
        while (arg != NULL) {
          int ret_arg = ni_validate_node(arg->node);
          if (ret_arg != 0) {
            ret = ret_arg;
          }
          ret_arg =
              type_cmp(arg->node->_result_type, def_arg->node->argument.type);
          if (ret_arg != 0) {
            char *expected_type = NULL;
            char *got_type = NULL;
            type_str(def_arg->node->argument.type, &expected_type);
            type_str(arg->node->_result_type, &got_type);
            log_validation_error(arg->node->location,
                                 "type mismatch expects '%s' passed '%s'",
                                 expected_type, got_type);
            free(expected_type);
            free(got_type);
            ret = ret_arg;
          }
          arg = arg->next;
          def_arg = def_arg->next;
        }
      }
    }
  }

  return ret;
}

int ni_validate_node_ret(ni_ast_node *ast_node) {
  int ret = ni_validate_node(ast_node->function_return.value);

  if (ast_node->function_return.value == NULL &&
      fun->function_declaration.return_type != NULL) {
    log_validation_error(ast_node->location, "return value expected");
    ret = 1;
  } else if (ast_node->function_return.value != NULL &&
             fun->function_declaration.return_type == NULL) {
    log_validation_error(ast_node->location, "return value not expected");
    ret = 1;
  }

  int type_ret = type_cmp(ast_node->function_return.value->_result_type,
                          fun->function_declaration.return_type);
  if (type_ret != 0) {
    char *expected_type = NULL;
    char *got_type = NULL;
    type_str(fun->function_declaration.return_type, &expected_type);
    type_str(ast_node->function_return.value->_result_type, &got_type);
    log_validation_error(ast_node->location,
                         "return type mismatch expects '%s' passed '%s'",
                         expected_type, got_type);
    free(expected_type);
    free(got_type);
    ret = type_ret;
  }

  return ret;
}

int type_cmp(ni_ast_type *ast_type_1, ni_ast_type *ast_type_2) {
  if (ast_type_1 == NULL && ast_type_2 == NULL) {
    return 0;
  }

  if (ast_type_1 != NULL && ast_type_2 != NULL &&
      ast_type_1->internal_type == ast_type_2->internal_type) {
    return 0;
  }

  return 1;
}

void type_str(const ni_ast_type *ast_type, char **str) {
  if (str == NULL) {
    return;
  }

  char out[30];
  if (ast_type == NULL) {
    sprintf(out, "void");
  } else {
    switch (ast_type->internal_type) {
    case NI_AST_INTERNAL_TYPE_UINT8:
      sprintf(out, "uint8");
      break;
    case NI_AST_INTERNAL_TYPE_INT8:
      sprintf(out, "int8");
      break;
    case NI_AST_INTERNAL_TYPE_INT16:
      sprintf(out, "int16");
      break;
    case NI_AST_INTERNAL_TYPE_UINT16:
      sprintf(out, "uint16");
      break;
    case NI_AST_INTERNAL_TYPE_INT32:
      sprintf(out, "int32");
      break;
    case NI_AST_INTERNAL_TYPE_UINT32:
      sprintf(out, "uint32");
      break;
    case NI_AST_INTERNAL_TYPE_INT64:
      sprintf(out, "int64");
      break;
    case NI_AST_INTERNAL_TYPE_UINT64:
      sprintf(out, "uint64");
      break;
    case NI_AST_INTERNAL_TYPE_INT128:
      sprintf(out, "int128");
      break;
    case NI_AST_INTERNAL_TYPE_UINT128:
      sprintf(out, "uint128");
      break;
    case NI_AST_INTERNAL_TYPE_FLOAT32:
      sprintf(out, "float32");
      break;
    case NI_AST_INTERNAL_TYPE_FLOAT64:
      sprintf(out, "float64");
      break;
    }
  }
  *str = strdup(out);
}
