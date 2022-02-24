#include <memory>
#include <sstream>

#include "nodes.hh"
#include "parser.tab.hh"

int ni::NProgram::parse() {
  yy::parser p(*this);
  return p.parse();
}
