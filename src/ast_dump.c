#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ni/ast.h"
#include "ni/ast_dump.h"
#include "parser.tab.h"

void ni_ast_dump_node(FILE *out, const ni_ast_node *node) {
  if (node == NULL) {
    fprintf(out, "null");
    return;
  }

  switch (node->type) {
  case NI_AST_TYPE_CONSTANT_NUMBER:
    fprintf(out, "{\"const_int\":%s}", node->int_const.value);
    break;
  case NI_AST_TYPE_BIN_OP:
    fprintf(out, "{\"bin_op\":{\"op\":\"");
    switch (node->bin_op.op) {
    case NI_AST_OPERATOR_PLUS:
      fprintf(out, "+");
      break;
    case NI_AST_OPERATOR_MINUS:
      fprintf(out, "-");
      break;
    case NI_AST_OPERATOR_MULTIPLY:
      fprintf(out, "*");
      break;
    }
    fprintf(out, "\",\"left\":");
    ni_ast_dump_node(out, node->bin_op.left);
    fprintf(out, ",\"right\":");
    ni_ast_dump_node(out, node->bin_op.right);
    fprintf(out, "}}");
    break;
  case NI_AST_TYPE_VARIABLE_DECLARATION:
    fprintf(out, "{\"var_decl\":{\"identifier\":\"%s\",\"type\":",
            node->variable_declaration.identifier);
    ni_ast_dump_node(out, node->variable_declaration.type);
    fprintf(out, "}}");
    break;
  case NI_AST_TYPE_VARIABLE_ASSIGNMENT:
    fprintf(out, "{\"var_assign\":{\"identifier\":\"%s\",\"value\":",
            node->variable_assignment.identifier);
    ni_ast_dump_node(out, node->variable_assignment.value);
    fprintf(out, "}}");
    break;
  case NI_AST_TYPE_VARIABLE_LOOKUP:
    fprintf(out, "{\"var_lookup\":{\"identifier\":\"%s\"}}",
            node->variable_lookup.identifier);
    break;
  case NI_AST_TYPE_ARGUMENT:
    fprintf(out, "{\"arg\":{\"identifier\":\"%s\",\"type\":",
            node->argument.identifier);
    ni_ast_dump_node(out, node->argument.type);
    fprintf(out, "}}");
    break;
  case NI_AST_TYPE_BLOCK:
    ni_ast_dump(out, node->block);
    break;
  case NI_AST_TYPE_DEFTYPE_INT:
    fprintf(out, "\"int\"");
    break;
  case NI_AST_TYPE_FUNCTION_CALL:
    fprintf(out, "{\"call\":{\"identifier\":\"%s\",\"params\":",
            node->function_call.identifier);
    ni_ast_dump(out, node->function_call.argument_list);
    fprintf(out, "}}");
    break;
  case NI_AST_TYPE_FUNCTION_RETURN:
    fprintf(out, "{\"return\":");
    ni_ast_dump_node(out, node->function_return.value);
    fprintf(out, "}");
    break;
  case NI_AST_TYPE_FUNCTION_DECLARATION:
    fprintf(out, "{\"fun\":{\"identifier\":\"%s\",\"ret_type\":",
            node->function_declaration.identifier);
    ni_ast_dump_node(out, node->function_declaration.return_type);
    fprintf(out, ",\"args\":");
    ni_ast_dump(out, node->function_declaration.argument_list);
    fprintf(out, ",\"block\":");
    ni_ast_dump(out, node->function_declaration.block);
    fprintf(out, "}}");
    break;
  }
}

void ni_ast_dump(FILE *out, const ni_ast_node_list *ast) {
  if (ast == NULL) {
    fprintf(out, "null");
    return;
  }
  ni_ast_node_list_entry *cur = ast->head;
  char separator[2] = "[";
  while (cur != NULL) {
    fprintf(out, "%s", separator);
    ni_ast_dump_node(out, cur->node);

    cur = cur->next;
    separator[0] = ',';
  }
  fprintf(out, "]");
}
