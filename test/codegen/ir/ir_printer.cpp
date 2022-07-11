#include <string>
#include <fstream>

#include "ni/ast/ast.hh"
#include "test/test.hh"
#include "ni/codegen/ir/ir_printer.hh"


int test_codegen_ir_ir_printer(int argc, char *argv[]) {
  auto ss = std::make_shared<std::stringstream>();

  ni::codegen::ir::ir_printer ir(ss);

  ir.global("start");
  ir.label("start");
  ir.call(ni::codegen::ir::ir_operand(
      ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp1"), "test");
  ir.global("test");
  ir.label("test");
  ir.mov(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp1"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::CONSTANT, "10"));
  ir.mov(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp2"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::CONSTANT, "20"));
  ir.mov(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp3"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::CONSTANT, "30"));
  ir.mov(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp4"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::CONSTANT, "40"));
  ir.add(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp5"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp1"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp2"));
  ir.mul(
    ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp6"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp3"),
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp4"));
  ir.push(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::CONSTANT, "20"));
  ir.alloc(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::MEMORY, "a"), 4);
  ir.pop(
      ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::MEMORY, "a"));

  ir.exit(ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::MEMORY,
                                      "a"));
  ir.jmp("test2");
  ir.ret(ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::CONSTANT,
                                     "0"));

  std::ifstream t("codegen/ir/ir_printer.ir");
  std::stringstream expected;
  expected << t.rdbuf();

  if (expected.str() != ss->str()) {
    std::cout << "expected: " << expected.str() << std::endl;
    std::cout << "actual: " << ss->str() << std::endl;
    return 1;
  }

  return 0;
}
