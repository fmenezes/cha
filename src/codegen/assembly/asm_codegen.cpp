#include <fstream>
#include <iostream>

#include "ni/ast/ast.hh"
#include "ni/codegen/assembly/asm_codegen.hh"
#include "ni/codegen/codegen.hh"
#include "ni/codegen/memory_calculator.hh"

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::constant_integer &node) {
  return_operand = ni::codegen::assembly::asm_operand(
      ni::codegen::assembly::asm_operand_type::CONSTANT, node.value);
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::binary_operation &node) {
  ni::ast::visitor::visit(*node.left);
  ni::codegen::assembly::asm_operand laddr = this->return_operand;
  if (laddr == asm_register::EAX) {
    p.push_back(asm_instruction(
        asm_operation::MOV,
        std::vector<ni::codegen::assembly::asm_operand>{
            ni::codegen::assembly::asm_operand(asm_register::ECX), laddr}));
    laddr = (ni::codegen::assembly::asm_operand)asm_register::ECX;
  }

  ni::ast::visitor::visit(*node.right);
  ni::codegen::assembly::asm_operand raddr = this->return_operand;
  if (raddr != asm_register::EAX) {
    p.push_back(asm_instruction(
        asm_operation::MOV,
        std::vector<ni::codegen::assembly::asm_operand>{
            ni::codegen::assembly::asm_operand(asm_register::EAX), raddr}));
    raddr = (ni::codegen::assembly::asm_operand)asm_register::EAX;
  }

  this->return_operand = raddr;

  if (node.op.compare("+") == 0) {
    p.push_back(asm_instruction(
        asm_operation::ADD,
        std::vector<ni::codegen::assembly::asm_operand>{raddr, laddr}));
  } else if (node.op.compare("-") == 0) {
    p.push_back(asm_instruction(
        asm_operation::SUB,
        std::vector<ni::codegen::assembly::asm_operand>{raddr, laddr}));
  } else if (node.op.compare("*") == 0) {
    p.push_back(asm_instruction(
        asm_operation::IMUL,
        std::vector<ni::codegen::assembly::asm_operand>{raddr, laddr}));
  } else {
    throw std::runtime_error("invalid operation " + node.op);
  }
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::variable_lookup &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  this->return_operand = s->second;
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::variable_assignment &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  ni::ast::visitor::visit(*node.value);

  p.push_back(asm_instruction(asm_operation::MOV,
                              std::vector<ni::codegen::assembly::asm_operand>{
                                  s->second, return_operand}));

  this->return_operand = s->second;
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::variable_declaration &node) {
  auto s = this->vars.find(node.identifier);
  if (s != this->vars.end()) {
    throw std::runtime_error(node.identifier + " already exists.");
  }

  this->current_stack_position -= 4;
  this->return_operand =
      asm_operand(asm_register::RBP, this->current_stack_position);
  this->vars.insert({node.identifier, this->return_operand});
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::function_declaration &node) {
  this->current_function_name = node.identifier;

  p.push_back(asm_instruction(
      asm_operation::GLOBAL,
      std::vector<ni::codegen::assembly::asm_operand>{
          asm_operand(asm_operand_type::LABEL, this->current_function_name)}));

  int memorySize = memory_calculator::calculare(node);
  p.push_back(asm_instruction(this->current_function_name, asm_operation::PUSH,
                              std::vector<ni::codegen::assembly::asm_operand>{
                                  asm_operand(asm_register::RBP)}));
  p.push_back(asm_instruction(
      asm_operation::MOV,
      std::vector<ni::codegen::assembly::asm_operand>{
          asm_operand(asm_register::RBP), asm_operand(asm_register::RSP)}));
  p.push_back(asm_instruction(asm_operation::SUB,
                              std::vector<ni::codegen::assembly::asm_operand>{
                                  asm_operand(asm_register::RSP),
                                  asm_operand(asm_operand_type::CONSTANT,
                                              std::to_string(memorySize))}));
  this->current_stack_position = 0;
  this->vars.clear();

  int argc = node.args.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int a = 0; a < argc; a++) {
    auto &arg = *node.args[a];
    this->current_stack_position -= 4;

    auto addr = asm_operand(asm_register::RBP, this->current_stack_position);
    this->vars.insert({arg.identifier, addr});
    p.push_back(asm_instruction(
        asm_operation::MOV,
        std::vector<ni::codegen::assembly::asm_operand>{addr, REGS[a]}));
  }

  if (node.args.size() > 6) {
    int offset = 16;
    for (int a = 6; a < node.args.size(); a++) {
      auto &arg = *node.args[a];
      this->vars.insert(
          {arg.identifier, asm_operand(asm_register::RBP, offset)});
      offset += 8;
    }
  }

  ni::ast::visitor::visit(*node.body);

  p.push_back(asm_instruction(this->current_function_name + "_epilogue",
                              asm_operation::ADD,
                              std::vector<ni::codegen::assembly::asm_operand>{
                                  asm_operand(asm_register::RSP),
                                  asm_operand(asm_operand_type::CONSTANT,
                                              std::to_string(memorySize))}));
  p.push_back(asm_instruction(asm_operation::POP,
                              std::vector<ni::codegen::assembly::asm_operand>{
                                  asm_operand(asm_register::RBP)}));
  p.push_back(asm_instruction(asm_operation::RET));
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::function_call &node) {
  int argc = node.params.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int i = 0; i < argc; i++) {
    auto &param = *node.params[i];
    auto reg = REGS[i];
    ni::ast::visitor::visit(param);
    p.push_back(asm_instruction(asm_operation::MOV,
                                std::vector<ni::codegen::assembly::asm_operand>{
                                    REGS[i], return_operand}));
  }
  for (int i = (node.params.size() - 1); i >= 6; i--) {
    auto &param = *node.params[i];
    ni::ast::visitor::visit(param);
    p.push_back(asm_instruction(
        asm_operation::PUSH,
        std::vector<ni::codegen::assembly::asm_operand>{return_operand}));
  }

  p.push_back(asm_instruction(
      asm_operation::CALL,
      std::vector<ni::codegen::assembly::asm_operand>{
          asm_operand(asm_operand_type::LABEL, node.identifier)}));

  argc = node.params.size();
  if (argc > REGS.size()) {
    argc -= REGS.size();
    p.push_back(asm_instruction(asm_operation::ADD,
                                std::vector<ni::codegen::assembly::asm_operand>{
                                    asm_operand(asm_register::RSP),
                                    asm_operand(asm_operand_type::CONSTANT,
                                                std::to_string(argc * 8))}));
  }

  return_operand = asm_operand(asm_register::EAX);
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::function_return &node) {
  if (node.value.get() != nullptr) {
    ni::ast::visitor::visit(*node.value);
    auto addr = this->return_operand;
    if (addr != asm_register::EAX) {
      p.push_back(asm_instruction(
          asm_operation::MOV, std::vector<ni::codegen::assembly::asm_operand>{
                                  asm_operand(asm_register::EAX), addr}));
    }
  }

  p.push_back(asm_instruction(
      asm_operation::JMP,
      std::vector<ni::codegen::assembly::asm_operand>{
          asm_operand(asm_operand_type::LABEL,
                      this->current_function_name + "_epilogue")}));

  return_operand = asm_operand(asm_register::EAX);
}

