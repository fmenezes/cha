#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "log.h"
#include "symbol_table.h"
#include "validate.h"

int cha_validate_top_level(cha_ast_node_list *ast);
int cha_validate_node_list(cha_ast_node_list *ast);
int cha_validate_node(cha_ast_node *ast_node);
int cha_validate_node_fun(cha_ast_node *ast_node);
int cha_validate_node_const(cha_ast_node *ast_node);
int cha_validate_node_var(cha_ast_node *ast_node);
int cha_validate_node_arg(cha_ast_node *ast_node);
int cha_validate_node_var_assign(cha_ast_node *ast_node);
int cha_validate_node_var_lookup(cha_ast_node *ast_node);
int cha_validate_node_bin_op(cha_ast_node *ast_node);
int cha_validate_node_call(cha_ast_node *ast_node);
int cha_validate_node_ret(cha_ast_node *ast_node);
int cha_validate_node_if(cha_ast_node *ast_node);
int cha_check_type_assignment(cha_ast_node *ast_node,
                              const cha_ast_type *ast_type);
void cha_set_type_on_const(cha_ast_node *ast_node,
                           cha_ast_internal_type internal_type);
void cha_validate_create_stack_frame();
void cha_validate_release_stack_frame();

symbol_table *validate_symbol_table = NULL;
cha_ast_node *fun = NULL;

// Type classification and conversion helpers
static inline bool is_signed_int(cha_ast_internal_type type) {
  return type >= CHA_AST_INTERNAL_TYPE_CONST_INT &&
         type <= CHA_AST_INTERNAL_TYPE_INT64;
}

static inline bool is_unsigned_int(cha_ast_internal_type type) {
  return type >= CHA_AST_INTERNAL_TYPE_CONST_UINT &&
         type <= CHA_AST_INTERNAL_TYPE_UINT64;
}

static bool is_float(cha_ast_internal_type type) {
  return (type == CHA_AST_INTERNAL_TYPE_FLOAT16 ||
          type == CHA_AST_INTERNAL_TYPE_FLOAT32 ||
          type == CHA_AST_INTERNAL_TYPE_FLOAT64 ||
          type == CHA_AST_INTERNAL_TYPE_CONST_FLOAT);
}

static bool is_numeric(cha_ast_internal_type type) {
  return is_signed_int(type) || is_unsigned_int(type) || is_float(type);
}

// Main arithmetic conversion function
static cha_ast_internal_type
cha_convert_arithmetic_types(cha_ast_internal_type left,
                             cha_ast_internal_type right) {
  if (is_float(left) && is_float(right)) {
    return (left > right) ? left : right;
  }

  if (is_signed_int(left) && is_signed_int(right)) {
    return (left > right) ? left : right;
  }

  if (is_unsigned_int(left) && is_unsigned_int(right)) {
    return (left > right) ? left : right;
  }

  return CHA_AST_INTERNAL_TYPE_UNDEF; // Default: incompatible types
}

// Assignment compatibility check
static int cha_check_assignment_compatibility(cha_ast_internal_type from,
                                              cha_ast_internal_type to) {
  // Assignment is more permissive than arithmetic operations
  if (from == CHA_AST_INTERNAL_TYPE_UNDEF ||
      to == CHA_AST_INTERNAL_TYPE_UNDEF) {
    return 1; // Error
  }

  // Same type assignments (except for non-assignable types)
  if (from == to) {
    return (from == CHA_AST_INTERNAL_TYPE_CONST_INT ||
            from == CHA_AST_INTERNAL_TYPE_CONST_UINT ||
            from == CHA_AST_INTERNAL_TYPE_CONST_FLOAT)
               ? 1
               : 0;
  }

  // Constants can be assigned to their corresponding types
  if (from == CHA_AST_INTERNAL_TYPE_CONST_INT && is_signed_int(to))
    return 0;
  if (from == CHA_AST_INTERNAL_TYPE_CONST_UINT && is_unsigned_int(to))
    return 0;
  if (from == CHA_AST_INTERNAL_TYPE_CONST_FLOAT && is_float(to))
    return 0;

  // Bool assignments are very restrictive
  if (to == CHA_AST_INTERNAL_TYPE_BOOL) {
    return (from == CHA_AST_INTERNAL_TYPE_BOOL) ? 0 : 1;
  }

  // Check for compatible type ranges
  if (is_numeric(from) && is_numeric(to)) {
    // Allow if they're the same category (signed->signed, unsigned->unsigned)
    if (is_signed_int(from) && is_signed_int(to))
      return 0;
    if (is_unsigned_int(from) && is_unsigned_int(to))
      return 0;
    if (is_float(from) && is_float(to))
      return 0;

    // Otherwise, mixed sign/type assignments are not allowed
    return 1;
  }

  // Otherwise, incompatible types
  return 1;
}

