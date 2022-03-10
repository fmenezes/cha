#include <fstream>
#include <iostream>

#include "codegen/attprinter.hh"

std::string registerName(const ni::codegen::Register32Bits &op) {
  switch (op) {
  case ni::codegen::Register32Bits::EAX:
    return "%eax";
  case ni::codegen::Register32Bits::EBX:
    return "%ebx";
  case ni::codegen::Register32Bits::ECX:
    return "%ecx";
  case ni::codegen::Register32Bits::EDX:
    return "%edx";
  case ni::codegen::Register32Bits::ESI:
    return "%esi";
  case ni::codegen::Register32Bits::EDI:
    return "%edi";
  case ni::codegen::Register32Bits::EBP:
    return "%ebp";
  case ni::codegen::Register32Bits::ESP:
    return "%esp";
  case ni::codegen::Register32Bits::R8D:
    return "%r8d";
  case ni::codegen::Register32Bits::R9D:
    return "%r9d";
  case ni::codegen::Register32Bits::R10D:
    return "%r10d";
  case ni::codegen::Register32Bits::R11D:
    return "%r11d";
  case ni::codegen::Register32Bits::R12D:
    return "%r12d";
  case ni::codegen::Register32Bits::R13D:
    return "%r13d";
  case ni::codegen::Register32Bits::R14D:
    return "%r14d";
  case ni::codegen::Register32Bits::R15D:
    return "%r15d";
  case ni::codegen::Register32Bits::EIP:
    return "%eip";
  case ni::codegen::Register32Bits::EFLAGS:
    return "%eflags";
  }

  throw std::runtime_error("invalid operand");
}

std::string registerName(const ni::codegen::Register64Bits &op) {
  switch (op) {
  case ni::codegen::Register64Bits::RAX:
    return "%rax";
  case ni::codegen::Register64Bits::RBX:
    return "%rbx";
  case ni::codegen::Register64Bits::RCX:
    return "%rcx";
  case ni::codegen::Register64Bits::RDX:
    return "%rdx";
  case ni::codegen::Register64Bits::RSI:
    return "%rsi";
  case ni::codegen::Register64Bits::RDI:
    return "%rdi";
  case ni::codegen::Register64Bits::RBP:
    return "%rbp";
  case ni::codegen::Register64Bits::RSP:
    return "%rsp";
  case ni::codegen::Register64Bits::R8:
    return "%r8";
  case ni::codegen::Register64Bits::R9:
    return "%r9";
  case ni::codegen::Register64Bits::R10:
    return "%r10";
  case ni::codegen::Register64Bits::R11:
    return "%r11";
  case ni::codegen::Register64Bits::R12:
    return "%r12";
  case ni::codegen::Register64Bits::R13:
    return "%r13";
  case ni::codegen::Register64Bits::R14:
    return "%r14";
  case ni::codegen::Register64Bits::R15:
    return "%r15";
  case ni::codegen::Register64Bits::RIP:
    return "%rip";
  case ni::codegen::Register64Bits::RFLAGS:
    return "%rflags";
  }

  throw std::runtime_error("invalid operand");
}

std::string serializeConstant(const std::string &value) { return "$" + value; }

std::string serializeRegister(const ni::codegen::Register32Bits &reg) {
  return "%" + registerName(reg);
}

std::string serializeRegister(const ni::codegen::Register64Bits &reg) {
  return "%" + registerName(reg);
}

std::string serializeRegister(const int &reg, const int &size) {
  switch (size) {
  case 32: {
    return serializeRegister((ni::codegen::Register32Bits)reg);
  }
  case 64: {
    return serializeRegister((ni::codegen::Register64Bits)reg);
  }
  }

  throw std::runtime_error("invalid register");
}

std::string serializeOffsetRegister(const int &reg, const int &size,
                                    const int &offset) {
  std::string serializedReg = serializeRegister(reg, size);
  if (offset == 0) {
    return "(" + serializedReg + ")";
  }

  return std::to_string(offset) + "(" + serializedReg + ")";
}

std::string serializeOperand(const ni::codegen::Operand &op) {
  switch (op.type) {
  case ni::codegen::OperandType::CONSTANT: {
    return serializeConstant(op.value);
    break;
  }
  case ni::codegen::OperandType::REGISTER: {
    return serializeRegister(op.reg, op.size);
    break;
  }
  case ni::codegen::OperandType::OFFSET_REGISTER: {
    return serializeOffsetRegister(op.reg, op.size, op.offset);
    break;
  }
  };
}

