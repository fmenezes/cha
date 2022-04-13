#pragma once

#include <memory>

#include "ast.hh"
#include "generated/location.hh"

namespace ni {
namespace ast {
class parser {
public:
  yy::location location;
  std::unique_ptr<program> prg;
  void parse(const std::string &f);
};
} // namespace ast
} // namespace ni
