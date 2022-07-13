#include <fstream>
#include <string>

#include "ni/ast/ast.hh"
#include "ni/codegen/ir/ir_printer.hh"
#include "test/test.hh"

int test_codegen_ir_ir_printer(int argc, char *argv[]) {

  ni::codegen::ir::ir_program p;
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::GLOBAL,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::LABEL, "start")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      "start", ni::codegen::ir::ir_operation::CALL,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp1"),
          ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::LABEL,
                                      "test")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::GLOBAL,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::LABEL, "test")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      "test", ni::codegen::ir::ir_operation::MOV,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp1"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::CONSTANT, "10")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::MOV,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp2"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::CONSTANT, "20")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::MOV,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp3"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::CONSTANT, "30")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::MOV,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp4"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::CONSTANT, "40")}

      ));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::ADD,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp5"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp1"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp2")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::MUL,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp6"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp3"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::TEMPORARY, "tmp4")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::PUSH,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::CONSTANT, "20")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::ALLOC,
      std::vector<ni::codegen::ir::ir_operand>{
          ni::codegen::ir::ir_operand(ni::codegen::ir::ir_operand_type::MEMORY,
                                      "a"),
          ni::codegen::ir::ir_operand(
              ni::codegen::ir::ir_operand_type::CONSTANT, "4")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::POP,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::MEMORY, "a")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::EXIT,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::MEMORY, "a")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::JMP,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::LABEL, "test2")}));
  p.push_back(ni::codegen::ir::ir_instruction(
      ni::codegen::ir::ir_operation::RET,
      std::vector<ni::codegen::ir::ir_operand>{ni::codegen::ir::ir_operand(
          ni::codegen::ir::ir_operand_type::CONSTANT, "0")}));

  std::stringstream ss;
  ss << ni::codegen::ir::ir_printer(p);

  std::ifstream t("codegen/ir/ir_printer.ir");
  std::stringstream expected;
  expected << t.rdbuf();

  if (expected.str() != ss.str()) {
    std::cout << "expected: " << expected.str() << std::endl;
    std::cout << "actual: " << ss.str() << std::endl;
    return 1;
  }

  return 0;
}
