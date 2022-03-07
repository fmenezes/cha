#include "ast.hh"

int ni::ast::Parser::parse(const std::string &f, NProgram &program) {
  ni::ast::Parser p(program);
  return p.parse(f);
}
