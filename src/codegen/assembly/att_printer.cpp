#include <algorithm>
#include <fstream>
#include <iostream>

#include "ni/codegen/assembly/att_printer.hh"

static const std::string reg_names[] = {
    "none", "dh",   "ch",     "bh",  "ah",   "al",   "bl",   "cl",   "dl",
    "sil",  "dil",  "bpl",    "spl", "r8b",  "r9b",  "r10b", "r11b", "r12b",
    "r13b", "r14b", "r15b",   "ax",  "bx",   "cx",   "dx",   "si",   "di",
    "bp",   "sp",   "r8w",    "r9w", "r10w", "r11w", "r12w", "r13w", "r14w",
    "r15w", "ip",   "flags",  "eax", "ebx",  "ecx",  "edx",  "esi",  "edi",
    "ebp",  "esp",  "r8d",    "r9d", "r10d", "r11d", "r12d", "r13d", "r14d",
    "r15d", "eip",  "eflags", "rax", "rbx",  "rcx",  "rdx",  "rsi",  "rdi",
    "rbp",  "rsp",  "r8",     "r9",  "r10",  "r11",  "r12",  "r13",  "r14",
    "r15",  "rip",  "rflags"};

static const std::string asm_operation_str[] = {
    ".text", ".globl", "mov", "add",  "sub", "imul",
    "jmp",   "call",   "ret", "push", "pop", "syscall"};

std::string
get_suffix(const std::vector<ni::codegen::assembly::asm_operand> &operands) {
  int size = 0;
  for (auto &operand : operands) {
    if (operand.get_size() > size) {
      size = operand.get_size();
    }
  }
  if (size == 0) {
    size = 64;
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

std::string print_label(std::string label, const ni::codegen::context &ctx) {
  if (ctx.target_os == ni::codegen::os::LINUX && label.compare("start") == 0) {
    return "_start";
  }
  if (ctx.target_os == ni::codegen::os::MACOS && label.compare("start") != 0) {
    return "_" + label;
  }
  return label;
}

std::string ni::codegen::assembly::att_printer::print_operand(
    const ni::codegen::assembly::asm_operand &operand) const {
  switch (operand.get_type()) {
  case ni::codegen::assembly::asm_operand_type::CONSTANT: {
    return "$" + operand.get_identifier();
  }
  case ni::codegen::assembly::asm_operand_type::REGISTER: {
    return "%" + reg_names[operand.get_register()];
  }
  case ni::codegen::assembly::asm_operand_type::OFFSET_REGISTER: {
    std::string tmp = "(%" + reg_names[operand.get_register()] + ")";
    if (operand.get_offset() > 0) {
      return std::to_string(operand.get_offset()) + tmp;
    }
    return tmp;
  }
  case ni::codegen::assembly::asm_operand_type::LABEL: {
    return print_label(operand.get_identifier(), ctx);
  }
  default: {
    throw std::runtime_error("invalid operand type");
  }
  }
}

void print_operation(std::ostream &out,
                     const ni::codegen::assembly::asm_instruction &inst,
                     const ni::codegen::context &ctx) {
  if (ctx.target_os == ni::codegen::os::MACOS &&
      inst.operation == ni::codegen::assembly::asm_operation::TEXT_SECTION) {
    out << ".section\t__TEXT,__text";
    return;
  }

  if (!is_directive(inst.operation)) {
    out << "\t";
  }

  out << asm_operation_str[inst.operation];

  if (!is_directive(inst.operation) &&
      inst.operation != ni::codegen::assembly::asm_operation::SYSCALL &&
      inst.operation != ni::codegen::assembly::asm_operation::JMP) {
    out << get_suffix(inst.operands);
  }
}

void ni::codegen::assembly::att_printer::print(std::ostream &out) const {
  for (auto &inst : p) {
    if (inst.label.length() > 0) {
      out << print_label(inst.label, ctx) << ":";
    }
    print_operation(out, inst, ctx);

    if (inst.operands.size() > 0) {
      std::vector<ni::codegen::assembly::asm_operand> operands;
      std::copy(inst.operands.begin(), inst.operands.end(),
                std::back_inserter(operands));
      if (operands.size() > 1) {
        std::rotate(operands.begin(),
                    operands.end() -
                        1, // ATT syntax moves the destination oprand to last
                    operands.end());
      }
      for (auto it = operands.begin(); it != operands.end(); ++it) {
        if (it == operands.begin()) // first element
        {
          out << "\t";
        } else {
          out << ",\t";
        }
        out << print_operand(*it);
      }
    }
    out << std::endl;
  }
}

std::ostream &operator<<(std::ostream &os,
                         const ni::codegen::assembly::att_printer &printer) {
  printer.print(os);
  return os;
}
