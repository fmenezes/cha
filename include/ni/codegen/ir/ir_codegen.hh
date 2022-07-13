#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ni/ast/ast.hh"
#include "ni/ast/visitor.hh"
#include "ni/codegen/codegen.hh"
#include "ni/codegen/ir/ir_common.hh"

namespace ni {
namespace codegen {
namespace ir {
class ir_codegen : public ni::ast::visitor, public codegen {
public:
  ir_codegen(const ni::ast::program &p) : codegen(p){};
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
  ir_operand return_operand = ir_operand(ir_operand_type::CONSTANT, "0");
  std::map<std::string, ir_operand> vars;
  int current_temprary_id = 0;
  std::string current_function_name;
  bool first_instruction = false;
  void generate_exit_call();
  void generate_start_function();
  std::string generate_label();
  void append_instruction(ir_instruction instruction);
  ir_program instructions;
};
} // namespace ir
} // namespace codegen
} // namespace ni