// Numeric comparison compatibility (returns 0 if compatible, 1 if error)
static int
cha_check_numeric_comparison_compatibility(cha_ast_internal_type left,
                                           cha_ast_internal_type right) {
  if (left == CHA_AST_INTERNAL_TYPE_UNDEF ||
      right == CHA_AST_INTERNAL_TYPE_UNDEF) {
    return 1;
  }

  // Same type comparisons
  if (left == right) {
    return (left == CHA_AST_INTERNAL_TYPE_CONST_INT ||
            left == CHA_AST_INTERNAL_TYPE_CONST_UINT ||
            left == CHA_AST_INTERNAL_TYPE_CONST_FLOAT)
               ? 1
               : 0;
  }

  // Numeric types can be compared
  if ((is_signed_int(left) || is_unsigned_int(left) || is_float(left)) &&
      (is_signed_int(right) || is_unsigned_int(right) || is_float(right))) {
    return 0;
  }

  // Bool can be compared with bool
  if (left == CHA_AST_INTERNAL_TYPE_BOOL &&
      right == CHA_AST_INTERNAL_TYPE_BOOL) {
    return 0;
  }

  return 1; // Incompatible
}

// Equality comparison compatibility (returns 0 if compatible, 1 if error)
static int
cha_check_equality_comparison_compatibility(cha_ast_internal_type left,
                                            cha_ast_internal_type right) {
  // Equality is similar to numeric comparison but slightly more permissive
  return cha_check_numeric_comparison_compatibility(left, right);
}

// clang-format on
int cha_validate(cha_ast_node_list *ast) {
  validate_symbol_table = make_symbol_table(SYMBOL_TABLE_SIZE, NULL);

  int ret = cha_validate_top_level(ast);

  free_all_symbol_tables(validate_symbol_table);
  return ret;
}

int cha_validate_top_level(cha_ast_node_list *ast) {
  int ret = 0;
  cha_ast_node_list_entry *ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    switch (ast_current_node->node->node_type) {
    case CHA_AST_NODE_TYPE_FUNCTION_DECLARATION:
      if (insert_symbol_table(
              validate_symbol_table,
              ast_current_node->node->function_declaration.identifier,
              ast_current_node->node, NULL, NULL) != 0) {
        ret = 1;
        log_validation_error(
            ast_current_node->node->location, "'%s' already defined",
            ast_current_node->node->function_declaration.identifier);
      }
      break;
    case CHA_AST_NODE_TYPE_CONSTANT_DECLARATION:
      break; // validate later
    default:
      ret = 1;
      log_validation_error(ast_current_node->node->location,
                           "unexpected token");
      break;
    }

    ast_current_node = ast_current_node->next;
  }

  ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    if (cha_validate_node(ast_current_node->node) != 0) {
      ret = 1;
    }
    ast_current_node = ast_current_node->next;
  }
  return ret;
}

int cha_validate_node_list(cha_ast_node_list *ast) {
  if (ast == NULL) {
    return 0;
  }

  int ret = 0;
  cha_ast_node_list_entry *ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    if (cha_validate_node(ast_current_node->node) != 0) {
      ret = 1;
    }
    ast_current_node = ast_current_node->next;
  }
  return ret;
}

