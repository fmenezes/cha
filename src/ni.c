#include "ni/ni.h"
#include "ast.h"
#include "parser.h"
#include "log.h"
#include "validate.h"
#include "codegen.h"

int ni_compile(const char *file, ni_compile_format format, const char *output_file) {
  ni_ast_node_list *ast;
  int ret = ni_parse(file, &ast);
  if (ret != 0) {
    log_error("Could not parse file %s\n", file);
    return ret;
  }
  ret = ni_validate(ast);
  if (ret != 0) {
    return ret;
  }

  ret = ni_ast_codegen(ast, format, output_file);

  free_ni_ast_node_list(ast);

  return ret;
}