#include "ni/validate.h"
#include "log.h"
#include "ni/ast.h"
#include "symbol_table.h"

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

symbol_table *fn_validate_table = NULL;
symbol_table *var_validate_table = NULL;
ni_ast_node *fun = NULL;

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
    if (ast_current_node->node->type != NI_AST_TYPE_FUNCTION_DECLARATION) {
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

  switch (ast_node->type) {
  case NI_AST_TYPE_FUNCTION_DECLARATION:
    return ni_validate_node_fun(ast_node);
  case NI_AST_TYPE_BLOCK:
    return ni_validate_node_list(ast_node->block);
  case NI_AST_TYPE_VARIABLE_DECLARATION:
    return ni_validate_node_var(ast_node);
  case NI_AST_TYPE_BIN_OP:
    return ni_validate_node_bin_op(ast_node);
  case NI_AST_TYPE_VARIABLE_ASSIGNMENT:
    return ni_validate_node_var_assign(ast_node);
  case NI_AST_TYPE_VARIABLE_LOOKUP:
    return ni_validate_node_var_lookup(ast_node);
  case NI_AST_TYPE_FUNCTION_CALL:
    return ni_validate_node_call(ast_node);
  case NI_AST_TYPE_FUNCTION_RETURN:
    return ni_validate_node_ret(ast_node);
  case NI_AST_TYPE_ARGUMENT:
    return ni_validate_node_arg(ast_node);
  case NI_AST_TYPE_CONSTANT_NUMBER:
  case NI_AST_TYPE_REFTYPE_BYTE:
  case NI_AST_TYPE_REFTYPE_SBYTE:
  case NI_AST_TYPE_REFTYPE_INT:
  case NI_AST_TYPE_REFTYPE_UINT:
  case NI_AST_TYPE_REFTYPE_SHORT:
  case NI_AST_TYPE_REFTYPE_USHORT:
  case NI_AST_TYPE_REFTYPE_LONG:
  case NI_AST_TYPE_REFTYPE_ULONG:
  case NI_AST_TYPE_REFTYPE_LARGE:
  case NI_AST_TYPE_REFTYPE_ULARGE:
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
  if (get_symbol_table(var_validate_table,
                       ast_node->variable_assignment.identifier) == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_assignment.identifier);
    return 1;
  }

  return ni_validate_node(ast_node->variable_assignment.value);
}

int ni_validate_node_var_lookup(ni_ast_node *ast_node) {
  if (get_symbol_table(var_validate_table,
                       ast_node->variable_lookup.identifier) == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_lookup.identifier);
    return 1;
  }
  return 0;
}

int ni_validate_node_bin_op(ni_ast_node *ast_node) {
  int ret_left = ni_validate_node(ast_node->bin_op.left);
  int ret_right = ni_validate_node(ast_node->bin_op.right);
  if (ret_left != 0 || ret_right != 0) {
    return 1;
  }
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
        log_validation_error(ast_node->location,
                             "function '%s' expects %d arguments",
                             ast_node->function_call.identifier,
                             fun->function_declaration.argument_list->count);
        ret = 1;
      } else {
        ni_ast_node_list_entry *arg =
            ast_node->function_call.argument_list->head;
        while (arg != NULL) {
          int ret_arg = ni_validate_node(arg->node);
          if (ret_arg != 0) {
            ret = ret_arg;
          }
          arg = arg->next;
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

  return ret;
}
