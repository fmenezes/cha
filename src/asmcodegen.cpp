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

int ni::ASMCodegen::internalCodegen(const ni::NInteger &node) { return 0; }

int ni::ASMCodegen::internalCodegen(const ni::NBinaryOperation &node) {
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableLookup &node) {
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableAssignment &node) {
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableDeclaration &node) {
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NProgram &node) {
  *this->outputFile << ".section __TEXT,__text" << std::endl;
  *this->outputFile << ".globl _main" << std::endl;
  *this->outputFile << std::endl;
  *this->outputFile << "_main:" << std::endl;
  *this->outputFile << "\tmovl $0x2000001, %eax" << std::endl;
  *this->outputFile << "\tmovl $100, %edi" << std::endl;
  *this->outputFile << "\tsyscall" << std::endl;
  return 0;
}

int ni::ASMCodegen::codegen(const std::string &output, std::string &error) {
  this->outputFile = new std::ofstream();
  this->outputFile->open(output, std::ios::trunc);
  ni::ASMCodegen::internalCodegen(this->program);
  this->outputFile->close();
  this->outputFile = nullptr;

  return 0;
}