int cha_validate_node(cha_ast_node *ast_node) {
  if (ast_node == NULL) {
    return 0;
  }

  switch (ast_node->node_type) {
  case CHA_AST_NODE_TYPE_FUNCTION_DECLARATION:
    return cha_validate_node_fun(ast_node);
  case CHA_AST_NODE_TYPE_CONSTANT_DECLARATION:
    return cha_validate_node_const(ast_node);
  case CHA_AST_NODE_TYPE_BLOCK:
    return cha_validate_node_list(ast_node->block);
  case CHA_AST_NODE_TYPE_VARIABLE_DECLARATION:
    return cha_validate_node_var(ast_node);
  case CHA_AST_NODE_TYPE_BIN_OP:
    return cha_validate_node_bin_op(ast_node);
  case CHA_AST_NODE_TYPE_VARIABLE_ASSIGNMENT:
    return cha_validate_node_var_assign(ast_node);
  case CHA_AST_NODE_TYPE_VARIABLE_LOOKUP:
    return cha_validate_node_var_lookup(ast_node);
  case CHA_AST_NODE_TYPE_FUNCTION_CALL:
    return cha_validate_node_call(ast_node);
  case CHA_AST_NODE_TYPE_FUNCTION_RETURN:
    return cha_validate_node_ret(ast_node);
  case CHA_AST_NODE_TYPE_ARGUMENT:
    return cha_validate_node_arg(ast_node);
  case CHA_AST_NODE_TYPE_IF:
    return cha_validate_node_if(ast_node);
  default:
    return 0; // no validation
  }
}

int cha_validate_node_fun(cha_ast_node *ast_node) {
  cha_validate_create_stack_frame();
  fun = ast_node;

  int ret =
      cha_validate_node_list(ast_node->function_declaration.argument_list);

  if (cha_validate_node_list(ast_node->function_declaration.block) != 0) {
    ret = 1;
  }

  fun = NULL;

  cha_validate_release_stack_frame();
  return ret;
}

int cha_validate_node_var(cha_ast_node *ast_node) {
  if (cha_validate_node(ast_node->variable_declaration.value) != 0) {
    return 1;
  }
  if (insert_symbol_table(validate_symbol_table,
                          ast_node->variable_declaration.identifier, ast_node,
                          NULL, NULL) != 0) {
    log_validation_error(ast_node->location, "variable '%s' already defined",
                         ast_node->variable_declaration.identifier);
    return 1;
  }
  return 0;
}

int cha_validate_node_const(cha_ast_node *ast_node) {
  if (insert_symbol_table(validate_symbol_table,
                          ast_node->constant_declaration.identifier, ast_node,
                          NULL, NULL) != 0) {
    log_validation_error(ast_node->location, "constant '%s' already defined",
                         ast_node->constant_declaration.identifier);
    return 1;
  }
  return 0;
}

int cha_validate_node_arg(cha_ast_node *ast_node) {
  if (insert_symbol_table(validate_symbol_table, ast_node->argument.identifier,
                          ast_node, NULL, NULL) != 0) {
    log_validation_error(ast_node->location, "argument '%s' already defined",
                         ast_node->argument.identifier);
    return 1;
  }
  return 0;
}

int cha_validate_node_var_assign(cha_ast_node *ast_node) {
  symbol_value *v = get_symbol_table(validate_symbol_table,
                                     ast_node->variable_assignment.identifier);
  if (v == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_assignment.identifier);
    return 1;
  }

  if (cha_validate_node(ast_node->variable_assignment.value) != 0) {
    return 1;
  }

  if (cha_check_type_assignment(ast_node->variable_assignment.value,
                                v->node->variable_declaration.type) != 0) {
    char expected_type[TYPE_STR_LEN];
    char got_type[TYPE_STR_LEN];
    type_str(expected_type, v->node->variable_declaration.type);
    type_str(got_type, ast_node->variable_assignment.value->_result_type);
    log_validation_error(ast_node->location,
                         "type mismatch expects '%s' passed '%s'",
                         expected_type, got_type);
    return 1;
  }

  return 0;
}

int cha_validate_node_var_lookup(cha_ast_node *ast_node) {
  symbol_value *v = get_symbol_table(validate_symbol_table,
                                     ast_node->variable_lookup.identifier);
  if (v == NULL) {
    log_validation_error(ast_node->location, "'%s' not found",
                         ast_node->variable_lookup.identifier);
    return 1;
  }
  switch (v->node->node_type) {
  case CHA_AST_NODE_TYPE_CONSTANT_DECLARATION: // replace lookup with value
    free(ast_node->variable_lookup.identifier);
    ast_node->node_type = v->node->constant_declaration.value->node_type;
    ast_node->_result_type = make_cha_ast_type(
        ast_node->location,
        v->node->constant_declaration.value->_result_type->internal_type);
    switch (ast_node->node_type) {
    case CHA_AST_NODE_TYPE_CONSTANT_UINT:
    case CHA_AST_NODE_TYPE_CONSTANT_INT:
    case CHA_AST_NODE_TYPE_CONSTANT_FLOAT:
      ast_node->const_value =
          strdup(v->node->constant_declaration.value->const_value);
      break;
    case CHA_AST_NODE_TYPE_CONSTANT_BOOL:
      ast_node->const_bool = v->node->constant_declaration.value->const_bool;
      break;
    default:
      // other types are irrelevant
      break;
    }
    break;
  case CHA_AST_NODE_TYPE_VARIABLE_DECLARATION: // copy type
    ast_node->_result_type = make_cha_ast_type(
        ast_node->location, v->node->variable_declaration.type->internal_type);
    break;
  default:
    log_validation_error(ast_node->location, "incompatible element found");
    return 1;
  }
  return 0;
}

