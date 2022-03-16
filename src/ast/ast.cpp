#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.hh"
#include "parser.tab.hh"

extern FILE *yyin;

void ni::ast::Parser::parse(const std::string &f) {
  if (!(yyin = fopen(f.c_str(), "r"))) {
    throw std::runtime_error("cannot open " + f);
  }
  yy::parser p(*this);
  int r = p.parse();
  fclose(yyin);
  if (r != 0) {
    throw std::runtime_error("error parsing");
  }
}


