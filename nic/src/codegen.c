#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>

#include "nic/ast.h"
#include "nic/codegen.h"
#include "symbol_table.h"

void initialize_modules();
void free_modules();
void log_error(const char *format, ...);
int ni_ast_codegen_toplevel(ni_ast_node_list *ast);
int ni_ast_codegen_node(ni_ast_node *ast_node);
int ni_ast_codegen_node_constant_int(ni_ast_node *ast_node);
int ni_ast_codegen_node_bin_op(ni_ast_node *ast_node);
int ni_ast_codegen_node_var(ni_ast_node *ast_node);
int ni_ast_codegen_node_var_assign(ni_ast_node *ast_node);
int ni_ast_codegen_node_var_lookup(ni_ast_node *ast_node);
int ni_ast_codegen_node_ret(ni_ast_node *ast_node);
int ni_ast_codegen_node_call(ni_ast_node *ast_node);
int ni_ast_codegen_node_fun(ni_ast_node *ast_node);
int ni_ast_codegen_block(ni_ast_node_list *block);
LLVMTypeRef make_fun_signature(ni_ast_node *ast_node);

LLVMContextRef context = NULL;
LLVMModuleRef module = NULL;
LLVMBuilderRef builder = NULL;
LLVMValueRef return_operand = NULL;
symbol_table *var_table = NULL;
symbol_table *fn_table = NULL;

int ni_ast_codegen_node(ni_ast_node *ast_node) {
  return_operand = NULL;
  if (ast_node == NULL) {
    return 0;
  }
  switch (ast_node->type) {
  case NI_AST_TYPE_INT_CONSTANT:
    return ni_ast_codegen_node_constant_int(ast_node);
  case NI_AST_TYPE_BIN_OP:
    return ni_ast_codegen_node_bin_op(ast_node);
  case NI_AST_TYPE_VARIABLE_DECLARATION:
    return ni_ast_codegen_node_var(ast_node);
  case NI_AST_TYPE_VARIABLE_ASSIGNMENT:
    return ni_ast_codegen_node_var_assign(ast_node);
  case NI_AST_TYPE_VARIABLE_LOOKUP:
    return ni_ast_codegen_node_var_lookup(ast_node);
  case NI_AST_TYPE_FUNCTION_CALL:
    return ni_ast_codegen_node_call(ast_node);
  case NI_AST_TYPE_FUNCTION_RETURN:
    return ni_ast_codegen_node_ret(ast_node);
  case NI_AST_TYPE_FUNCTION_DECLARATION:
    return ni_ast_codegen_node_fun(ast_node);
  case NI_AST_TYPE_BLOCK:
    return ni_ast_codegen_block(ast_node->block);
  case NI_AST_TYPE_INT_TYPE:
  case NI_AST_TYPE_ARGUMENT:
    // DO NOTHING
    break;
  }

  return 0;
}

int ni_ast_codegen_block(ni_ast_node_list *block) {
  ni_ast_node_list_entry *cur = block->head;
  while (cur != NULL) {
    int ret = ni_ast_codegen_node(cur->node);
    if (ret != 0) {
      return ret;
    }
    cur = cur->next;
  }
  return 0;
}

int ni_ast_codegen_toplevel(ni_ast_node_list *ast) {
  ni_ast_node_list_entry *cur = ast->head;
  while (cur != NULL) {
    int ret = ni_ast_codegen_node(cur->node);
    if (ret != 0) {
      return ret;
    }
    cur = cur->next;
  }
  return 0;
}

int ni_ast_codegen_node_constant_int(ni_ast_node *ast_node) {
  long long value = strtoll(ast_node->int_const.value, NULL, 10);
  return_operand = LLVMConstInt(LLVMInt32Type(), value, 1);
  return 0;
}

int ni_ast_codegen_node_bin_op(ni_ast_node *ast_node) {
  int ret = ni_ast_codegen_node(ast_node->bin_op.left);
  if (ret != 0) {
    return ret;
  }
  LLVMValueRef left_operand = return_operand;
  ret = ni_ast_codegen_node(ast_node->bin_op.right);
  if (ret != 0) {
    return ret;
  }
  LLVMValueRef right_operand = return_operand;

  switch (ast_node->bin_op.op) {
  case NI_AST_OPERATOR_PLUS:
    return_operand = LLVMBuildAdd(builder, left_operand, right_operand, "add");
    break;
  case NI_AST_OPERATOR_MINUS:
    return_operand = LLVMBuildSub(builder, left_operand, right_operand, "sub");
    break;
  case NI_AST_OPERATOR_MULTIPLY:
    return_operand = LLVMBuildMul(builder, left_operand, right_operand, "mul");
    break;
  }

  return 0;
}

int ni_ast_codegen_node_var(ni_ast_node *ast_node) {
  LLVMTypeRef type = LLVMInt32Type();

  LLVMValueRef addr =
      LLVMBuildAlloca(builder, type, ast_node->variable_declaration.identifier);

  symbol_value *value = malloc(sizeof(symbol_value));
  value->type = type;
  value->ref = addr;
  value->node = ast_node;

  insert_symbol_table(var_table, ast_node->variable_declaration.identifier,
                      value);
  return 0;
}

