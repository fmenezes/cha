#include <fstream>
#include <iostream>

#include "ni/codegen/ir/ir_printer.hh"

static const std::string ir_operation_str[] = {
    ".GLOBAL", "ALLOC", "MOV",  "ADD", "SUB", "MUL",
    "JMP",     "CALL",  "PUSH", "POP", "RET", "EXIT"};

std::string ir_operand_to_str(const ni::codegen::ir::ir_operand &op) {
  switch (op.get_operand_type()) {
  case ni::codegen::ir::ir_operand_type::CONSTANT:
    return "$" + op.get_identifier();
  case ni::codegen::ir::ir_operand_type::TEMPORARY:
    return "%" + op.get_identifier();
  case ni::codegen::ir::ir_operand_type::MEMORY:
    return "&" + op.get_identifier();
  default:
    return op.get_identifier();
  }
}

void ni::codegen::ir::ir_printer::print(std::ostream &out) const {
  for (auto &inst : program) {
    if (inst.label.length() > 0) {
      out << inst.label << ":";
    }
    out << "\t" << ir_operation_str[inst.operation];
    for (auto it = inst.operands.begin(); it != inst.operands.end(); ++it) {
      if (it == inst.operands.begin()) // first element
      {
        out << "\t";
      } else {
        out << ",\t";
      }
      out << ir_operand_to_str(*it);
    }
    out << std::endl;
  }
}

std::ostream &operator<<(std::ostream &os,
                         const ni::codegen::ir::ir_printer &printer) {
  printer.print(os);
  return os;
}