void ni::codegen::ATTPrinter::openFile() {
  if (outputFile != nullptr) {
    throw new std::runtime_error("file already opened");
  }

  this->outputFile = new std::ofstream();
  this->outputFile->open(this->outputFilePath, std::ios::trunc);
}

bool ni::codegen::ATTPrinter::openedFile() const {
  return outputFile != nullptr;
}

void ni::codegen::ATTPrinter::checkFile() const {
  if (!this->openedFile()) {
    throw new std::runtime_error("no file opened");
  }
}

void ni::codegen::ATTPrinter::closeFile() {
  this->checkFile();

  this->outputFile->close();
  this->outputFile = nullptr;
}

void ni::codegen::ATTPrinter::textHeader() {
  this->checkFile();

  if (this->targetOS == ni::codegen::OS::MACOS) {
    *this->outputFile << ".section\t__TEXT,__text" << std::endl;
  } else {
    *this->outputFile << ".text" << std::endl;
  }
}

void ni::codegen::ATTPrinter::global(const std::string &label) {
  this->checkFile();

  if (this->targetOS == ni::codegen::OS::MACOS) {
    *this->outputFile << ".globl\t_" << label << std::endl;
  } else {
    *this->outputFile << ".globl\t" << label << std::endl;
  }
}

void ni::codegen::ATTPrinter::label(const std::string &label) {
  this->checkFile();

  if (this->targetOS == ni::codegen::OS::MACOS) {
    *this->outputFile << "_" << label << ":" << std::endl;
  } else {
    *this->outputFile << label << ":" << std::endl;
  }
}

std::string getSuffix(const int &size) {
  switch (size) {
  case 8: {
    return "b";
  }
  case 16: {
    return "w";
  }
  case 32: {
    return "l";
  }
  case 64: {
    return "q";
  }
  case 80: {
    return "t";
  }
  default: {
    throw std::runtime_error("invalid size");
  }
  }
}

void ni::codegen::ATTPrinter::mov(const Operand &dst, const Operand &src) {
  this->checkFile();

  *this->outputFile << "\tmov" << getSuffix(dst.size) << "\t"
                    << serializeOperand(src) << ", " << serializeOperand(dst)
                    << std::endl;
}

void ni::codegen::ATTPrinter::syscall() {
  this->checkFile();

  *this->outputFile << "\tsyscall" << std::endl;
}

void ni::codegen::ATTPrinter::add(const Operand &dst, const Operand &src) {
  this->checkFile();

  *this->outputFile << "\tadd" << getSuffix(dst.size) << "\t"
                    << serializeOperand(src) << ", " << serializeOperand(dst)
                    << std::endl;
}

void ni::codegen::ATTPrinter::sub(const Operand &dst, const Operand &src) {
  this->checkFile();

  *this->outputFile << "\tsub" << getSuffix(dst.size) << "\t"
                    << serializeOperand(src) << ", " << serializeOperand(dst)
                    << std::endl;
}

void ni::codegen::ATTPrinter::imul(const Operand &dst, const Operand &src) {
  this->checkFile();

  *this->outputFile << "\timu" << getSuffix(dst.size) << "\t"
                    << serializeOperand(src) << ", " << serializeOperand(dst)
                    << std::endl;
}

void ni::codegen::ATTPrinter::call(const std::string &label) {
  this->checkFile();

  if (this->targetOS == ni::codegen::OS::MACOS) {
    *this->outputFile << "\tcallq\t_" << label << std::endl;
  } else {
    *this->outputFile << "\tcallq\t" << label << std::endl;
  }
}

void ni::codegen::ATTPrinter::jmp(const std::string &label) {
  this->checkFile();

  if (this->targetOS == ni::codegen::OS::MACOS) {
    *this->outputFile << "\tjmpq\t_" << label << std::endl;
  } else {
    *this->outputFile << "\tjmpq\t" << label << std::endl;
  }
}

void ni::codegen::ATTPrinter::push(const Operand &src) {
  this->checkFile();

  *this->outputFile << "\tpushq\t" << serializeOperand(src) << std::endl;
}

void ni::codegen::ATTPrinter::pop(const Operand &dst) {
  this->checkFile();

  *this->outputFile << "\tpopq\t" << serializeOperand(dst) << std::endl;
}

void ni::codegen::ATTPrinter::ret() {
  this->checkFile();

  *this->outputFile << "\tretq" << std::endl;
}
