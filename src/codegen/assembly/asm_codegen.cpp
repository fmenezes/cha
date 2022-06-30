#include <fstream>
#include <iostream>

#include "ni/ast/ast.hh"
#include "ni/codegen/assembly/asm_codegen.hh"
#include "ni/codegen/codegen.hh"
#include "ni/codegen/memory_calculator.hh"

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::constant_integer &node) {
  this->return_operand = ni::codegen::assembly::operand(std::stoi(node.value));
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::binary_operation &node) {
  ni::ast::visitor::visit(*node.left);
  ni::codegen::assembly::operand laddr = this->return_operand;
  if (laddr == register_32bits::EAX) {
    this->printer.mov(register_32bits::ECX, laddr);
    laddr = (ni::codegen::assembly::operand)register_32bits::ECX;
  }

  ni::ast::visitor::visit(*node.right);
  ni::codegen::assembly::operand raddr = this->return_operand;
  if (raddr != register_32bits::EAX) {
    this->printer.mov(register_32bits::EAX, raddr);
    raddr = (ni::codegen::assembly::operand)register_32bits::EAX;
  }

  this->return_operand = raddr;

  if (node.op.compare("+") == 0) {
    this->printer.add(raddr, laddr);
  } else if (node.op.compare("-") == 0) {
    this->printer.sub(raddr, laddr);
  } else if (node.op.compare("*") == 0) {
    this->printer.imul(raddr, laddr);
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
  this->printer.mov(s->second, this->return_operand);
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
      operand(register_64bits::RBP, this->current_stack_position);
  this->vars.insert({node.identifier, this->return_operand});
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::function_declaration &node) {
  this->current_function_name = node.identifier;

  this->printer.global(this->current_function_name);
  this->printer.label(this->current_function_name);

  int memorySize = memory_calculator::calculare(node);
  this->printer.push(register_64bits::RBP);
  this->printer.mov(register_64bits::RBP, register_64bits::RSP);
  this->printer.sub(register_64bits::RSP, memorySize);
  this->current_stack_position = 0;
  this->vars.clear();

  int argc = node.args.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int a = 0; a < argc; a++) {
    auto &arg = *node.args[a];
    this->current_stack_position -= 4;

    auto addr = operand(register_64bits::RBP, this->current_stack_position);
    this->vars.insert({arg.identifier, addr});
    this->printer.mov(addr, REGS[a]);
  }

  if (node.args.size() > 6) {
    int offset = 16;
    for (int a = 6; a < node.args.size(); a++) {
      auto &arg = *node.args[a];
      this->vars.insert(
          {arg.identifier, operand(register_64bits::RBP, offset)});
      offset += 8;
    }
  }

  ni::ast::visitor::visit(*node.body);

  this->printer.label(this->current_function_name + "_epilogue");
  this->printer.add(register_64bits::RSP, memorySize);
  this->printer.pop(register_64bits::RBP);
  this->printer.ret();
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
    this->printer.mov(REGS[i], this->return_operand);
  }
  for (int i = (node.params.size() - 1); i >= 6; i--) {
    auto &param = *node.params[i];
    ni::ast::visitor::visit(param);
    this->printer.push(this->return_operand);
  }

  this->printer.call(node.identifier);

  argc = node.params.size();
  if (argc > REGS.size()) {
    argc -= REGS.size();
    this->printer.add(register_64bits::RSP, (argc * 8));
  }

  this->return_operand = register_32bits::EAX;
}

void ni::codegen::assembly::asm_codegen::visit(
    const ni::ast::function_return &node) {
  if (node.value.get() != nullptr) {
    ni::ast::visitor::visit(*node.value);
    auto addr = this->return_operand;
    if (addr != register_32bits::EAX) {
      this->printer.mov(register_32bits::EAX, addr);
    }
  }
  this->printer.jmp(this->current_function_name + "_epilogue");
  this->return_operand = register_32bits::EAX;
}

void ni::codegen::assembly::asm_codegen::generate_exit_call() {
  int exitCode = 60;
  if (this->ctx.target_os == ni::codegen::os::MACOS) {
    exitCode = 0x2000001;
  }

  this->printer.mov(register_32bits::EAX, exitCode);
  this->printer.syscall();
}

void ni::codegen::assembly::asm_codegen::visit(const ni::ast::program &node) {
  this->printer.text_header();
  this->generate_start_function();
  ni::ast::visitor::visit(node);
}

void ni::codegen::assembly::asm_codegen::generate_start_function() {
  this->printer.label_start();
  this->printer.call("main");
  this->printer.mov(register_32bits::EDI, register_32bits::EAX);
  this->generate_exit_call();
}

void ni::codegen::assembly::asm_codegen::generate(const std::string &output) {
  this->printer.open_file(output);
  this->visit(this->program);
  this->printer.close_file();
}
