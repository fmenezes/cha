#include <stdlib.h>
#include <string.h>

#include "nic/ast.h"
#include "nic/ast_dump.h"
#include "parser.tab.h"

void ni_ast_dump_node(ni_ast_node *node) {
  if (node == NULL) {
    printf("null");
    return;
  }

  switch (node->type) {
  case NI_AST_TYPE_INT_CONSTANT:
    printf("{\"const_int\":%s}", node->int_const.value);
    break;
  case NI_AST_TYPE_BIN_OP:
    printf("{\"bin_op\":{\"op\":\"%s\",\"left\":", node->bin_op.op);
    ni_ast_dump_node(node->bin_op.left);
    printf(",\"right\":");
    ni_ast_dump_node(node->bin_op.right);
    printf("}}");
    break;
  case NI_AST_TYPE_VARIABLE_DECLARATION:
    printf("{\"var_decl\":{\"identifier\":\"%s\",\"type\":",
           node->variable_declaration.identifier);
    ni_ast_dump_node(node->variable_declaration.type);
    printf("}}");
    break;
  case NI_AST_TYPE_VARIABLE_ASSIGNMENT:
    printf("{\"var_assign\":{\"identifier\":\"%s\",\"value\":",
           node->variable_assignment.identifier);
    ni_ast_dump_node(node->variable_assignment.value);
    printf("}}");
    break;
  case NI_AST_TYPE_VARIABLE_LOOKUP:
    printf("{\"var_lookup\":{\"identifier\":\"%s\"}}",
           node->variable_lookup.identifier);
    break;
  case NI_AST_TYPE_ARGUMENT:
    printf("{\"arg\":{\"identifier\":\"%s\",\"type\":",
           node->argument.identifier);
    ni_ast_dump_node(node->argument.type);
    printf("}}");
    break;
  case NI_AST_TYPE_BLOCK:
    ni_ast_dump(node->block);
    break;
  case NI_AST_TYPE_INT_TYPE:
    printf("\"int\"");
    break;
  case NI_AST_TYPE_FUNCTION_CALL:
    printf("{\"call\":{\"identifier\":\"%s\",\"params\":",
           node->function_call.identifier);
    ni_ast_dump(node->function_call.argument_list);
    printf("}}");
    break;
  case NI_AST_TYPE_FUNCTION_RETURN:
    printf("{\"return\":");
    ni_ast_dump_node(node->function_return.value);
    printf("}");
    break;
  case NI_AST_TYPE_FUNCTION_DECLARATION:
    printf("{\"fun\":{\"identifier\":\"%s\",\"ret_type\":",
           node->function_declaration.identifier);
    ni_ast_dump_node(node->function_declaration.return_type);
    printf(",\"args\":");
    ni_ast_dump(node->function_declaration.argument_list);
    printf(",\"block\":");
    ni_ast_dump(node->function_declaration.block);
    printf("}}");
    break;
  }
}

void ni_ast_dump(ni_ast_node_list *ast) {
  if (ast == NULL) {
    printf("null");
    return;
  }
  ni_ast_node_list_entry *cur = ast->head;
  char separator[2] = "[";
  while (cur != NULL) {
    printf("%s", separator);
    ni_ast_dump_node(cur->node);

    cur = cur->next;
    separator[0] = ',';
  }
  printf("]");
}
