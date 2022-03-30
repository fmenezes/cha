#include "codegen/memorycalculator.hh"
#include "ast/ast.hh"
#include "codegen/asmcodegen.hh"

void ni::codegen::MemoryCalculator::visit(
    const ni::ast::NFunctionDeclaration &node) {
  auto argc = node.args.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  this->total += argc * 4;
  ni::ast::Visitor::visit(node);
}
void ni::codegen::MemoryCalculator::visit(
    const ni::ast::NVariableDeclaration &node) {
  this->total += 4;
  ni::ast::Visitor::visit(node);
}

int ni::codegen::MemoryCalculator::calculare(
    const ni::ast::NFunctionDeclaration &node) {
  ni::codegen::MemoryCalculator c;
  c.visit(node);
  return c.total;
}
