#include <fstream>
#include <memory>
#include <string>
#include <unistd.h>

#include "ni/ast/ast.hh"
#include "ni/ast/validator.hh"
#include "ni/parse/syntax_parser.hh"
#include "test/test.hh"

int test_ast_validation_passes(int argc, char *argv[]) {
  auto p = parse("ast/validation_passes.ni");
  try {
    ni::ast::validator::validate(*p);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
