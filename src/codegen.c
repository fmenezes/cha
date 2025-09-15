#include <stdlib.h>
#include <string.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>

#include "ast.h"
#include "codegen.h"
#include "log.h"
#include "symbol_table.h"

int initialize_modules(const char *module_id);
void free_modules();
int cha_ast_codegen_toplevel(cha_ast_node_list *ast);
int cha_ast_codegen_node(cha_ast_node *ast_node);
int cha_ast_codegen_node_constant_number(cha_ast_node *ast_node);
int cha_ast_codegen_node_constant_float(cha_ast_node *ast_node);
int cha_ast_codegen_node_constant_bool(cha_ast_node *ast_node);
int cha_ast_codegen_node_bin_op(cha_ast_node *ast_node);
int cha_ast_codegen_node_var(cha_ast_node *ast_node);
int cha_ast_codegen_node_var_assign(cha_ast_node *ast_node);
int cha_ast_codegen_node_var_lookup(cha_ast_node *ast_node);
int cha_ast_codegen_node_ret(cha_ast_node *ast_node);
int cha_ast_codegen_node_call(cha_ast_node *ast_node);
int cha_ast_codegen_node_fun(cha_ast_node *ast_node);
int cha_ast_codegen_block(cha_ast_node_list *block);
int cha_ast_codegen_if(cha_ast_node *ast_node);
LLVMTypeRef make_fun_signature(cha_ast_node *ast_node);
LLVMTypeRef make_type(cha_ast_type *ast_type);
int signed_type(const cha_ast_type *ast_type);
int float_type(const cha_ast_type *ast_type);
void cha_create_stack_frame();
void cha_release_stack_frame();

LLVMContextRef context = NULL;
LLVMModuleRef module = NULL;
LLVMBuilderRef builder = NULL;
LLVMValueRef return_operand = NULL;
LLVMTargetMachineRef target_machine = NULL;
LLVMTargetDataRef target_data_layout = NULL;
char *target_triple = NULL;
symbol_table *codegen_symbol_table = NULL;

int cha_ast_codegen_node(cha_ast_node *ast_node) {
  return_operand = NULL;
  if (ast_node == NULL) {
    return 0;
  }
  switch (ast_node->node_type) {
  case CHA_AST_NODE_TYPE_CONSTANT_INT:
  case CHA_AST_NODE_TYPE_CONSTANT_UINT:
    return cha_ast_codegen_node_constant_number(ast_node);
  case CHA_AST_NODE_TYPE_CONSTANT_FLOAT:
    return cha_ast_codegen_node_constant_float(ast_node);
  case CHA_AST_NODE_TYPE_CONSTANT_BOOL:
    return cha_ast_codegen_node_constant_bool(ast_node);
  case CHA_AST_NODE_TYPE_BIN_OP:
    return cha_ast_codegen_node_bin_op(ast_node);
  case CHA_AST_NODE_TYPE_VARIABLE_DECLARATION:
    return cha_ast_codegen_node_var(ast_node);
  case CHA_AST_NODE_TYPE_VARIABLE_ASSIGNMENT:
    return cha_ast_codegen_node_var_assign(ast_node);
  case CHA_AST_NODE_TYPE_VARIABLE_LOOKUP:
    return cha_ast_codegen_node_var_lookup(ast_node);
  case CHA_AST_NODE_TYPE_FUNCTION_CALL:
    return cha_ast_codegen_node_call(ast_node);
  case CHA_AST_NODE_TYPE_FUNCTION_RETURN:
    return cha_ast_codegen_node_ret(ast_node);
  case CHA_AST_NODE_TYPE_FUNCTION_DECLARATION:
    return cha_ast_codegen_node_fun(ast_node);
  case CHA_AST_NODE_TYPE_BLOCK:
    return cha_ast_codegen_block(ast_node->block);
  case CHA_AST_NODE_TYPE_IF:
    return cha_ast_codegen_if(ast_node);
  case CHA_AST_NODE_TYPE_ARGUMENT:
  case CHA_AST_NODE_TYPE_CONSTANT_DECLARATION:
    // DO NOTHING
    break;
  default:
    log_validation_error(ast_node->location, "not implemented");
    return 1;
  }

  return 0;
}

