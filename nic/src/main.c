#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nic/ast.h"
#include "nic/codegen.h"
#include "nic/validate.h"
#include "log.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    log_error("Usage: %s <format> <outputfile> <inputfile>\n", argv[0]);
    log_error("format: -s for Assembly Code\n");
    log_error("format: -c for Object File\n");
    log_error("format: -ll for LLVM IR\n");
    return 1;
  }

  char *format = argv[1];
  char *outputfile = argv[2];
  char *inputfile = argv[3];

  ni_ast_codegen_format codegen_format;
  if (strcmp(format, "-s") == 0) {
    codegen_format = ASSEMBLY;
  } else if (strcmp(format, "-c") == 0) {
    codegen_format = OBJECT_FILE;
  } else if (strcmp(format, "-ll") == 0) {
    codegen_format = LLVM_IR;
  } else {
    log_error("Invalid format: %s\n", format);
    return 1;
  }

  FILE *file = fopen(inputfile, "r");
  if (file == NULL) {
    log_error("Could not open file %s\n", inputfile);
    return 1;
  }

  ni_ast_node_list *ast;
  int ret = ni_ast_parse(file, &ast);
  fclose(file);
  if (ret != 0) {
    log_error("Could not parse file %s\n", inputfile);
    return ret;
  }

  ret = ni_ast_validate(ast);
  if (ret != 0) {
    return ret;
  }

  ret = ni_ast_codegen(ast, codegen_format, outputfile);

  free_ni_ast_node_list(ast);

  return ret;
}
