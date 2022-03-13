#include <fstream>
#include <iostream>

#include "ast/ast.hh"
#include "codegen/asmcodegen.hh"
#include "codegen/codegen.hh"

const std::vector<ni::codegen::Operand>
    REGS({(ni::codegen::Operand)ni::codegen::Register32Bits::EDI,
          (ni::codegen::Operand)ni::codegen::Register32Bits::ESI,
          (ni::codegen::Operand)ni::codegen::Register32Bits::EDX,
          (ni::codegen::Operand)ni::codegen::Register32Bits::ECX,
          (ni::codegen::Operand)ni::codegen::Register32Bits::R8D,
          (ni::codegen::Operand)ni::codegen::Register32Bits::R9D});

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::Node &node) {
  auto s = dynamic_cast<const ni::ast::NStatement *>(&node);
  if (s != nullptr) {
    return this->internalCodegen(*s);
  }

  auto fd = dynamic_cast<const ni::ast::NFunctionDeclaration *>(&node);
  if (fd != nullptr) {
    return this->internalCodegen(*fd);
  }

  auto p = dynamic_cast<const ni::ast::NProgram *>(&node);
  if (p != nullptr) {
    return this->internalCodegen(*p);
  }

  throw std::runtime_error("Unkown node type");
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NStatement &node) {
  auto d = dynamic_cast<const ni::ast::NVariableDeclaration *>(&node);
  if (d != nullptr) {
    return this->internalCodegen(*d);
  }

  auto a = dynamic_cast<const ni::ast::NVariableAssignment *>(&node);
  if (a != nullptr) {
    return this->internalCodegen(*a);
  }

  auto fr = dynamic_cast<const ni::ast::NFunctionReturn *>(&node);
  if (fr != nullptr) {
    return this->internalCodegen(*fr);
  }

  auto e = dynamic_cast<const ni::ast::NExpression *>(&node);
  if (e != nullptr) {
    return this->internalCodegen(*e);
  }

  throw std::runtime_error("Unkown node type");
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NExpression &node) {
  auto c = dynamic_cast<const ni::ast::NConstant *>(&node);
  if (c != nullptr) {
    return this->internalCodegen(*c);
  }

  auto b = dynamic_cast<const ni::ast::NBinaryOperation *>(&node);
  if (b != nullptr) {
    return this->internalCodegen(*b);
  }

  auto l = dynamic_cast<const ni::ast::NVariableLookup *>(&node);
  if (l != nullptr) {
    return this->internalCodegen(*l);
  }

  auto fc = dynamic_cast<const ni::ast::NFunctionCall *>(&node);
  if (fc != nullptr) {
    return this->internalCodegen(*fc);
  }

  throw std::runtime_error("Unkown node type");
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NConstant &node) {
  auto i = dynamic_cast<const ni::ast::NConstantInteger *>(&node);
  if (i != nullptr) {
    return this->internalCodegen(*i);
  }

  throw std::runtime_error("Unkown node type");
}

ni::codegen::Operand ni::codegen::ASMCodegen::internalCodegen(
    const ni::ast::NConstantInteger &node) {
  return ni::codegen::Operand(std::stoi(node.value));
}

ni::codegen::Operand ni::codegen::ASMCodegen::internalCodegen(
    const ni::ast::NBinaryOperation &node) {
  ni::codegen::Operand laddr = this->internalCodegen(*node.left);
  if (laddr == Register32Bits::EAX) {
    this->printer.mov(Register32Bits::ECX, laddr);
    laddr = (ni::codegen::Operand)Register32Bits::ECX;
  }

  ni::codegen::Operand raddr = this->internalCodegen(*node.right);
  if (raddr != Register32Bits::EAX) {
    this->printer.mov(Register32Bits::EAX, raddr);
    raddr = (ni::codegen::Operand)Register32Bits::EAX;
  }

  if (node.op.compare("+") == 0) {
    this->printer.add(raddr, laddr);
    return raddr;
  } else if (node.op.compare("-") == 0) {
    this->printer.sub(raddr, laddr);
    return raddr;
  } else if (node.op.compare("*") == 0) {
    this->printer.imul(raddr, laddr);
    return raddr;
  }

  throw std::runtime_error("Invalid operation " + node.op);
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NVariableLookup &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  return s->second;
}