int cha_ast_codegen_block(cha_ast_node_list *block) {
  if (block == NULL) {
    return 0;
  }
  cha_ast_node_list_entry *cur = block->head;
  while (cur != NULL) {
    int ret = cha_ast_codegen_node(cur->node);
    if (ret != 0) {
      return ret;
    }
    cur = cur->next;
  }
  return 0;
}

int cha_ast_codegen_toplevel(cha_ast_node_list *ast) {
  cha_ast_node_list_entry *cur = ast->head;
  while (cur != NULL) {
    int ret = cha_ast_codegen_node(cur->node);
    if (ret != 0) {
      return ret;
    }
    cur = cur->next;
  }
  return 0;
}

int cha_ast_codegen_node_constant_number(cha_ast_node *ast_node) {
  unsigned char radix;
  if (strncmp("0x", ast_node->const_value, 2) == 0 ||
      strncmp("0X", ast_node->const_value, 2) == 0) {
    radix = 16;
  } else {
    radix = 10;
  }

  LLVMTypeRef t = make_type(ast_node->_result_type);

  return_operand = LLVMConstIntOfString(t, ast_node->const_value, radix);
  return 0;
}

int cha_ast_codegen_node_constant_float(cha_ast_node *ast_node) {
  LLVMTypeRef t = make_type(ast_node->_result_type);

  return_operand = LLVMConstRealOfString(t, ast_node->const_value);
  return 0;
}

int cha_ast_codegen_node_constant_bool(cha_ast_node *ast_node) {
  LLVMTypeRef t = make_type(ast_node->_result_type);

  char str_bool[2];
  sprintf(str_bool, "%d", ast_node->const_bool);

  return_operand = LLVMConstIntOfString(t, str_bool, 10);
  return 0;
}

int cha_ast_codegen_node_bin_op(cha_ast_node *ast_node) {
  int ret = cha_ast_codegen_node(ast_node->bin_op.left);
  if (ret != 0) {
    return ret;
  }
  LLVMValueRef left_operand = return_operand;
  ret = cha_ast_codegen_node(ast_node->bin_op.right);
  if (ret != 0) {
    return ret;
  }
  LLVMValueRef right_operand = return_operand;

  switch (ast_node->bin_op.op) {
  case CHA_AST_OPERATOR_ADD:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildFAdd(builder, left_operand, right_operand, "add");
    } else {
      return_operand =
          LLVMBuildAdd(builder, left_operand, right_operand, "add");
    }
    break;
  case CHA_AST_OPERATOR_SUBTRACT:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildFSub(builder, left_operand, right_operand, "sub");
    } else {
      return_operand =
          LLVMBuildSub(builder, left_operand, right_operand, "sub");
    }
    break;
  case CHA_AST_OPERATOR_MULTIPLY:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildFMul(builder, left_operand, right_operand, "mul");
    } else {
      return_operand =
          LLVMBuildMul(builder, left_operand, right_operand, "mul");
    }
    break;
  case CHA_AST_OPERATOR_EQUALS_EQUALS:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand = LLVMBuildFCmp(builder, LLVMRealOEQ, left_operand,
                                     right_operand, "eq");

    } else {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntEQ, left_operand, right_operand, "eq");
    }
    break;
  case CHA_AST_OPERATOR_NOT_EQUALS:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand = LLVMBuildFCmp(builder, LLVMRealONE, left_operand,
                                     right_operand, "ne");

    } else {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntNE, left_operand, right_operand, "ne");
    }
    break;
  case CHA_AST_OPERATOR_GREATER_THAN:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand = LLVMBuildFCmp(builder, LLVMRealOGT, left_operand,
                                     right_operand, "gt");

    } else if (signed_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntSGT, left_operand, right_operand, "gt");
    } else {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntUGT, left_operand, right_operand, "gt");
    }
    break;
  case CHA_AST_OPERATOR_GREATER_THAN_OR_EQUALS:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand = LLVMBuildFCmp(builder, LLVMRealOGE, left_operand,
                                     right_operand, "ge");

    } else if (signed_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntSGE, left_operand, right_operand, "ge");
    } else {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntUGE, left_operand, right_operand, "ge");
    }
    break;
  case CHA_AST_OPERATOR_LESS_THAN:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand = LLVMBuildFCmp(builder, LLVMRealOLT, left_operand,
                                     right_operand, "lt");

    } else if (signed_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntSLT, left_operand, right_operand, "lt");
    } else {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntULT, left_operand, right_operand, "lt");
    }
    break;
  case CHA_AST_OPERATOR_LESS_THAN_OR_EQUALS:
    if (float_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand = LLVMBuildFCmp(builder, LLVMRealOLE, left_operand,
                                     right_operand, "le");

    } else if (signed_type(ast_node->bin_op.left->_result_type) == 0) {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntSLE, left_operand, right_operand, "le");
    } else {
      return_operand =
          LLVMBuildICmp(builder, LLVMIntULE, left_operand, right_operand, "le");
    }
    break;
  case CHA_AST_OPERATOR_AND:
    return_operand = LLVMBuildAnd(builder, left_operand, right_operand, "and");
    break;
  case CHA_AST_OPERATOR_OR:
    return_operand = LLVMBuildOr(builder, left_operand, right_operand, "or");
    break;
  default:
    log_validation_error(ast_node->location, "operation not supported");
    return 1;
  }

  return 0;
}

