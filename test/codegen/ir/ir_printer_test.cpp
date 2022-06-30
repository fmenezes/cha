#include <sstream>
#include <gtest/gtest.h>
#include <stdio.h>

#include "ni/codegen/ir/ir_printer.hh"

namespace ni {
namespace test {
namespace {
TEST(IRPrinterTest, Main) {
  std::string expected;
  expected.append(".global\tstart\n");
  expected.append("start:\n");
  expected.append("\tcall\t%tmp1,\ttest\n");
  expected.append(".global\ttest\n");
  expected.append("test:\n");
  expected.append("\tmov\t%tmp1,\t$10\n");
  expected.append("\tmov\t%tmp2,\t$20\n");
  expected.append("\tmov\t%tmp3,\t$30\n");
  expected.append("\tmov\t%tmp4,\t$40\n");
  expected.append("\tadd\t%tmp5,\t%tmp1,\t%tmp2\n");
  expected.append("\tmul\t%tmp6,\t%tmp3,\t%tmp4\n");
  expected.append("\tpush\t$20\n");
  expected.append("\talloc\t&a,\t$4\n");
  expected.append("\tpop\t&a\n");
  expected.append("\texit\t&a\n");
  expected.append("\tjmp\ttest2\n");
  expected.append("\tret\t$0\n");

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

  EXPECT_EQ(ss->str(), expected);
}
} // namespace
} // namespace test
} // namespace ni
