#include <fstream>
#include <iostream>

#include "codegen/ir/ir_printer.hh"

void ni::codegen::ir::ir_printer::global(const std::string &label) {
  *out << ".global\t" << label << std::endl;
}

void ni::codegen::ir::ir_printer::label(const std::string &label) {
  *out << label << ":" << std::endl;
}

void ni::codegen::ir::ir_printer::alloc(const ir_operand &dst, const int b) {
  *out << "\talloc\t" << dst.str() << ",\t" << b << std::endl;
}

void ni::codegen::ir::ir_printer::mov(const ir_operand &dst,
                                      const ir_operand &src) {
  *out << "\tmov\t" << dst.str() << ",\t" << src.str() << std::endl;
}

void ni::codegen::ir::ir_printer::exit(const ir_operand &src) {
  *out << "\texit\t" << src.str() << std::endl;
}

void ni::codegen::ir::ir_printer::add(const ir_operand &dst,
                                      const ir_operand &src,
                                      const ir_operand &src2) {
  *out << "\tadd\t" << dst.str() << ",\t" << src.str() << ",\t" << src2.str()
       << std::endl;
}

void ni::codegen::ir::ir_printer::sub(const ir_operand &dst,
                                      const ir_operand &src,
                                      const ir_operand &src2) {
  *out << "\tsub\t" << dst.str() << ",\t" << src.str() << ",\t" << src2.str()
       << std::endl;
}

void ni::codegen::ir::ir_printer::mul(const ir_operand &dst,
                                      const ir_operand &src,
                                      const ir_operand &src2) {
  *out << "\tmul\t" << dst.str() << ",\t" << src.str() << ",\t" << src2.str()
       << std::endl;
}

void ni::codegen::ir::ir_printer::call(const ir_operand &dst,
                                       const std::string &label) {
  *out << "\tcall\t" << dst.str() << ",\t" << label << std::endl;
}

void ni::codegen::ir::ir_printer::jmp(const std::string &label) {
  *out << "\tjmp\t" << label << std::endl;
}

void ni::codegen::ir::ir_printer::push(const ir_operand &src) {
  *out << "\tpush\t" << src.str() << std::endl;
}

void ni::codegen::ir::ir_printer::pop(const ir_operand &dst) {
  *out << "\tpop\t" << dst.str() << std::endl;
}

void ni::codegen::ir::ir_printer::ret(const ir_operand &src) {
  *out << "\tret\t" << src.str() << std::endl;
}