ni::codegen::Operand ni::codegen::ASMCodegen::internalCodegen(
    const ni::ast::NVariableAssignment &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  ni::codegen::Operand addr = this->internalCodegen(*node.value);
  this->printer.mov(s->second, addr);
  return s->second;
}

ni::codegen::Operand ni::codegen::ASMCodegen::internalCodegen(
    const ni::ast::NVariableDeclaration &node) {
  auto s = this->vars.find(node.identifier);
  if (s != this->vars.end()) {
    throw std::runtime_error(node.identifier + " already exists.");
  }

  this->currentStackPosition -= 4;
  Operand returnAddr = Operand(Register64Bits::RBP, this->currentStackPosition);
  this->vars.insert({node.identifier, returnAddr});
  return returnAddr;
}
int calculateMemorySize(const ni::ast::Node *node) {
  auto b = dynamic_cast<const ni::ast::NBinaryOperation *>(node);
  if (b != nullptr) {
    return calculateMemorySize(b->left.get()) +
           calculateMemorySize(b->right.get());
  }

  auto d = dynamic_cast<const ni::ast::NVariableDeclaration *>(node);
  if (d != nullptr) {
    return 4;
  }

  auto fd = dynamic_cast<const ni::ast::NFunctionDeclaration *>(node);
  if (fd != nullptr) {
    int ret = fd->args.size() * 4;
    if (fd->args.size() > REGS.size()) {
      ret = REGS.size() * 4;
    }
    ret += 4;
    for (auto &it : fd->body) {
      ret += calculateMemorySize(it.get());
    }
    return ret;
  }

  return 0;
}

ni::codegen::Operand ni::codegen::ASMCodegen::internalCodegen(
    const ni::ast::NFunctionDeclaration &node) {
  this->currentFunctionName = node.identifier;

  this->printer.global(this->currentFunctionName);
  this->printer.label(this->currentFunctionName);

  int memorySize = calculateMemorySize(&node);
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

  for (const auto &it : node.body) {
    this->internalCodegen(*it);
  }

  this->printer.label(this->currentFunctionName + "_epilogue");
  this->printer.add(Register64Bits::RSP, memorySize);
  this->printer.pop(Register64Bits::RBP);
  this->printer.ret();

  return Operand();
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NFunctionCall &node) {
  int argc = node.params.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int i = 0; i < argc; i++) {
    auto &param = *node.params[i];
    auto reg = REGS[i];
    auto addr = this->internalCodegen(param);
    this->printer.mov(REGS[i], addr);
  }
  for (int i = (node.params.size() - 1); i >= 6; i--) {
    auto &param = *node.params[i];
    auto addr = this->internalCodegen(param);
    this->printer.push(addr);
  }

  this->printer.call(node.identifier);

  argc = node.params.size();
  if (argc > REGS.size()) {
    argc -= REGS.size();
    this->printer.add(Register64Bits::RSP, (argc * 8));
  }

  return Register32Bits::EAX;
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NFunctionReturn &node) {
  if (node.value.get() != nullptr) {
    auto addr = this->internalCodegen(*node.value.get());
    if (addr != Register32Bits::EAX) {
      this->printer.mov(Register32Bits::EAX, addr);
    }
  }
  this->printer.jmp(this->currentFunctionName + "_epilogue");
  return (ni::codegen::Operand)Register32Bits::EAX;
}

void ni::codegen::ASMCodegen::generateExitCall() {
  int exitCode = 60;
  if (this->context.targetOS == ni::codegen::OS::MACOS) {
    exitCode = 0x2000001;
  }

  this->printer.mov(Register32Bits::EAX, exitCode);
  this->printer.syscall();
}

ni::codegen::Operand
ni::codegen::ASMCodegen::internalCodegen(const ni::ast::NProgram &node) {
  this->printer.textHeader();
  for (const auto &it : node.instructions) {
    this->internalCodegen(*it);
  }
  return Operand();
}

int ni::codegen::ASMCodegen::codegen(const std::string &output,
                                     std::string &error) {
  this->printer.openFile(output);
  ni::codegen::ASMCodegen::internalCodegen(this->program);
  this->printer.closeFile();
  return 0;
}
