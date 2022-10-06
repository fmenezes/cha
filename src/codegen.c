#include <stdlib.h>
#include <string.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>

#include "log.h"
#include "ni/ast.h"
#include "ni/codegen.h"
#include "symbol_table.h"

int initialize_modules(const char *module_id);
void free_modules();
int ni_ast_codegen_toplevel(ni_ast_node_list *ast);
int ni_ast_codegen_node(ni_ast_node *ast_node);
int ni_ast_codegen_node_constant_number(ni_ast_node *ast_node);
int ni_ast_codegen_node_constant_float(ni_ast_node *ast_node);
int ni_ast_codegen_node_bin_op(ni_ast_node *ast_node);
int ni_ast_codegen_node_var(ni_ast_node *ast_node);
int ni_ast_codegen_node_var_assign(ni_ast_node *ast_node);
int ni_ast_codegen_node_var_lookup(ni_ast_node *ast_node);
int ni_ast_codegen_node_ret(ni_ast_node *ast_node);
int ni_ast_codegen_node_call(ni_ast_node *ast_node);
int ni_ast_codegen_node_fun(ni_ast_node *ast_node);
int ni_ast_codegen_block(ni_ast_node_list *block);
LLVMTypeRef make_fun_signature(ni_ast_node *ast_node);
LLVMTypeRef make_type(ni_ast_node *ast_node);
LLVMBool signed_type(ni_ast_node *ast_node);

LLVMContextRef context = NULL;
LLVMModuleRef module = NULL;
LLVMBuilderRef builder = NULL;
LLVMValueRef return_operand = NULL;
LLVMTargetMachineRef target_machine = NULL;
LLVMTargetDataRef target_data_layout = NULL;
char *target_triple = NULL;
symbol_table *var_table = NULL;
symbol_table *fn_table = NULL;