int cha_validate_node_bin_op(cha_ast_node *ast_node) {
  int ret_left = cha_validate_node(ast_node->bin_op.left);
  int ret_right = cha_validate_node(ast_node->bin_op.right);
  if (ret_left != 0 || ret_right != 0) {
    return 1;
  }

  switch (ast_node->bin_op.op) {
  case CHA_AST_OPERATOR_ADD:      // +
  case CHA_AST_OPERATOR_SUBTRACT: // -
  case CHA_AST_OPERATOR_MULTIPLY: // *
  case CHA_AST_OPERATOR_DIVIDE:   // /
    ast_node->_result_type = make_cha_ast_type(
        ast_node->location,
        cha_convert_arithmetic_types(
            ast_node->bin_op.left->_result_type->internal_type,
            ast_node->bin_op.right->_result_type->internal_type));
    cha_set_type_on_const(ast_node->bin_op.left,
                          ast_node->_result_type->internal_type);
    cha_set_type_on_const(ast_node->bin_op.right,
                          ast_node->_result_type->internal_type);

    break;
  case CHA_AST_OPERATOR_GREATER_THAN:           // >
  case CHA_AST_OPERATOR_GREATER_THAN_OR_EQUALS: // >=
  case CHA_AST_OPERATOR_LESS_THAN:              // <
  case CHA_AST_OPERATOR_LESS_THAN_OR_EQUALS:    // <=
    if (cha_check_numeric_comparison_compatibility(
            ast_node->bin_op.left->_result_type->internal_type,
            ast_node->bin_op.right->_result_type->internal_type) == 1) {
      ast_node->_result_type =
          make_cha_ast_type(ast_node->location, CHA_AST_INTERNAL_TYPE_UNDEF);
    } else {
      ast_node->_result_type =
          make_cha_ast_type(ast_node->location, CHA_AST_INTERNAL_TYPE_BOOL);
    }
    cha_set_type_on_const(ast_node->bin_op.left,
                          ast_node->bin_op.right->_result_type->internal_type);
    cha_set_type_on_const(ast_node->bin_op.right,
                          ast_node->bin_op.left->_result_type->internal_type);
    break;
  case CHA_AST_OPERATOR_EQUALS_EQUALS: // ==
  case CHA_AST_OPERATOR_NOT_EQUALS:    // !=
    if (cha_check_equality_comparison_compatibility(
            ast_node->bin_op.left->_result_type->internal_type,
            ast_node->bin_op.right->_result_type->internal_type) == 1) {
      ast_node->_result_type =
          make_cha_ast_type(ast_node->location, CHA_AST_INTERNAL_TYPE_UNDEF);
    } else {
      ast_node->_result_type =
          make_cha_ast_type(ast_node->location, CHA_AST_INTERNAL_TYPE_BOOL);
    }
    cha_set_type_on_const(ast_node->bin_op.left,
                          ast_node->bin_op.right->_result_type->internal_type);
    cha_set_type_on_const(ast_node->bin_op.right,
                          ast_node->bin_op.left->_result_type->internal_type);
    break;
  case CHA_AST_OPERATOR_AND: // &&
  case CHA_AST_OPERATOR_OR:  // ||
    // Boolean operators require both operands to be boolean
    if (ast_node->bin_op.left->_result_type->internal_type !=
            CHA_AST_INTERNAL_TYPE_BOOL ||
        ast_node->bin_op.right->_result_type->internal_type !=
            CHA_AST_INTERNAL_TYPE_BOOL) {
      ast_node->_result_type =
          make_cha_ast_type(ast_node->location, CHA_AST_INTERNAL_TYPE_UNDEF);
    } else {
      ast_node->_result_type =
          make_cha_ast_type(ast_node->location, CHA_AST_INTERNAL_TYPE_BOOL);
    }
    break;
  }
  if (ast_node->_result_type->internal_type == CHA_AST_INTERNAL_TYPE_UNDEF) {
    char ltype[TYPE_STR_LEN];
    char rtype[TYPE_STR_LEN];
    type_str(ltype, ast_node->bin_op.left->_result_type);
    type_str(rtype, ast_node->bin_op.right->_result_type);

    log_validation_error(ast_node->location,
                         "incompatible types found for operation: '%s', '%s'",
                         ltype, rtype);

    return 1;
  }

  return 0;
}

