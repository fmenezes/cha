#pragma once

#include "ast/ast.hh"

namespace ni {
namespace codegen {

class MemoryCalculator : public ni::ast::Visitor {
public:
  static int calculare(const ni::ast::NFunctionDeclaration &node);

protected:
  void visit(const ni::ast::NFunctionDeclaration &node) override;
  void visit(const ni::ast::NVariableDeclaration &node) override;

private:
  int total = 0;

};
} // namespace codegen
} // namespace ni