int cha_ast_codegen_node_var(cha_ast_node *ast_node) {
  LLVMTypeRef type = make_type(ast_node->variable_declaration.type);

  LLVMValueRef addr =
      LLVMBuildAlloca(builder, type, ast_node->variable_declaration.identifier);

  if (insert_symbol_table(codegen_symbol_table,
                          ast_node->variable_declaration.identifier, ast_node,
                          addr, type) != 0) {
    return 1;
  }

  if (ast_node->variable_declaration.value != NULL) {
    if (cha_ast_codegen_node(ast_node->variable_declaration.value) != 0) {
      return 1;
    }

    LLVMBuildStore(builder, return_operand, addr);
  }

  return 0;
}

int cha_ast_codegen_node_var_assign(cha_ast_node *ast_node) {
  symbol_value *value = get_symbol_table(codegen_symbol_table,
                                         ast_node->variable_lookup.identifier);
  if (value == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_lookup.identifier);
    return 1;
  }

  int ret = cha_ast_codegen_node(ast_node->variable_assignment.value);
  if (ret != 0) {
    return ret;
  }
  LLVMBuildStore(builder, return_operand, value->ref);
  return 0;
}

int cha_ast_codegen_node_var_lookup(cha_ast_node *ast_node) {
  symbol_value *value = get_symbol_table(codegen_symbol_table,
                                         ast_node->variable_lookup.identifier);
  if (value == NULL) {
    value = get_symbol_table(codegen_symbol_table,
                             ast_node->variable_lookup.identifier);
    if (value == NULL) {
      log_validation_error(ast_node->location, "'%s' not found",
                           ast_node->variable_lookup.identifier);
      return 1;
    }
  }

  return_operand = LLVMBuildLoad2(builder, value->type, value->ref,
                                  ast_node->variable_lookup.identifier);
  return 0;
}

int cha_ast_codegen_node_call(cha_ast_node *ast_node) {
  LLVMValueRef *args = NULL;
  int arg_count = 0;
  if (ast_node->function_call.argument_list != NULL) {
    arg_count = ast_node->function_call.argument_list->count;
    args = malloc(sizeof(LLVMValueRef) * arg_count);

    cha_ast_node_list_entry *current =
        ast_node->function_call.argument_list->head;
    int i = 0;
    while (current != NULL) {
      int ret = cha_ast_codegen_node(current->node);
      if (ret != 0) {
        free(args);
        return ret;
      }
      args[i] = return_operand;
      current = current->next;
      i++;
    }
  }

  symbol_value *function = get_symbol_table(codegen_symbol_table,
                                            ast_node->function_call.identifier);
  if (function == NULL) {
    free(args);
    log_validation_error(ast_node->location, "function '%s' not found",
                         ast_node->function_call.identifier);
    return 1;
  }

  return_operand =
      LLVMBuildCall2(builder, function->type, function->ref, args, arg_count,
                     ast_node->function_call.identifier);

  free(args);

  return 0;
}

int cha_ast_codegen_node_ret(cha_ast_node *ast_node) {
  if (ast_node->function_return.value != NULL) {
    int ret = cha_ast_codegen_node(ast_node->function_return.value);
    if (ret != 0) {
      return ret;
    }
    LLVMBuildRet(builder, return_operand);
  } else {
    LLVMBuildRetVoid(builder);
  }
  return 0;
}