int ni_ast_codegen_node_var_assign(ni_ast_node *ast_node) {
  symbol_value *value =
      get_symbol_table(var_table, ast_node->variable_lookup.identifier);
  if (value == NULL) {
    log_error("variable '%s' not found", ast_node->variable_lookup.identifier);
    return 1;
  }

  int ret = ni_ast_codegen_node(ast_node->variable_assignment.value);
  if (ret != 0) {
    return ret;
  }
  LLVMBuildStore(builder, return_operand, value->ref);
  return 0;
}

int ni_ast_codegen_node_var_lookup(ni_ast_node *ast_node) {
  symbol_value *value =
      get_symbol_table(var_table, ast_node->variable_lookup.identifier);
  if (value == NULL) {
    log_error("variable '%s' not found", ast_node->variable_lookup.identifier);
    return 1;
  }

  return_operand = LLVMBuildLoad2(builder, value->type, value->ref,
                                  ast_node->variable_lookup.identifier);
  return 0;
}

int ni_ast_codegen_node_call(ni_ast_node *ast_node) {
  LLVMValueRef *args = NULL;
  int arg_count = 0;
  if (ast_node->function_call.argument_list != NULL) {
    arg_count = ast_node->function_call.argument_list->count;
    args = malloc(sizeof(LLVMValueRef) * arg_count);

    ni_ast_node_list_entry *current =
        ast_node->function_call.argument_list->head;
    int i = 0;
    while (current != NULL) {
      int ret = ni_ast_codegen_node(current->node);
      if (ret != 0) {
        free(args);
        return ret;
      }
      args[i] = return_operand;
      current = current->next;
      i++;
    }
  }

  symbol_value *function =
      get_symbol_table(fn_table, ast_node->function_call.identifier);
  if (function == NULL) {
    free(args);
    log_error("function '%s' not found", ast_node->function_call.identifier);
    return 1;
  }

  return_operand =
      LLVMBuildCall2(builder, function->type, function->ref, args, arg_count,
                     ast_node->function_call.identifier);

  free(args);

  return 0;
}

int ni_ast_codegen_node_ret(ni_ast_node *ast_node) {
  log_error("return not implemented");
  return 1;
}

int ni_ast_codegen_node_fun(ni_ast_node *ast_node) {
  var_table = make_symbol_table(101);

  LLVMTypeRef fn_type = make_fun_signature(ast_node);

  LLVMValueRef function = LLVMAddFunction(
      module, ast_node->function_declaration.identifier, fn_type);

  symbol_value *entry = malloc(sizeof(symbol_value));
  entry->node = ast_node;
  entry->ref = function;
  entry->type = fn_type;

  insert_symbol_table(fn_table, ast_node->function_declaration.identifier,
                      entry);

  if (ast_node->function_declaration.argument_list != NULL) {
    LLVMBasicBlockRef args_block = LLVMAppendBasicBlock(function, "args");
    LLVMPositionBuilderAtEnd(builder, args_block);

    ni_ast_node_list_entry *current =
        ast_node->function_declaration.argument_list->head;
    int i = 0;
    while (current != NULL) {
      LLVMTypeRef type = LLVMInt32Type();
      LLVMValueRef addr =
          LLVMBuildAlloca(builder, type, current->node->argument.identifier);
      LLVMBuildStore(builder, LLVMGetParam(function, i), addr);

      symbol_value *value = malloc(sizeof(symbol_value));
      value->type = type;
      value->ref = addr;
      value->node = current->node;

      insert_symbol_table(var_table, current->node->argument.identifier, value);

      current = current->next;
      i++;
    }
  }

  LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function, "entry");
  LLVMPositionBuilderAtEnd(builder, entry_block);

  int ret = ni_ast_codegen_block(ast_node->function_declaration.block);
  free_symbol_table(var_table);

  return ret;
}

int ni_ast_codegen(ni_ast_node_list *ast, enum ni_ast_codegen_format format,
                   char *file) {
  initialize_modules();

  int ret = ni_ast_codegen_toplevel(ast);
  if (ret != 0) {
    free_modules();
    return ret;
  }

  char *errors = NULL;
  if (LLVMPrintModuleToFile(module, file, &errors) == 1) {
    log_error("codegen error: %s\n", errors);
    LLVMDisposeMessage(errors);
    free_modules();
    return 1;
  }

  free_modules();
  return 0;
}

LLVMTypeRef make_fun_signature(ni_ast_node *ast_node) {
  LLVMTypeRef *arg_types = NULL;
  int arg_count = 0;
  if (ast_node->function_declaration.argument_list != NULL) {
    arg_count = ast_node->function_declaration.argument_list->count;
  }
  if (arg_count > 0) {
    arg_types = malloc(sizeof(LLVMTypeRef) * arg_count);
    ni_ast_node_list_entry *current =
        ast_node->function_declaration.argument_list->head;
    int i = 0;
    while (current != NULL) {
      arg_types[i] = LLVMInt32Type();
      current = current->next;
      i++;
    }
  }

  LLVMTypeRef return_type = LLVMVoidType();
  if (ast_node->function_declaration.return_type != NULL) {
    return_type = LLVMInt32Type();
  }

  return LLVMFunctionType(return_type, arg_types, arg_count, 0);
}

void initialize_modules() {
  fn_table = make_symbol_table(101);
  context = LLVMContextCreate();
  module = LLVMModuleCreateWithNameInContext("nic", context);
  builder = LLVMCreateBuilderInContext(context);
}

void free_modules() {
  free_symbol_table(fn_table);
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMContextDispose(context);
}

void log_error(const char *format, ...) {
  fprintf(stderr, "codegen error: ");

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}