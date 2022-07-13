#include <fstream>
#include <string>

#include "ni/ast/ast.hh"
#include "ni/codegen/assembly/att_printer.hh"
#include "ni/codegen/codegen.hh"
#include "test/test.hh"

int test_codegen_assembly_attprinter_mac(int argc, char *argv[]) {
  ni::codegen::assembly::asm_program p;

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::TEXT_SECTION));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::GLOBAL,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::LABEL, "start")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      "start", ni::codegen::assembly::asm_operation::CALL,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::LABEL, "test")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::GLOBAL,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::LABEL, "test")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      "test", ni::codegen::assembly::asm_operation::MOV,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_register::RAX),
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::CONSTANT, "10")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::MOV,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_register::EAX),
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::CONSTANT, "10")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::ADD,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_register::EDI),
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::CONSTANT, "20")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::SUB,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_register::ECX),
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::CONSTANT, "30")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::IMUL,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_register::EDX),
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::CONSTANT, "40")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::PUSH,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::CONSTANT, "50", 64)}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::SYSCALL));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::JMP,
      std::vector<ni::codegen::assembly::asm_operand>{
          ni::codegen::assembly::asm_operand(
              ni::codegen::assembly::asm_operand_type::LABEL, "test2")}));

  p.push_back(ni::codegen::assembly::asm_instruction(
      ni::codegen::assembly::asm_operation::RET));

  auto c =
      ni::codegen::context(ni::codegen::os::MACOS, ni::codegen::arch::x86_64);

  auto printer = new ni::codegen::assembly::att_printer(c, p);

  std::stringstream gotBuffer;
  gotBuffer << ni::codegen::assembly::att_printer(c, p);

  std::ifstream expectedFile("codegen/assembly/attprinter_mac.s");
  std::stringstream expectedBuffer;
  expectedBuffer << expectedFile.rdbuf();
  expectedFile.close();

  if (expectedBuffer.str() != gotBuffer.str()) {
    std::cout << "expected: " << expectedBuffer.str() << std::endl;
    std::cout << "actual: " << gotBuffer.str() << std::endl;
    return 1;
  }

  return 0;
}
