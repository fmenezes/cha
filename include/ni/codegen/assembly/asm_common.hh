#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ni {
namespace codegen {
namespace assembly {
enum asm_operand_type { NOP, REGISTER, OFFSET_REGISTER, CONSTANT, LABEL };

enum asm_register {
  NONE,

  // 8 bits
  DH,
  CH,
  BH,
  AH,
  AL,
  BL,
  CL,
  DL,
  SIL,
  DIL,
  BPL,
  SPL,
  R8B,
  R9B,
  R10B,
  R11B,
  R12B,
  R13B,
  R14B,
  R15B,

  // 16 bits
  AX,
  BX,
  CX,
  DX,
  SI,
  DI,
  BP,
  SP,
  R8W,
  R9W,
  R10W,
  R11W,
  R12W,
  R13W,
  R14W,
  R15W,
  IP,
  FLAGS,

  // 32 bits
  EAX,
  EBX,
  ECX,
  EDX,
  ESI,
  EDI,
  EBP,
  ESP,
  R8D,
  R9D,
  R10D,
  R11D,
  R12D,
  R13D,
  R14D,
  R15D,
  EIP,
  EFLAGS,

  // 64 bits
  RAX,
  RBX,
  RCX,
  RDX,
  RSI,
  RDI,
  RBP,
  RSP,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  RIP,
  RFLAGS
};

int register_size(asm_register reg);

class asm_operand {
public:
  explicit asm_operand(asm_register reg)
      : node_type(asm_operand_type::REGISTER), reg(std::move(reg)),
        size(register_size(reg)){};
  asm_operand(asm_register reg, int offset)
      : node_type(asm_operand_type::OFFSET_REGISTER), reg(std::move(reg)),
        offset(offset){};
  asm_operand(asm_operand_type node_type, std::string identifier)
      : node_type(node_type), identifier(std::move(identifier)) {
    if (node_type == asm_operand_type::LABEL ||
        node_type == asm_operand_type::CONSTANT) {
      size = 0;
    }
  };
  asm_operand(asm_operand_type node_type, std::string identifier, int size)
      : node_type(node_type), identifier(std::move(identifier)), size(size){};

  asm_operand_type get_type() const { return node_type; }
  std::string get_identifier() const { return identifier; }
  asm_register get_register() const { return reg; }
  int get_offset() const { return offset; }
  int get_size() const { return size; }

  operator asm_register() const {
    if (node_type != asm_operand_type::REGISTER) {
      throw std::runtime_error("invalid conversion");
    }
    return reg;
  }
  bool operator==(asm_register a) const {
    return node_type == asm_operand_type::REGISTER && reg == a;
  }
  bool operator!=(asm_register a) const {
    return node_type != asm_operand_type::REGISTER || reg != a;
    ;
  }

private:
  int offset = 0;
  int size = 32;
  asm_register reg = asm_register::NONE;
  asm_operand_type node_type = asm_operand_type::NOP;
  std::string identifier = "";
};

enum asm_operation {
  // directives
  TEXT_SECTION,
  GLOBAL,
  // instructions
  MOV,
  ADD,
  SUB,
  IMUL,
  JMP,
  CALL,
  RET,
  PUSH,
  POP,
  SYSCALL,
};

bool is_directive(asm_operation op);

class asm_instruction {
public:
  explicit asm_instruction(asm_operation operation)
      : operation(std::move(operation)){};
  asm_instruction(asm_operation operation, std::vector<asm_operand> operands)
      : operation(std::move(operation)), operands(std::move(operands)){};
  asm_instruction(std::string label, asm_operation operation,
                  std::vector<asm_operand> operands)
      : label(std::move(label)), operation(std::move(operation)),
        operands(std::move(operands)){};

  std::vector<asm_operand> operands;
  asm_operation operation;
  std::string label;
};

typedef std::vector<asm_instruction> asm_program;

const std::vector<ni::codegen::assembly::asm_operand>
    REGS({(ni::codegen::assembly::asm_operand)
              ni::codegen::assembly::asm_register::EDI,
          (ni::codegen::assembly::asm_operand)
              ni::codegen::assembly::asm_register::ESI,
          (ni::codegen::assembly::asm_operand)
              ni::codegen::assembly::asm_register::EDX,
          (ni::codegen::assembly::asm_operand)
              ni::codegen::assembly::asm_register::ECX,
          (ni::codegen::assembly::asm_operand)
              ni::codegen::assembly::asm_register::R8D,
          (ni::codegen::assembly::asm_operand)
              ni::codegen::assembly::asm_register::R9D});

} // namespace assembly
} // namespace codegen
} // namespace ni
