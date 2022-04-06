#include "codegen/memory_calculator.hh"
#include "ast/ast.hh"
#include "codegen/asm_codegen.hh"

void ni::codegen::memory_calculator::visit(
    const ni::ast::function_declaration &node) {
  auto argc = node.args.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  this->total += argc * 4;
  ni::ast::visitor::visit(node);
}
void ni::codegen::memory_calculator::visit(
    const ni::ast::variable_declaration &node) {
  this->total += 4;
  ni::ast::visitor::visit(node);
}

int ni::codegen::memory_calculator::calculare(
    const ni::ast::function_declaration &node) {
  ni::codegen::memory_calculator c;
  c.visit(node);
  return c.total;
}
