#pragma once

#include <memory>

#include "ast.hh"
#include "generated/location.hh"
#include "location.hh"

namespace ni {
namespace ast {
class syntax_error : public std::runtime_error {
public:
  syntax_error(const location &loc, const std::string &m)
      : loc(loc), std::runtime_error(m) {}
  location loc;
};

class parser {
public:
  yy::location location;
  std::unique_ptr<program> prg;
  void parse(const std::string &f);
};
} // namespace ast
} // namespace ni
