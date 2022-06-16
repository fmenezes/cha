#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast.hh"
#include "ast/visitor.hh"
#include "codegen/assembly/att_printer.hh"
#include "codegen/codegen.hh"

namespace ni {
namespace codegen {
namespace assembly {

const std::vector<ni::codegen::assembly::operand>
    REGS({(ni::codegen::assembly::operand)
              ni::codegen::assembly::register_32bits::EDI,
          (ni::codegen::assembly::operand)
              ni::codegen::assembly::register_32bits::ESI,
          (ni::codegen::assembly::operand)
              ni::codegen::assembly::register_32bits::EDX,
          (ni::codegen::assembly::operand)
              ni::codegen::assembly::register_32bits::ECX,
          (ni::codegen::assembly::operand)
              ni::codegen::assembly::register_32bits::R8D,
          (ni::codegen::assembly::operand)
              ni::codegen::assembly::register_32bits::R9D});

class asm_codegen : public ni::ast::visitor, public codegen {
public:
  asm_codegen(const ni::ast::program &p) : codegen(p){};
  asm_codegen(const ni::ast::program &p, const context &ctx)
      : codegen(p, ctx), printer(ctx){};
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
  att_printer printer;
  std::map<std::string, operand> vars;
  std::string current_function_name;
  int current_stack_position;
  void generate_exit_call();
  void generate_start_function();
  operand return_operand;
};
} // namespace assembly
} // namespace codegen
} // namespace ni
