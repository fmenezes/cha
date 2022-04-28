#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.hh"
#include "parse/syntax_parser.hh"

#include "generated/location.hh"
#include "generated/parser.tab.hh"

extern FILE *yyin;

void ni::parse::syntax_parser::parse(const std::string &f) {
  if (!(yyin = fopen(f.c_str(), "r"))) {
    throw std::runtime_error("cannot open " + f);
  }

  std::string *fp = new std::string(f);
  this->location = yy::location(fp);
  yy::parser p(*this);
  int r = p.parse();
  fclose(yyin);
  if (r != 0) {
    throw std::runtime_error("error parsing");
  }
}