int ni_ast_codegen_node(ni_ast_node *ast_node) {
  return_operand = NULL;
  if (ast_node == NULL) {
    return 0;
  }
  switch (ast_node->type) {
  case NI_AST_TYPE_CONSTANT_NUMBER:
    return ni_ast_codegen_node_constant_number(ast_node);
  case NI_AST_TYPE_CONSTANT_FLOAT:
    return ni_ast_codegen_node_constant_float(ast_node);
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
  case NI_AST_TYPE_ARGUMENT:
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
  case NI_AST_TYPE_REFTYPE_FLOAT:
  case NI_AST_TYPE_REFTYPE_DOUBLE:
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

int ni_ast_codegen_node_constant_number(ni_ast_node *ast_node) {
  unsigned char radix;
  if (strncmp("0x", ast_node->const_value, 2) == 0 ||
      strncmp("0X", ast_node->const_value, 2) == 0) {
    radix = 16;
  } else {
    radix = 10;
  }

  return_operand = LLVMConstIntOfString(LLVMInt32TypeInContext(context),
                                        ast_node->const_value, radix);
  return 0;
}

int ni_ast_codegen_node_constant_float(ni_ast_node *ast_node) {
  return_operand = LLVMConstRealOfString(LLVMFloatTypeInContext(context), ast_node->const_value);
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
  LLVMTypeRef type = make_type(ast_node->argument.type);

  LLVMValueRef addr =
      LLVMBuildAlloca(builder, type, ast_node->variable_declaration.identifier);

  return insert_symbol_table(var_table,
                             ast_node->variable_declaration.identifier,
                             ast_node, addr, type);
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
  if (ast_node->function_return.value != NULL) {
    int ret = ni_ast_codegen_node(ast_node->function_return.value);
    if (ret != 0) {
      return ret;
    }
    LLVMBuildRet(builder, return_operand);
  } else {
    LLVMBuildRetVoid(builder);
  }
  return 0;
}

int ni_ast_codegen_node_fun(ni_ast_node *ast_node) {
  var_table = make_symbol_table(SYMBOL_TABLE_SIZE);

  LLVMTypeRef fn_type = make_fun_signature(ast_node);

  LLVMValueRef function = LLVMAddFunction(
      module, ast_node->function_declaration.identifier, fn_type);

  insert_symbol_table(fn_table, ast_node->function_declaration.identifier,
                      ast_node, function, fn_type);

  LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function, "entry");
  LLVMPositionBuilderAtEnd(builder, entry_block);

  if (ast_node->function_declaration.argument_list != NULL) {
    ni_ast_node_list_entry *current =
        ast_node->function_declaration.argument_list->head;
    int i = 0;
    while (current != NULL) {
      LLVMTypeRef type = LLVMInt32TypeInContext(context);
      LLVMValueRef addr =
          LLVMBuildAlloca(builder, type, current->node->argument.identifier);
      LLVMBuildStore(builder, LLVMGetParam(function, i), addr);

      insert_symbol_table(var_table, current->node->argument.identifier,
                          current->node, addr, type);

      current = current->next;
      i++;
    }
  }

  int ret = ni_ast_codegen_block(ast_node->function_declaration.block);
  free_symbol_table(var_table);

  if (LLVMVerifyFunction(function, LLVMPrintMessageAction) != 0) {
    ret = 1;
  }

  return ret;
}

int ni_ast_codegen(ni_ast_node_list *ast, enum ni_ast_codegen_format format,
                   const char *file_path) {
  int ret = initialize_modules("nic");
  if (ret != 0) {
    return ret;
  }

  ret = ni_ast_codegen_toplevel(ast);
  if (ret != 0) {
    free_modules();
    return ret;
  }

  char *errors = NULL;
  if (LLVMVerifyModule(module, LLVMReturnStatusAction, &errors) != 0) {
    log_error(errors);
    LLVMDisposeMessage(errors);
    free_modules();
    return 1;
  }
  LLVMDisposeMessage(errors);
  errors = NULL;

  if (format == NI_CODEGEN_FORMAT_LLVM_IR) {
    if (LLVMPrintModuleToFile(module, file_path, &errors) != 0) {
      log_error(errors);
      LLVMDisposeMessage(errors);
      free_modules();
      return 1;
    }
    LLVMDisposeMessage(errors);
    errors = NULL;
  } else {
    LLVMCodeGenFileType gen_type = LLVMObjectFile;
    if (format == NI_CODEGEN_FORMAT_ASSEMBLY_FILE) {
      gen_type = LLVMAssemblyFile;
    }
    char *obj_file_path = (char *)file_path;
    if (format == NI_CODEGEN_FORMAT_BINARY_FILE) {
      obj_file_path = malloc(strlen(file_path) + 3);
      sprintf(obj_file_path, "%s.o", file_path);
    }
    if (LLVMTargetMachineEmitToFile(target_machine, module, obj_file_path,
                                    gen_type, &errors) != 0) {
      if (format == NI_CODEGEN_FORMAT_BINARY_FILE) {
        free(obj_file_path);
      }
      log_error(errors);
      LLVMDisposeMessage(errors);
      free_modules();
      return 1;
    }
    LLVMDisposeMessage(errors);
    errors = NULL;

    if (format == NI_CODEGEN_FORMAT_BINARY_FILE) {
      free_modules();
      char cmd[5000];
      sprintf(cmd, "cc -o %s %s", file_path, obj_file_path);
      ret = system(cmd);
      remove(obj_file_path);
      free(obj_file_path);
      return ret;
    }
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
      LLVMTypeRef arg_type = make_type(current->node->argument.type);
      arg_types[i] = arg_type;
      current = current->next;
      i++;
    }
  }

  LLVMTypeRef return_type =
      make_type(ast_node->function_declaration.return_type);

  LLVMTypeRef ret = LLVMFunctionType(return_type, arg_types, arg_count, 0);

  free(arg_types);

  return ret;
}

