#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast.hh"
#include "ast/visitor.hh"
#include "codegen/codegen.hh"
#include "codegen/ir/ir_printer.hh"

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
  std::unique_ptr<ir_printer> printer;
  std::map<std::string, ir_operand> vars;
  int current_temprary_id = 0;
  std::string current_function_name;
  std::shared_ptr<std::ofstream> output_file;
  void generate_exit_call();
  void generate_start_function();
  void close_file();
};
} // namespace ir
} // namespace codegen
} // namespace ni
