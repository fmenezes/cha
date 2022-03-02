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

  auto fd = dynamic_cast<const ni::NFunctionDeclaration *>(&node);
  if (fd != nullptr) {
    return this->internalCodegen(*fd);
  }

  auto fc = dynamic_cast<const ni::NFunctionCall *>(&node);
  if (fc != nullptr) {
    return this->internalCodegen(*fc);
  }

  auto fr = dynamic_cast<const ni::NFunctionReturn *>(&node);
  if (fr != nullptr) {
    return this->internalCodegen(*fr);
  }

  auto p = dynamic_cast<const ni::NProgram *>(&node);
  if (p != nullptr) {
    return this->internalCodegen(*p);
  }

  throw std::runtime_error("Unkown node type");
}

int ni::ASMCodegen::internalCodegen(const ni::NInteger &node) {
  *this->outputFile << "\tmovl\t$" << node.value << ", %eax" << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NBinaryOperation &node) {

  int ret = this->internalCodegen(*node.left.get());
  if (ret != 0) {
    return ret;
  }
  *this->outputFile << "\tmovl\t%eax, %ebx" << std::endl;

  ret = this->internalCodegen(*node.right.get());
  if (ret != 0) {
    return ret;
  }

  if (node.op.compare("+") == 0) {
    *this->outputFile << "\taddl\t%ebx, %eax" << std::endl;
    return 0;
  } else if (node.op.compare("-") == 0) {
    *this->outputFile << "\tsubl\t%ebx, %eax" << std::endl;
    return 0;
  } else if (node.op.compare("*") == 0) {
    *this->outputFile << "\timull\t%ebx, %eax" << std::endl;
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
  *this->outputFile << "\tmovl\t" << s->second << "(%rbp),%eax" << std::endl;
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
  *this->outputFile << "\tmovl\t%eax, " << s->second << "(%rbp)" << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableDeclaration &node) {
  this->currentStackPosition -= 4;
  this->vars[node.identifier] = this->currentStackPosition;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NFunctionDeclaration &node) {
  this->currentFunctionName = this->generateFunctionName(node.identifier);

  int ret = this->generateFunction(this->currentFunctionName);
  if (ret != 0) {
    return ret;
  }
  ret = this->generateFunctionPrologue();
  if (ret != 0) {
    return ret;
  }
  this->resetStackFrame();
  for (const auto &it : node.body) {
    int ret = this->internalCodegen(*it.get());
    if (ret != 0) {
      return ret;
    }
  }
  return this->generateFunctionEpilogue(this->currentFunctionName);
}

int ni::ASMCodegen::internalCodegen(const ni::NFunctionCall &node) {
  std::string fnName = this->generateFunctionName(node.identifier);
  *this->outputFile << "\tcallq " << fnName << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NFunctionReturn &node) {
  int ret = this->internalCodegen(*node.value.get());
  if (ret != 0) {
    return ret;
  }

  *this->outputFile << "\tjmp " << this->currentFunctionName << "_epilogue"
                    << std::endl;
  return 0;
}

void ni::ASMCodegen::resetStackFrame() {
  this->currentStackPosition = 0;
  this->vars.clear();
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

std::string
ni::ASMCodegen::generateFunctionName(const std::string &name) const {
  if (this->targetOS == ni::OS::MACOS) {
    return "_" + name;
  }
  return name;
}

int ni::ASMCodegen::generateFunction(const std::string &name) {
  *this->outputFile << ".globl\t" << name << std::endl;
  *this->outputFile << std::endl;
  *this->outputFile << name << ":" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunctionPrologue() {
  *this->outputFile << "\tpushq\t%rbp" << std::endl;
  *this->outputFile << "\tmovq\t%rsp, %rbp" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunctionEpilogue(const std::string &name) {
  *this->outputFile << name << "_epilogue:" << std::endl;
  *this->outputFile << "\tpopq\t%rbp" << std::endl;
  *this->outputFile << "\tretq" << std::endl << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NProgram &node) {
  int ret = this->generateTextSection();
  if (ret != 0) {
    return ret;
  }
  for (const auto &it : node.instructions) {
    ret = this->internalCodegen(*it.get());
    if (ret != 0) {
      return ret;
    }
  }
  return 0;
}

int ni::ASMCodegen::codegen(const std::string &output, std::string &error) {
  this->outputFile = new std::ofstream();
  this->outputFile->open(output, std::ios::trunc);
  int ret = ni::ASMCodegen::internalCodegen(this->program);
  this->outputFile->close();
  this->outputFile = nullptr;

  return ret;
}
