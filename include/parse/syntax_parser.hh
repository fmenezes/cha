#pragma once

#include <memory>

#include "ast/ast.hh"
#include "ast/location.hh"
#include "generated/location.hh"

namespace ni {
namespace parse {
class syntax_error : public std::runtime_error {
public:
  syntax_error(const ni::ast::location &loc, const std::string &m)
      : loc(loc), std::runtime_error(m) {}
  ni::ast::location loc;
};

class syntax_parser {
public:
  yy::location location;
  std::unique_ptr<ni::ast::program> prg;
  void parse(const std::string &f);
};
} // namespace parse
} // namespace ni