int cha_ast_codegen_node_fun(cha_ast_node *ast_node) {
  cha_create_stack_frame();

  LLVMTypeRef fn_type = make_fun_signature(ast_node);

  LLVMValueRef function = LLVMAddFunction(
      module, ast_node->function_declaration.identifier, fn_type);

  insert_symbol_table(codegen_symbol_table,
                      ast_node->function_declaration.identifier, ast_node,
                      function, fn_type);

  LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function, "entry");
  LLVMPositionBuilderAtEnd(builder, entry_block);

  if (ast_node->function_declaration.argument_list != NULL) {
    cha_ast_node_list_entry *current =
        ast_node->function_declaration.argument_list->head;
    int i = 0;
    while (current != NULL) {
      LLVMTypeRef type = LLVMInt32TypeInContext(context);
      LLVMValueRef addr =
          LLVMBuildAlloca(builder, type, current->node->argument.identifier);
      LLVMBuildStore(builder, LLVMGetParam(function, i), addr);

      insert_symbol_table(codegen_symbol_table,
                          current->node->argument.identifier, current->node,
                          addr, type);

      current = current->next;
      i++;
    }
  }

  int ret = cha_ast_codegen_block(ast_node->function_declaration.block);
  cha_release_stack_frame();

  if (LLVMVerifyFunction(function, LLVMPrintMessageAction) != 0) {
    ret = 1;
  }

  return ret;
}

