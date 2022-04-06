#include <fstream>
#include <iostream>

#include "codegen/att_printer.hh"

std::string serialize_constant(const int &value) {
  return "$" + std::to_string(value);
}

std::string serialize_register(const ni::codegen::register_32bits &reg) {
  switch (reg) {
  case ni::codegen::register_32bits::EAX:
    return "%eax";
  case ni::codegen::register_32bits::EBX:
    return "%ebx";
  case ni::codegen::register_32bits::ECX:
    return "%ecx";
  case ni::codegen::register_32bits::EDX:
    return "%edx";
  case ni::codegen::register_32bits::ESI:
    return "%esi";
  case ni::codegen::register_32bits::EDI:
    return "%edi";
  case ni::codegen::register_32bits::EBP:
    return "%ebp";
  case ni::codegen::register_32bits::ESP:
    return "%esp";
  case ni::codegen::register_32bits::R8D:
    return "%r8d";
  case ni::codegen::register_32bits::R9D:
    return "%r9d";
  case ni::codegen::register_32bits::R10D:
    return "%r10d";
  case ni::codegen::register_32bits::R11D:
    return "%r11d";
  case ni::codegen::register_32bits::R12D:
    return "%r12d";
  case ni::codegen::register_32bits::R13D:
    return "%r13d";
  case ni::codegen::register_32bits::R14D:
    return "%r14d";
  case ni::codegen::register_32bits::R15D:
    return "%r15d";
  case ni::codegen::register_32bits::EIP:
    return "%eip";
  case ni::codegen::register_32bits::EFLAGS:
    return "%eflags";
  }

  throw std::runtime_error("invalid register");
}

std::string serialize_register(const ni::codegen::register_64bits &reg) {
  switch (reg) {
  case ni::codegen::register_64bits::RAX:
    return "%rax";
  case ni::codegen::register_64bits::RBX:
    return "%rbx";
  case ni::codegen::register_64bits::RCX:
    return "%rcx";
  case ni::codegen::register_64bits::RDX:
    return "%rdx";
  case ni::codegen::register_64bits::RSI:
    return "%rsi";
  case ni::codegen::register_64bits::RDI:
    return "%rdi";
  case ni::codegen::register_64bits::RBP:
    return "%rbp";
  case ni::codegen::register_64bits::RSP:
    return "%rsp";
  case ni::codegen::register_64bits::R8:
    return "%r8";
  case ni::codegen::register_64bits::R9:
    return "%r9";
  case ni::codegen::register_64bits::R10:
    return "%r10";
  case ni::codegen::register_64bits::R11:
    return "%r11";
  case ni::codegen::register_64bits::R12:
    return "%r12";
  case ni::codegen::register_64bits::R13:
    return "%r13";
  case ni::codegen::register_64bits::R14:
    return "%r14";
  case ni::codegen::register_64bits::R15:
    return "%r15";
  case ni::codegen::register_64bits::RIP:
    return "%rip";
  case ni::codegen::register_64bits::RFLAGS:
    return "%rflags";
  }

  throw std::runtime_error("invalid register");
}

std::string serialize_register(const int &reg, const int &size) {
  switch (size) {
  case 32: {
    return serialize_register((ni::codegen::register_32bits)reg);
  }
  case 64: {
    return serialize_register((ni::codegen::register_64bits)reg);
  }
  }

  throw std::runtime_error("invalid register");
}

std::string serialize_offset_register(const int &reg, const int &size,
                                      const int &offset) {
  std::string serializedReg = serialize_register(reg, size);
  if (offset == 0) {
    return "(" + serializedReg + ")";
  }

  return std::to_string(offset) + "(" + serializedReg + ")";
}

std::string serialize_operand(const ni::codegen::operand &op) {
  switch (op.get_type()) {
  case ni::codegen::operand_type::CONSTANT: {
    return serialize_constant(op.get_value());
    break;
  }
  case ni::codegen::operand_type::REGISTER: {
    return serialize_register(op.get_register(), op.get_size());
    break;
  }
  case ni::codegen::operand_type::OFFSET_REGISTER: {
    return serialize_offset_register(op.get_register(), op.get_size(),
                                     op.get_offset());
    break;
  }
  default: {
    throw new std::runtime_error("invalid operand");
  }
  };
}

void ni::codegen::att_printer::open_file(const std::string &filePath) {
  if (output_file != nullptr) {
    throw new std::runtime_error("file already opened");
  }

  this->output_file = new std::ofstream();
  this->output_file->open(filePath, std::ios::trunc);
}

