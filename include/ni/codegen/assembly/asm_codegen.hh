#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ni/ast/ast.hh"
#include "ni/ast/visitor.hh"
#include "ni/codegen/assembly/asm_common.hh"
#include "ni/codegen/assembly/att_printer.hh"
#include "ni/codegen/codegen.hh"

namespace ni {
namespace codegen {
namespace assembly {
class asm_codegen : public ni::ast::visitor, public codegen {
public:
  asm_codegen(const ni::ast::program &p) : codegen(p){};
  asm_codegen(const ni::ast::program &p, const context &ctx)
      : codegen(p, ctx){};
  void generate(const std::string &output) override;

protected:
  void visit(const ni::ast::program &node) override;
  void visit(const ni::ast::function_declaration &node) override;
  void visit(const ni::ast::function_call &node) override;
  void visit(const ni::ast::function_return &node) override;
  void visit(const ni::ast::variable_lookup &node) override;
  void visit(const ni::ast::variable_declaration &node) override;
  void visit(const ni::ast::variable_assignment &node) override;
  void visit(const ni::ast::binary_operation &node) override;
  void visit(const ni::ast::constant_integer &node) override;

private:
  std::map<std::string, asm_operand> vars;
  std::string current_function_name;
  int current_stack_position;
  void generate_exit_call();
  void generate_start_function();
  asm_operand return_operand = asm_operand(asm_operand_type::CONSTANT, "0");
  asm_program p;
};
} // namespace assembly
} // namespace codegen
} // namespace ni