int cha_validate_node_call(cha_ast_node *ast_node) {
  symbol_value *callee_fun = get_symbol_table(
      validate_symbol_table, ast_node->function_call.identifier);

  if (callee_fun == NULL) {
    log_validation_error(ast_node->location, "function '%s' not found",
                         ast_node->function_call.identifier);
    return 1;
  }

  if (callee_fun->node->function_declaration.return_type != NULL) {
    ast_node->_result_type = make_cha_ast_type(
        ast_node->location,
        callee_fun->node->function_declaration.return_type->internal_type);
  }

  if ((ast_node->function_call.argument_list == NULL ||
       ast_node->function_call.argument_list->count == 0) &&
      callee_fun->node->function_declaration.argument_list != NULL &&
      callee_fun->node->function_declaration.argument_list->count > 0) {
    log_validation_error(ast_node->location, "function '%s' expects arguments",
                         ast_node->function_call.identifier);
    return 1;
  }

  if (ast_node->function_call.argument_list == NULL ||
      ast_node->function_call.argument_list->count == 0) {
    return 0;
  }

  if (callee_fun->node->function_declaration.argument_list == NULL) {
    log_validation_error(ast_node->location,
                         "function '%s' expects no arguments",
                         ast_node->function_call.identifier);
    return 1;
  }

  if (ast_node->function_call.argument_list->count !=
      callee_fun->node->function_declaration.argument_list->count) {
    log_validation_error(
        ast_node->location,
        "function '%s' expects %d arguments but %d were passed",
        ast_node->function_call.identifier,
        fun->function_declaration.argument_list->count,
        ast_node->function_call.argument_list->count);
    return 1;
  }

  cha_ast_node_list_entry *arg = ast_node->function_call.argument_list->head;
  cha_ast_node_list_entry *def_arg =
      callee_fun->node->function_declaration.argument_list->head;
  int ret = 0;
  while (arg != NULL) {
    if (cha_validate_node(arg->node) != 0) {
      ret = 1;
    }
    if (cha_check_type_assignment(arg->node, def_arg->node->argument.type) !=
        0) {
      char expected_type[TYPE_STR_LEN];
      char got_type[TYPE_STR_LEN];
      type_str(expected_type, def_arg->node->argument.type);
      type_str(got_type, arg->node->_result_type);
      log_validation_error(arg->node->location,
                           "type mismatch expects '%s' passed '%s'",
                           expected_type, got_type);
      ret = 1;
    }

    arg = arg->next;
    def_arg = def_arg->next;
  }
  return ret;
}

int cha_validate_node_ret(cha_ast_node *ast_node) {
  if (cha_validate_node(ast_node->function_return.value) != 0) {
    return 1;
  }

  if (ast_node->function_return.value == NULL &&
      fun->function_declaration.return_type == NULL) {
    return 0;
  }

  if (ast_node->function_return.value == NULL &&
      fun->function_declaration.return_type != NULL) {
    log_validation_error(ast_node->location, "return value expected");
    return 1;
  }

  if (ast_node->function_return.value != NULL &&
      fun->function_declaration.return_type == NULL) {
    log_validation_error(ast_node->location, "return value not expected");
    return 1;
  }

  if (cha_check_type_assignment(ast_node->function_return.value,
                                fun->function_declaration.return_type) != 0) {
    char expected_type[TYPE_STR_LEN];
    char got_type[TYPE_STR_LEN];
    type_str(expected_type, fun->function_declaration.return_type);
    type_str(got_type, ast_node->function_return.value->_result_type);
    log_validation_error(ast_node->location,
                         "return type mismatch expects '%s' passed '%s'",
                         expected_type, got_type);
    return 1;
  }

  return 0;
}