int cha_ast_codegen(cha_ast_node_list *ast, cha_compile_format format,
                   const char *file_path) {
  int ret = initialize_modules("nic");
  if (ret != 0) {
    free_modules();
    return ret;
  }

  ret = cha_ast_codegen_toplevel(ast);
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

  if (format == CHA_COMPILE_FORMAT_LLVM_IR) {
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
    if (format == CHA_COMPILE_FORMAT_ASSEMBLY_FILE) {
      gen_type = LLVMAssemblyFile;
    }
    char *obj_file_path = (char *)file_path;
    if (format == CHA_COMPILE_FORMAT_BINARY_FILE) {
      obj_file_path = malloc(strlen(file_path) + 3);
      sprintf(obj_file_path, "%s.o", file_path);
    }
    if (LLVMTargetMachineEmitToFile(target_machine, module, obj_file_path,
                                    gen_type, &errors) != 0) {
      if (format == CHA_COMPILE_FORMAT_BINARY_FILE) {
        free(obj_file_path);
      }
      log_error(errors);
      LLVMDisposeMessage(errors);
      free_modules();
      return 1;
    }
    LLVMDisposeMessage(errors);
    errors = NULL;

    if (format == CHA_COMPILE_FORMAT_BINARY_FILE) {
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

LLVMTypeRef make_fun_signature(cha_ast_node *ast_node) {
  LLVMTypeRef *arg_types = NULL;
  int arg_count = 0;
  if (ast_node->function_declaration.argument_list != NULL) {
    arg_count = ast_node->function_declaration.argument_list->count;
  }
  if (arg_count > 0) {
    arg_types = malloc(sizeof(LLVMTypeRef) * arg_count);
    cha_ast_node_list_entry *current =
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
  codegen_symbol_table = make_symbol_table(SYMBOL_TABLE_SIZE, NULL);

  return 0;
}

void free_modules() {
  LLVMDisposeTargetMachine(target_machine);
  LLVMDisposeTargetData(target_data_layout);
  LLVMDisposeMessage(target_triple);
  LLVMDisposeBuilder(builder);
  LLVMDisposeModule(module);
  LLVMContextDispose(context);
  free_all_symbol_tables(codegen_symbol_table);
}

int signed_type(const cha_ast_type *ast_type) {
  if (ast_type == NULL) {
    return 1;
  }

  switch (ast_type->internal_type) {
  case CHA_AST_INTERNAL_TYPE_CONST_UINT:
  case CHA_AST_INTERNAL_TYPE_UINT:
  case CHA_AST_INTERNAL_TYPE_UINT8:
  case CHA_AST_INTERNAL_TYPE_UINT16:
  case CHA_AST_INTERNAL_TYPE_UINT32:
  case CHA_AST_INTERNAL_TYPE_UINT64:
  case CHA_AST_INTERNAL_TYPE_UINT128:
    return 1;
  default:
    return 0;
  }
}

int float_type(const cha_ast_type *ast_type) {
  if (ast_type == NULL) {
    return 0;
  }

  switch (ast_type->internal_type) {
  case CHA_AST_INTERNAL_TYPE_CONST_FLOAT:
  case CHA_AST_INTERNAL_TYPE_FLOAT16:
  case CHA_AST_INTERNAL_TYPE_FLOAT32:
  case CHA_AST_INTERNAL_TYPE_FLOAT64:
    return 0;
  default:
    return 1;
  }

  return 1;
}

int cha_ast_codegen_if(cha_ast_node *ast_node) {
  LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
  LLVMValueRef fun = LLVMGetBasicBlockParent(current_block);

  LLVMBasicBlockRef then_block =
      LLVMAppendBasicBlockInContext(context, fun, "then");
  LLVMBasicBlockRef else_block =
      LLVMAppendBasicBlockInContext(context, fun, "else");
  LLVMBasicBlockRef end_block =
      LLVMAppendBasicBlockInContext(context, fun, "end");

  cha_create_stack_frame();
  LLVMPositionBuilderAtEnd(builder, then_block);
  if (cha_ast_codegen_block(ast_node->if_block.block) != 0) {
    cha_release_stack_frame();
    return 1;
  }
  if (LLVMGetLastInstruction(then_block) == NULL ||
      !LLVMIsATerminatorInst(LLVMGetLastInstruction(then_block))) {
    LLVMBuildBr(builder, end_block);
  }
  cha_release_stack_frame();

  cha_create_stack_frame();
  LLVMPositionBuilderAtEnd(builder, else_block);
  if (cha_ast_codegen_block(ast_node->if_block.else_block) != 0) {
    cha_release_stack_frame();
    return 1;
  }
  if (LLVMGetLastInstruction(else_block) == NULL ||
      !LLVMIsATerminatorInst(LLVMGetLastInstruction(else_block))) {
    LLVMBuildBr(builder, end_block);
  }
  cha_release_stack_frame();

  LLVMPositionBuilderAtEnd(builder, current_block);
  if (cha_ast_codegen_node(ast_node->if_block.condition) != 0) {
    return 1;
  }
  LLVMBuildCondBr(builder, return_operand, then_block, else_block);

  LLVMPositionBuilderAtEnd(builder, end_block);

  return 0;
}

LLVMTypeRef make_type(cha_ast_type *ast_type) {
  if (ast_type == NULL) {
    return LLVMVoidTypeInContext(context);
  }

  switch (ast_type->internal_type) {
  case CHA_AST_INTERNAL_TYPE_UNDEF:
    return LLVMVoidTypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_CONST_INT:
  case CHA_AST_INTERNAL_TYPE_CONST_UINT:
  case CHA_AST_INTERNAL_TYPE_INT:
  case CHA_AST_INTERNAL_TYPE_UINT:
    return LLVMIntPtrTypeInContext(context, target_data_layout);
  case CHA_AST_INTERNAL_TYPE_UINT8:
  case CHA_AST_INTERNAL_TYPE_INT8:
    return LLVMInt8TypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_INT16:
  case CHA_AST_INTERNAL_TYPE_UINT16:
    return LLVMInt16TypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_INT32:
  case CHA_AST_INTERNAL_TYPE_UINT32:
    return LLVMInt32TypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_INT64:
  case CHA_AST_INTERNAL_TYPE_UINT64:
    return LLVMInt64TypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_INT128:
  case CHA_AST_INTERNAL_TYPE_UINT128:
    return LLVMInt128TypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_FLOAT16:
    return LLVMHalfTypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_CONST_FLOAT:
  case CHA_AST_INTERNAL_TYPE_FLOAT32:
    return LLVMFloatTypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_FLOAT64:
    return LLVMDoubleTypeInContext(context);
  case CHA_AST_INTERNAL_TYPE_BOOL:
    return LLVMInt1TypeInContext(context);
  }

  return LLVMVoidTypeInContext(context);
}

void cha_create_stack_frame() {
  symbol_table *new_table =
      make_symbol_table(SYMBOL_TABLE_SIZE, codegen_symbol_table);
  codegen_symbol_table = new_table;
}

void cha_release_stack_frame() {
  symbol_table *new_table = codegen_symbol_table;
  codegen_symbol_table = codegen_symbol_table->parent;
  free_symbol_table(new_table);
}
