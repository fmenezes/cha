#include <unistd.h>
#include <string>
#include <fstream>
#include <memory>

#include "ni/parse/syntax_parser.hh"
#include "ni/ast/validator.hh"
#include "ni/ast/ast.hh"
#include "test/test.hh"

int test_parse_parse_success(int argc, char *argv[]) {
  ni::parse::syntax_parser p;
  try{
    p.parse("parse/parse_success.ni");
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
