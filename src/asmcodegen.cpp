#include <fstream>
#include <iostream>

#include "nodes.hh"

int ni::ASMCodegen::internalCodegen(const ni::NProgram &node) {
  *this->outputFile << "global _start" << std::endl;
  *this->outputFile << std::endl;
  *this->outputFile << "_start:" << std::endl;
  *this->outputFile << "mov rax, 60" << std::endl;
  *this->outputFile << "mov rdi, 100" << std::endl;
  *this->outputFile << "syscall" << std::endl;
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