void ni::codegen::assembly::asm_codegen::generate_exit_call() {
  int exitCode = 60;
  if (this->ctx.target_os == ni::codegen::os::MACOS) {
    exitCode = 0x2000001;
  }

  p.push_back(asm_instruction(
      asm_operation::MOV,
      std::vector<ni::codegen::assembly::asm_operand>{
          asm_operand(asm_register::EAX),
          asm_operand(asm_operand_type::CONSTANT, std::to_string(exitCode))}));

  p.push_back(asm_instruction(asm_operation::SYSCALL));
}

void ni::codegen::assembly::asm_codegen::visit(const ni::ast::program &node) {
  p.push_back(asm_instruction(asm_operation::TEXT_SECTION));
  this->generate_start_function();
  ni::ast::visitor::visit(node);
}

void ni::codegen::assembly::asm_codegen::generate_start_function() {
  p.push_back(
      asm_instruction(asm_operation::GLOBAL,
                      std::vector<ni::codegen::assembly::asm_operand>{
                          asm_operand(asm_operand_type::LABEL, "start")}));

  p.push_back(
      asm_instruction("start", asm_operation::CALL,
                      std::vector<ni::codegen::assembly::asm_operand>{
                          asm_operand(asm_operand_type::LABEL, "main")}));

  p.push_back(asm_instruction(
      asm_operation::MOV,
      std::vector<ni::codegen::assembly::asm_operand>{
          asm_operand(asm_register::EDI), asm_operand(asm_register::EAX)}));
  this->generate_exit_call();
}

void ni::codegen::assembly::asm_codegen::generate(const std::string &output) {
  this->visit(this->program);

  std::ofstream output_file;
  output_file.open(output, std::ios::trunc);
  output_file << ni::codegen::assembly::att_printer(ctx, p);
  output_file.close();
}