bool ni::codegen::att_printer::opened_file() const {
  return output_file != nullptr;
}

void ni::codegen::att_printer::check_file() const {
  if (!this->opened_file()) {
    throw new std::runtime_error("no file opened");
  }
}

void ni::codegen::att_printer::close_file() {
  this->check_file();

  this->output_file->close();
  this->output_file = nullptr;
}

void ni::codegen::att_printer::text_header() {
  this->check_file();

  if (this->context.target_os == ni::codegen::os::MACOS) {
    *this->output_file << ".section\t__TEXT,__text" << std::endl;
  } else {
    *this->output_file << ".text" << std::endl;
  }
}

void ni::codegen::att_printer::global(const std::string &label) {
  this->check_file();

  if (this->context.target_os == ni::codegen::os::MACOS) {
    *this->output_file << ".globl\t_" << label << std::endl;
  } else {
    *this->output_file << ".globl\t" << label << std::endl;
  }
}

void ni::codegen::att_printer::label(const std::string &label) {
  this->check_file();

  if (this->context.target_os == ni::codegen::os::MACOS) {
    *this->output_file << "_" << label << ":" << std::endl;
  } else {
    *this->output_file << label << ":" << std::endl;
  }
}

void ni::codegen::att_printer::label_start() {
  this->check_file();

  if (this->context.target_os == ni::codegen::os::MACOS) {
    *this->output_file << ".globl\tstart" << std::endl;
    *this->output_file << "start:" << std::endl;
  } else {
    *this->output_file << ".globl\t_start" << std::endl;
    *this->output_file << "_start:" << std::endl;
  }
}

std::string get_suffix(const ni::codegen::operand &dst,
                       const ni::codegen::operand &src) {
  int size;
  if (src.get_size() > 0 && src.get_size() < dst.get_size()) {
    size = src.get_size();
  } else {
    size = dst.get_size();
  }

  switch (size) {
  case 8: { // byte
    return "b";
  }
  case 16: { // word
    return "w";
  }
  case 32: { // long
    return "l";
  }
  case 64: { // quad word
    return "q";
  }
  case 80: { // temporary decimal
    return "t";
  }
  default: {
    throw std::runtime_error("invalid size");
  }
  }
}

void ni::codegen::att_printer::mov(const operand &dst, const operand &src) {
  this->check_file();

  *this->output_file << "\tmov" << get_suffix(dst, src) << "\t"
                     << serialize_operand(src) << ", " << serialize_operand(dst)
                     << std::endl;
}

void ni::codegen::att_printer::syscall() {
  this->check_file();

  *this->output_file << "\tsyscall" << std::endl;
}

void ni::codegen::att_printer::add(const operand &dst, const operand &src) {
  this->check_file();

  *this->output_file << "\tadd" << get_suffix(dst, src) << "\t"
                     << serialize_operand(src) << ", " << serialize_operand(dst)
                     << std::endl;
}

void ni::codegen::att_printer::sub(const operand &dst, const operand &src) {
  this->check_file();

  *this->output_file << "\tsub" << get_suffix(dst, src) << "\t"
                     << serialize_operand(src) << ", " << serialize_operand(dst)
                     << std::endl;
}

void ni::codegen::att_printer::imul(const operand &dst, const operand &src) {
  this->check_file();

  *this->output_file << "\timul" << get_suffix(dst, src) << "\t"
                     << serialize_operand(src) << ", " << serialize_operand(dst)
                     << std::endl;
}

void ni::codegen::att_printer::call(const std::string &label) {
  this->check_file();

  if (this->context.target_os == ni::codegen::os::MACOS) {
    *this->output_file << "\tcallq\t_" << label << std::endl;
  } else {
    *this->output_file << "\tcallq\t" << label << std::endl;
  }
}

void ni::codegen::att_printer::jmp(const std::string &label) {
  this->check_file();

  if (this->context.target_os == ni::codegen::os::MACOS) {
    *this->output_file << "\tjmp\t_" << label << std::endl;
  } else {
    *this->output_file << "\tjmp\t" << label << std::endl;
  }
}

void ni::codegen::att_printer::push(const operand &src) {
  this->check_file();

  *this->output_file << "\tpushq\t" << serialize_operand(src) << std::endl;
}

void ni::codegen::att_printer::pop(const operand &dst) {
  this->check_file();

  *this->output_file << "\tpopq\t" << serialize_operand(dst) << std::endl;
}

void ni::codegen::att_printer::ret() {
  this->check_file();

  *this->output_file << "\tretq" << std::endl;
}