int cha_validate_node_if(cha_ast_node *ast_node) {
  if (cha_validate_node(ast_node->if_block.condition) != 0) {
    return 1;
  }
  if (ast_node->if_block.condition->_result_type->internal_type !=
      CHA_AST_INTERNAL_TYPE_BOOL) {
    log_validation_error(ast_node->if_block.condition->location,
                         "condition should return bool");
    return 1;
  }
  cha_validate_create_stack_frame();
  if (cha_validate_node_list(ast_node->if_block.block) != 0) {
    cha_validate_release_stack_frame();
    return 1;
  }
  cha_validate_release_stack_frame();
  cha_validate_create_stack_frame();
  if (cha_validate_node_list(ast_node->if_block.else_block) != 0) {
    cha_validate_release_stack_frame();
    return 1;
  }
  cha_validate_release_stack_frame();
  return 0;
}

void type_str(char *out, const cha_ast_type *ast_type) {
  if (out == NULL) {
    return;
  }

  if (ast_type == NULL) {
    sprintf(out, "void");
  } else {
    switch (ast_type->internal_type) {
    case CHA_AST_INTERNAL_TYPE_UNDEF:
      sprintf(out, "undefined");
      break;
    case CHA_AST_INTERNAL_TYPE_CONST_INT:
      sprintf(out, "c_int");
      break;
    case CHA_AST_INTERNAL_TYPE_CONST_UINT:
      sprintf(out, "c_uint");
      break;
    case CHA_AST_INTERNAL_TYPE_INT8:
      sprintf(out, "int8");
      break;
    case CHA_AST_INTERNAL_TYPE_UINT8:
      sprintf(out, "uint8");
      break;
    case CHA_AST_INTERNAL_TYPE_INT16:
      sprintf(out, "int16");
      break;
    case CHA_AST_INTERNAL_TYPE_UINT16:
      sprintf(out, "uint16");
      break;
    case CHA_AST_INTERNAL_TYPE_INT32:
      sprintf(out, "int32");
      break;
    case CHA_AST_INTERNAL_TYPE_UINT32:
      sprintf(out, "uint32");
      break;
    case CHA_AST_INTERNAL_TYPE_INT64:
      sprintf(out, "int64");
      break;
    case CHA_AST_INTERNAL_TYPE_UINT64:
      sprintf(out, "uint64");
      break;
    case CHA_AST_INTERNAL_TYPE_CONST_FLOAT:
      sprintf(out, "float");
      break;
    case CHA_AST_INTERNAL_TYPE_FLOAT16:
      sprintf(out, "float16");
      break;
    case CHA_AST_INTERNAL_TYPE_FLOAT32:
      sprintf(out, "float32");
      break;
    case CHA_AST_INTERNAL_TYPE_FLOAT64:
      sprintf(out, "float64");
      break;
    case CHA_AST_INTERNAL_TYPE_INT:
      sprintf(out, "int");
      break;
    case CHA_AST_INTERNAL_TYPE_UINT:
      sprintf(out, "uint");
      break;
    case CHA_AST_INTERNAL_TYPE_BOOL:
      sprintf(out, "bool");
      break;
    }
  }
}

int cha_check_type_assignment(cha_ast_node *ast_node,
                              const cha_ast_type *ast_type) {

  if (cha_check_assignment_compatibility(ast_node->_result_type->internal_type,
                                         ast_type->internal_type) != 0) {
    return 1;
  }
  cha_set_type_on_const(ast_node, ast_type->internal_type);

  return 0;
}

void cha_set_type_on_const(cha_ast_node *ast_node,
                           cha_ast_internal_type internal_type) {
  if (ast_node->_result_type->internal_type ==
          CHA_AST_INTERNAL_TYPE_CONST_INT ||
      ast_node->_result_type->internal_type ==
          CHA_AST_INTERNAL_TYPE_CONST_UINT ||
      ast_node->_result_type->internal_type ==
          CHA_AST_INTERNAL_TYPE_CONST_FLOAT) {
    ast_node->_result_type =
        make_cha_ast_type(ast_node->location, internal_type);
  }
}

void cha_validate_create_stack_frame() {
  symbol_table *new_table =
      make_symbol_table(SYMBOL_TABLE_SIZE, validate_symbol_table);
  validate_symbol_table = new_table;
}

void cha_validate_release_stack_frame() {
  symbol_table *new_table = validate_symbol_table;
  validate_symbol_table = validate_symbol_table->parent;
  free_symbol_table(new_table);
}
