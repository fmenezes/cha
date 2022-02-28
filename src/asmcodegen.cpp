#include <fstream>
#include <iostream>

#include "nodes.hh"

int ni::ASMCodegen::internalCodegen(const ni::Node &node) {
  auto i = dynamic_cast<const ni::NInteger *>(&node);
  if (i != nullptr) {
    return this->internalCodegen(*i);
  }

  auto b = dynamic_cast<const ni::NBinaryOperation *>(&node);
  if (b != nullptr) {
    return this->internalCodegen(*b);
  }

  auto d = dynamic_cast<const ni::NVariableDeclaration *>(&node);
  if (d != nullptr) {
    return this->internalCodegen(*d);
  }

  auto a = dynamic_cast<const ni::NVariableAssignment *>(&node);
  if (a != nullptr) {
    return this->internalCodegen(*a);
  }

  auto l = dynamic_cast<const ni::NVariableLookup *>(&node);
  if (l != nullptr) {
    return this->internalCodegen(*l);
  }

  auto p = dynamic_cast<const ni::NProgram *>(&node);
  if (p != nullptr) {
    return this->internalCodegen(*p);
  }

  throw std::runtime_error("Unkown node type");
}

int ni::ASMCodegen::internalCodegen(const ni::NInteger &node) {
  *this->outputFile << "\tmovl\t$" << node.value << ", %edi" << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NBinaryOperation &node) {

  int ret = this->internalCodegen(*node.left.get());
  if (ret != 0) {
    return ret;
  }
  *this->outputFile << "\tmovl\t%edi, %eax" << std::endl;

  ret = this->internalCodegen(*node.right.get());
  if (ret != 0) {
    return ret;
  }
  *this->outputFile << "\tmovl\t%edi, %ebx" << std::endl;

  if (node.op.compare("+") == 0) {
    *this->outputFile << "\taddl\t%eax, %ebx" << std::endl;
    *this->outputFile << "\tmovl\t%ebx, %edi" << std::endl;
    return 0;
  } else if (node.op.compare("-") == 0) {
    *this->outputFile << "\tsubl\t%eax, %ebx" << std::endl;
    *this->outputFile << "\tmovl\t%ebx, %edi" << std::endl;
    return 0;
  } else if (node.op.compare("*") == 0) {
    *this->outputFile << "\timull\t%eax, %ebx, %edi" << std::endl;
    return 0;
  }

  std::cerr << "Invalid operation " << node.op << std::endl;
  return 1;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableLookup &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    std::cerr << node.identifier << " Not found." << std::endl;
    return 1;
  }
  *this->outputFile << "\tmovl\t" << s->second << "(%rbp),%edi" << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableAssignment &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    std::cerr << node.identifier << " Not found." << std::endl;
    return 1;
  }
  int ret = this->internalCodegen(*node.value.get());
  if (ret != 0) {
    return ret;
  }
  *this->outputFile << "\tmovl\t%edi, " << s->second << "(%rbp)" << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableDeclaration &node) {
  this->currentStackPosition -= 4;
  this->vars[node.identifier] = this->currentStackPosition;
  return 0;
}

int ni::ASMCodegen::generateTextSection() {
  if (this->targetOS == ni::OS::MACOS) {
    *this->outputFile << ".section\t__TEXT,__text" << std::endl;
  } else {
    *this->outputFile << ".text" << std::endl;
  }
  return 0;
}

int ni::ASMCodegen::generateExitCall() {
  std::string exitCode = "60";
  if (this->targetOS == ni::OS::MACOS) {
    exitCode = "0x2000001";
  }
  *this->outputFile << "\tmovl\t$" << exitCode << ", %eax" << std::endl;
  *this->outputFile << "\tsyscall" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunction(const std::string &name) {
  std::string fnName = name;
  if (this->targetOS == ni::OS::MACOS) {
    fnName = "_" + fnName;
  }
  *this->outputFile << ".globl\t" << fnName << std::endl;
  *this->outputFile << std::endl;
  *this->outputFile << fnName << ":" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunctionPrologue() {
  *this->outputFile << "\tpushq\t%rbp" << std::endl;
  *this->outputFile << "\tmovq\t%rsp, %rbp" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunctionEpilogue() {
  *this->outputFile << "\tpopq\t%rbp" << std::endl;
  *this->outputFile << "\tretq" << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NProgram &node) {
  int ret = this->generateTextSection();
  if (ret != 0) {
    return ret;
  }
  ret = this->generateFunction("main");
  if (ret != 0) {
    return ret;
  }
  ret = this->generateFunctionPrologue();
  if (ret != 0) {
    return ret;
  }
  for (const auto &it : node.instructions) {
    ret = this->internalCodegen(*it.get());
    if (ret != 0) {
      return ret;
    }
  }
  ret = this->generateExitCall();
  if (ret != 0) {
    return ret;
  }
  return this->generateFunctionEpilogue();
}

int ni::ASMCodegen::codegen(const std::string &output, std::string &error) {
  this->outputFile = new std::ofstream();
  this->outputFile->open(output, std::ios::trunc);
  int ret = ni::ASMCodegen::internalCodegen(this->program);
  this->outputFile->close();
  this->outputFile = nullptr;

  return ret;
}

ni::OS ni::ASMCodegen::defaultOs() {
#if defined(__APPLE__) || defined(__MACH__)
  return ni::OS::MACOS;
#else
  return ni::OS::LINUX;
#endif
}
