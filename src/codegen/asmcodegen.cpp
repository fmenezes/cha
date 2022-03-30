#include <fstream>
#include <iostream>

#include "ast/ast.hh"
#include "codegen/asmcodegen.hh"
#include "codegen/codegen.hh"
#include "codegen/memorycalculator.hh"

void ni::codegen::ASMCodegen::visit(const ni::ast::NConstantInteger &node) {
  this->returnOperand = ni::codegen::Operand(std::stoi(node.value));
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NBinaryOperation &node) {
  ni::ast::Visitor::visit(*node.left);
  ni::codegen::Operand laddr = this->returnOperand;
  if (laddr == Register32Bits::EAX) {
    this->printer.mov(Register32Bits::ECX, laddr);
    laddr = (ni::codegen::Operand)Register32Bits::ECX;
  }

  ni::ast::Visitor::visit(*node.right);
  ni::codegen::Operand raddr = this->returnOperand;
  if (raddr != Register32Bits::EAX) {
    this->printer.mov(Register32Bits::EAX, raddr);
    raddr = (ni::codegen::Operand)Register32Bits::EAX;
  }

  this->returnOperand = raddr;

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

void ni::codegen::ASMCodegen::visit(const ni::ast::NVariableLookup &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  this->returnOperand = s->second;
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NVariableAssignment &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  ni::ast::Visitor::visit(*node.value);
  this->printer.mov(s->second, this->returnOperand);
  this->returnOperand = s->second;
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NVariableDeclaration &node) {
  auto s = this->vars.find(node.identifier);
  if (s != this->vars.end()) {
    throw std::runtime_error(node.identifier + " already exists.");
  }

  this->currentStackPosition -= 4;
  this->returnOperand =
      Operand(Register64Bits::RBP, this->currentStackPosition);
  this->vars.insert({node.identifier, this->returnOperand});
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NFunctionDeclaration &node) {
  this->currentFunctionName = node.identifier;

  this->printer.global(this->currentFunctionName);
  this->printer.label(this->currentFunctionName);

  int memorySize = MemoryCalculator::calculare(node);
  this->printer.push(Register64Bits::RBP);
  this->printer.mov(Register64Bits::RBP, Register64Bits::RSP);
  this->printer.sub(Register64Bits::RSP, memorySize);
  this->currentStackPosition = 0;
  this->vars.clear();

  int argc = node.args.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int a = 0; a < argc; a++) {
    auto &arg = *node.args[a];
    this->currentStackPosition -= 4;

    auto addr = Operand(Register64Bits::RBP, this->currentStackPosition);
    this->vars.insert({arg.identifier, addr});
    this->printer.mov(addr, REGS[a]);
  }

  if (node.args.size() > 6) {
    int offset = 16;
    for (int a = 6; a < node.args.size(); a++) {
      auto &arg = *node.args[a];
      this->vars.insert({arg.identifier, Operand(Register64Bits::RBP, offset)});
      offset += 8;
    }
  }

  ni::ast::Visitor::visit(*node.body);

  this->printer.label(this->currentFunctionName + "_epilogue");
  this->printer.add(Register64Bits::RSP, memorySize);
  this->printer.pop(Register64Bits::RBP);
  this->printer.ret();
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NFunctionCall &node) {
  int argc = node.params.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int i = 0; i < argc; i++) {
    auto &param = *node.params[i];
    auto reg = REGS[i];
    ni::ast::Visitor::visit(param);
    this->printer.mov(REGS[i], this->returnOperand);
  }
  for (int i = (node.params.size() - 1); i >= 6; i--) {
    auto &param = *node.params[i];
    ni::ast::Visitor::visit(param);
    this->printer.push(this->returnOperand);
  }

  this->printer.call(node.identifier);

  argc = node.params.size();
  if (argc > REGS.size()) {
    argc -= REGS.size();
    this->printer.add(Register64Bits::RSP, (argc * 8));
  }

  this->returnOperand = Register32Bits::EAX;
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NFunctionReturn &node) {
  if (node.value.get() != nullptr) {
    ni::ast::Visitor::visit(*node.value);
    auto addr = this->returnOperand;
    if (addr != Register32Bits::EAX) {
      this->printer.mov(Register32Bits::EAX, addr);
    }
  }
  this->printer.jmp(this->currentFunctionName + "_epilogue");
  this->returnOperand = Register32Bits::EAX;
}

void ni::codegen::ASMCodegen::generateExitCall() {
  int exitCode = 60;
  if (this->context.targetOS == ni::codegen::OS::MACOS) {
    exitCode = 0x2000001;
  }

  this->printer.mov(Register32Bits::EAX, exitCode);
  this->printer.syscall();
}

void ni::codegen::ASMCodegen::visit(const ni::ast::NProgram &node) {
  this->printer.textHeader();
  this->generateStartFunction();
  ni::ast::Visitor::visit(node);
}

void ni::codegen::ASMCodegen::generateStartFunction() {
  this->printer.labelStart();
  this->printer.call("main");
  this->printer.mov(Register32Bits::EDI, Register32Bits::EAX);
  this->generateExitCall();
}

void ni::codegen::ASMCodegen::codegen(const std::string &output) {
  this->printer.openFile(output);
  this->visit(this->program);
  this->printer.closeFile();
}
