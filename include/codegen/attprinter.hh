#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "codegen.hh"

namespace ni {
namespace codegen {

enum OperandType { NOP, REGISTER, OFFSET_REGISTER, CONSTANT };

enum Register8Bits {
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

enum Register16Bits {
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
  FLAGS
};

enum Register32Bits {
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
  EFLAGS
};

enum Register64Bits {
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

class Operand {
public:
  constexpr Operand(const Register8Bits &reg)
      : value(0), type(OperandType::REGISTER), reg(reg), size(8), offset(0){};
  constexpr Operand(const Register16Bits &reg)
      : value(0), type(OperandType::REGISTER), reg(reg), size(16), offset(0){};
  constexpr Operand(const Register32Bits &reg)
      : value(0), type(OperandType::REGISTER), reg(reg), size(32), offset(0){};
  constexpr Operand(const Register64Bits &reg)
      : value(0), type(OperandType::REGISTER), reg(reg), size(64), offset(0){};
  constexpr Operand(const Register8Bits &reg, int offset)
      : value(0), type(OperandType::OFFSET_REGISTER), reg(reg), size(8),
        offset(offset){};
  constexpr Operand(const Register16Bits &reg, int offset)
      : value(0), type(OperandType::OFFSET_REGISTER), reg(reg), size(16),
        offset(offset){};
  constexpr Operand(const Register32Bits &reg, int offset)
      : value(0), type(OperandType::OFFSET_REGISTER), reg(reg), size(32),
        offset(offset){};
  constexpr Operand(const Register64Bits &reg, int offset)
      : value(0), type(OperandType::OFFSET_REGISTER), reg(reg), size(64),
        offset(offset){};
  constexpr Operand(const int &value)
      : value(value), type(OperandType::CONSTANT), size(0), offset(0), reg(0){};
  constexpr Operand()
      : value(0), type(OperandType::NOP), size(0), offset(0), reg(0){};
  int getSize() const { return size; }
  int getSizeBytes() const { return size / 8; }
  OperandType getType() const { return type; }
  int getValue() const { return value; }
  int getRegister() const { return reg; }
  int getOffset() const { return offset; }

  void operator=(const ni::codegen::Operand &copy) {
    this->value = copy.value;
    this->type = copy.type;
    this->size = copy.size;
    this->offset = copy.offset;
    this->reg = copy.reg;
  }

  operator Register8Bits() const {
    if (size != 8 || (type != OperandType::REGISTER &&
                      type != OperandType::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (Register8Bits)reg;
  }
  operator Register16Bits() const {
    if (size != 16 || (type != OperandType::REGISTER &&
                       type != OperandType::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (Register16Bits)reg;
  }
  operator Register32Bits() const {
    if (size != 32 || (type != OperandType::REGISTER &&
                       type != OperandType::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (Register32Bits)reg;
  }
  operator Register64Bits() const {
    if (size != 64 || (type != OperandType::REGISTER &&
                       type != OperandType::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid conversion");
    }
    return (Register64Bits)reg;
  }
  bool operator==(Register8Bits a) const {
    return (size == 8 &&
            (type == OperandType::REGISTER ||
             type == OperandType::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator==(Register16Bits a) const {
    return (size == 16 &&
            (type == OperandType::REGISTER ||
             type == OperandType::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator==(Register32Bits a) const {
    return (size == 32 &&
            (type == OperandType::REGISTER ||
             type == OperandType::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator==(Register64Bits a) const {
    return (size == 32 &&
            (type == OperandType::REGISTER ||
             type == OperandType::OFFSET_REGISTER) &&
            reg == a);
  }
  bool operator!=(Register8Bits a) const { return !(*this == a); }
  bool operator!=(Register16Bits a) const { return !(*this == a); }
  bool operator!=(Register32Bits a) const { return !(*this == a); }
  bool operator!=(Register64Bits a) const { return !(*this == a); }

private:
  int size;
  int offset;
  int reg;
  OperandType type;
  int value;
};

class ATTPrinter {
public:
  ATTPrinter(){};
  ATTPrinter(const Context &context) : context(context){};

  const Context context;

  void openFile(const std::string &filePath);
  bool openedFile() const;
  void closeFile();
  void textHeader();
  void global(const std::string &label);
  void labelStart();
  void label(const std::string &label);
  void mov(const Operand &dst, const Operand &src);
  void add(const Operand &dst, const Operand &src);
  void sub(const Operand &dst, const Operand &src);
  void imul(const Operand &dst, const Operand &src);
  void jmp(const std::string &label);
  void call(const std::string &label);
  void push(const Operand &src);
  void pop(const Operand &dst);
  void syscall();
  void ret();

  ~ATTPrinter() {
    if (outputFile != nullptr) {
      closeFile();
    }
  };

private:
  std::ofstream *outputFile;
  void checkFile() const;
};

} // namespace codegen
} // namespace ni
