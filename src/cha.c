#include "cha/cha.h"
#include "ast.h"
#include "codegen.h"
#include "log.h"
#include "parser.h"
#include "validate.h"

int cha_compile(const char *file, cha_compile_format format,
               const char *output_file) {
  cha_ast_node_list *ast;
  int ret = cha_parse(file, &ast);
  if (ret != 0) {
    log_error("Could not parse file %s\n", file);
    return ret;
  }
  ret = cha_validate(ast);
  if (ret != 0) {
    return ret;
  }

  ret = cha_ast_codegen(ast, format, output_file);

  free_cha_ast_node_list(ast);

  return ret;
}