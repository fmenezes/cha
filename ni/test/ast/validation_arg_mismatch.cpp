#include <fstream>
#include <memory>
#include <string>
#include <unistd.h>

#include "ni/ast/ast.hh"
#include "ni/ast/validator.hh"
#include "ni/parse/syntax_parser.hh"
#include "test/test.hh"

int test_ast_validation_arg_mismatch(int argc, char *argv[]) {
  auto p = parse("ast/validation_arg_mismatch.ni");
  try {
    ni::ast::validator::validate(*p);
  } catch (const ni::parse::syntax_error &e) {
    return 0;
  }

  std::cerr << "should have thrown ni::parse::syntax_error" << std::endl;
  return 1;
}
