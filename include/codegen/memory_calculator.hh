#pragma once

#include "ast/ast.hh"
#include "ast/visitor.hh"

namespace ni {
namespace codegen {

class memory_calculator : public ni::ast::visitor {
public:
  static int calculare(const ni::ast::function_declaration &node);

protected:
  void visit(const ni::ast::function_declaration &node) override;
  void visit(const ni::ast::variable_declaration &node) override;

private:
  int total = 0;
};
} // namespace codegen
} // namespace ni
