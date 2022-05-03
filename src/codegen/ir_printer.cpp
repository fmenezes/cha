#include <fstream>
#include <iostream>

#include "codegen/ir_printer.hh"

void ni::codegen::ir_printer::text_header() { *out << ".text" << std::endl; }

void ni::codegen::ir_printer::global(const std::string &label) {
  *out << ".global\t" << label << std::endl;
}

void ni::codegen::ir_printer::label(const std::string &label) {
  *out << label << ":" << std::endl;
}

void ni::codegen::ir_printer::alloc(const ir_operand &dst, const int b) {
  *out << "\talloc\t" << dst.str() << ", " << b << std::endl;
}

void ni::codegen::ir_printer::mov(const ir_operand &dst,
                                  const ir_operand &src) {
  *out << "\tmov\t" << dst.str() << ", " << src.str() << std::endl;
}

void ni::codegen::ir_printer::syscall() { *out << "\tsyscall" << std::endl; }

void ni::codegen::ir_printer::add(const ir_operand &dst,
                                  const ir_operand &src) {
  *out << "\tadd\t" << dst.str() << ", " << src.str() << std::endl;
}

void ni::codegen::ir_printer::sub(const ir_operand &dst,
                                  const ir_operand &src) {
  *out << "\tsub\t" << dst.str() << ", " << src.str() << std::endl;
}

void ni::codegen::ir_printer::mul(const ir_operand &dst,
                                  const ir_operand &src) {
  *out << "\tmul\t" << dst.str() << ", " << src.str() << std::endl;
}

void ni::codegen::ir_printer::call(const std::string &label) {
  *out << "\tcall\t" << label << std::endl;
}

void ni::codegen::ir_printer::jmp(const std::string &label) {
  *out << "\tjmp\t" << label << std::endl;
}

void ni::codegen::ir_printer::push(const ir_operand &src) {
  *out << "\tpush\t" << src.str() << std::endl;
}

void ni::codegen::ir_printer::pop(const ir_operand &dst) {
  *out << "\tpop\t" << dst.str() << std::endl;
}

void ni::codegen::ir_printer::ret() { *out << "\tret" << std::endl; }
