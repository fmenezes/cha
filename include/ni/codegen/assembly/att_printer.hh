#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "ni/codegen/codegen.hh"

namespace ni {
namespace codegen {
namespace assembly {

enum operand_type { NOP, REGISTER, OFFSET_REGISTER, CONSTANT };

enum register_8bits {
  DH = -4,
  CH,
  BH,
  AH,
  AL = 1,
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
  R15B
};

enum register_16bits {
  AX = 1,
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
  FLAGS
};

enum register_32bits {
  EAX = 1,
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
  EFLAGS
};

enum register_64bits {
  RAX = 1,
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

class operand {
public:
  constexpr operand(const register_8bits &reg)
      : value(0), node_type(operand_type::REGISTER), reg(reg), size(8),
        offset(0){};
  constexpr operand(const register_16bits &reg)
      : value(0), node_type(operand_type::REGISTER), reg(reg), size(16),
        offset(0){};
  constexpr operand(const register_32bits &reg)
      : value(0), node_type(operand_type::REGISTER), reg(reg), size(32),
        offset(0){};
  constexpr operand(const register_64bits &reg)
      : value(0), node_type(operand_type::REGISTER), reg(reg), size(64),
        offset(0){};
  constexpr operand(const register_8bits &reg, int offset)
      : value(0), node_type(operand_type::OFFSET_REGISTER), reg(reg), size(8),
        offset(offset){};
  constexpr operand(const register_16bits &reg, int offset)
      : value(0), node_type(operand_type::OFFSET_REGISTER), reg(reg), size(16),
        offset(offset){};
  constexpr operand(const register_32bits &reg, int offset)
      : value(0), node_type(operand_type::OFFSET_REGISTER), reg(reg), size(32),
        offset(offset){};
  constexpr operand(const register_64bits &reg, int offset)
      : value(0), node_type(operand_type::OFFSET_REGISTER), reg(reg), size(64),
        offset(offset){};
  constexpr operand(const int &value)
      : value(value), node_type(operand_type::CONSTANT), size(0), offset(0),
        reg(0){};
  constexpr operand()
      : value(0), node_type(operand_type::NOP), size(0), offset(0), reg(0){};
  operand(const operand &op)
      : value(op.value), node_type(op.node_type), size(op.size),
        offset(op.offset), reg(op.reg){};
  int get_size() const { return size; }
  int get_size_bytes() const { return size / 8; }
  operand_type get_type() const { return node_type; }
  int get_value() const { return value; }
  int get_register() const { return reg; }
  int get_offset() const { return offset; }

  operator register_8bits() const {
    if (size != 8 || (node_type != operand_type::REGISTER &&
                      node_type != operand_type::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (register_8bits)reg;
  }
  operator register_16bits() const {
    if (size != 16 || (node_type != operand_type::REGISTER &&
                       node_type != operand_type::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (register_16bits)reg;
  }
  operator register_32bits() const {
    if (size != 32 || (node_type != operand_type::REGISTER &&
                       node_type != operand_type::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (register_32bits)reg;
  }
  operator register_64bits() const {
    if (size != 64 || (node_type != operand_type::REGISTER &&
                       node_type != operand_type::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (register_64bits)reg;
  }
  bool operator==(register_8bits a) const {
    return (size == 8 &&
            (node_type == operand_type::REGISTER ||
             node_type == operand_type::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator==(register_16bits a) const {
    return (size == 16 &&
            (node_type == operand_type::REGISTER ||
             node_type == operand_type::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator==(register_32bits a) const {
    return (size == 32 &&
            (node_type == operand_type::REGISTER ||
             node_type == operand_type::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator==(register_64bits a) const {
    return (size == 32 &&
            (node_type == operand_type::REGISTER ||
             node_type == operand_type::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator!=(register_8bits a) const { return !(*this == a); }
  bool operator!=(register_16bits a) const { return !(*this == a); }
  bool operator!=(register_32bits a) const { return !(*this == a); }
  bool operator!=(register_64bits a) const { return !(*this == a); }

private:
  int size;
  int offset;
  int reg;
  operand_type node_type;
  int value;
};

class att_printer {
public:
  att_printer(){};
  att_printer(const context &ctx) : ctx(ctx){};

  const context ctx;

  void open_file(const std::string &filePath);
  bool opened_file() const;
  void close_file();
  void text_header();
  void global(const std::string &label);
  void label_start();
  void label(const std::string &label);
  void mov(const operand &dst, const operand &src);
  void add(const operand &dst, const operand &src);
  void sub(const operand &dst, const operand &src);
  void imul(const operand &dst, const operand &src);
  void jmp(const std::string &label);
  void call(const std::string &label);
  void push(const operand &src);
  void pop(const operand &dst);
  void syscall();
  void ret();

  ~att_printer() {
    if (output_file != nullptr) {
      close_file();
    }
  };

private:
  std::ofstream *output_file = nullptr;
  void check_file() const;
};

} // namespace assembly
} // namespace codegen
} // namespace ni
