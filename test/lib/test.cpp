#include <unistd.h>

#include "test/test.hh"
#include "ni/parse/syntax_parser.hh"

std::unique_ptr<ni::ast::program> parse(const std::string &filename) {
  ni::parse::syntax_parser p;
  p.parse(filename);
  return std::move(p.prg);
}

std::string makeTempDir() {
  char buf[] = "ni_unit_test_XXXXXX";
  char *tmp = mkdtemp(buf);
  if (tmp == nullptr) {
    throw std::runtime_error("unexpected error: " + std::to_string(errno));
  }
  return std::string("./") + tmp;
}