int initialize_modules(const char *module_id) {
  LLVMInitializeAArch64TargetInfo();
  LLVMInitializeAArch64Target();
  LLVMInitializeAArch64TargetMC();
  LLVMInitializeAArch64AsmPrinter();

  LLVMInitializeX86TargetInfo();
  LLVMInitializeX86Target();
  LLVMInitializeX86TargetMC();
  LLVMInitializeX86AsmPrinter();

  LLVMInitializeARMTargetInfo();
  LLVMInitializeARMTarget();
  LLVMInitializeARMTargetMC();
  LLVMInitializeARMAsmPrinter();

  target_triple = LLVMGetDefaultTargetTriple();
  char cpu[] = "generic";
  char features[] = "";

  char *error = NULL;

  LLVMTargetRef target;
  if (LLVMGetTargetFromTriple(target_triple, &target, &error) != 0) {
    log_error(error);
    LLVMDisposeMessage(error);
    return 1;
  }

  context = LLVMContextCreate();
  module = LLVMModuleCreateWithNameInContext(module_id, context);
  builder = LLVMCreateBuilderInContext(context);

  LLVMSetTarget(module, target_triple);
  target_machine = LLVMCreateTargetMachine(
      target, target_triple, cpu, features, LLVMCodeGenLevelDefault,
      LLVMRelocDefault, LLVMCodeModelDefault);
  target_data_layout = LLVMCreateTargetDataLayout(target_machine);
  LLVMSetModuleDataLayout(module, target_data_layout);
  fn_table = make_symbol_table(SYMBOL_TABLE_SIZE);

  return 0;
}

void free_modules() {
  LLVMDisposeTargetMachine(target_machine);
  LLVMDisposeTargetData(target_data_layout);
  LLVMDisposeMessage(target_triple);
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMContextDispose(context);
  free_symbol_table(fn_table);
}

LLVMBool signed_type(ni_ast_node *ast_node) {
  if (ast_node == NULL) {
    return 0;
  }

  switch (ast_node->type) {
  case NI_AST_TYPE_REFTYPE_BYTE:
  case NI_AST_TYPE_REFTYPE_USHORT:
  case NI_AST_TYPE_REFTYPE_ULONG:
  case NI_AST_TYPE_REFTYPE_ULARGE:
  case NI_AST_TYPE_REFTYPE_UINT:
    return 0;
  }

  return 1;
}

LLVMTypeRef make_type(ni_ast_node *ast_node) {
  if (ast_node == NULL) {
    return LLVMVoidTypeInContext(context);
  }

  switch (ast_node->type) {
  case NI_AST_TYPE_REFTYPE_BYTE:
  case NI_AST_TYPE_REFTYPE_SBYTE:
    return LLVMInt8TypeInContext(context);
  case NI_AST_TYPE_REFTYPE_INT:
  case NI_AST_TYPE_REFTYPE_UINT:
    return LLVMInt32TypeInContext(context);
  case NI_AST_TYPE_REFTYPE_SHORT:
  case NI_AST_TYPE_REFTYPE_USHORT:
    return LLVMInt16TypeInContext(context);
  case NI_AST_TYPE_REFTYPE_LONG:
  case NI_AST_TYPE_REFTYPE_ULONG:
    return LLVMInt64TypeInContext(context);
  case NI_AST_TYPE_REFTYPE_LARGE:
  case NI_AST_TYPE_REFTYPE_ULARGE:
    return LLVMInt128TypeInContext(context);
  case NI_AST_TYPE_REFTYPE_SFLOAT:
    return LLVMHalfTypeInContext(context);
  case NI_AST_TYPE_REFTYPE_FLOAT:
    return LLVMFloatTypeInContext(context);
  case NI_AST_TYPE_REFTYPE_DOUBLE:
    return LLVMDoubleTypeInContext(context);
  }

  return LLVMVoidTypeInContext(context);
}
