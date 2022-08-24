#include <fstream>
#include <memory>
#include <string>
#include <unistd.h>

#include "ni/ast/ast.hh"
#include "ni/ast/validator.hh"
#include "ni/parse/syntax_parser.hh"
#include "test/test.hh"

int test_parse_parse_failure(int argc, char *argv[]) {
  ni::parse::syntax_parser p;
  try {
    p.parse("parse/parse_failure.ni");
  } catch (const ni::parse::syntax_error &e) {
    return 0;
  }

  std::cerr << "should have thrown ni::parse::syntax_error" << std::endl;
  return 1;
}
