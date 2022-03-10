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

enum OperandType { REGISTER, OFFSET_REGISTER, CONSTANT };

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
  Operand(Register8Bits reg)
      : type(OperandType::REGISTER), reg(reg), size(8), offset(0){};
  Operand(Register16Bits reg)
      : type(OperandType::REGISTER), reg(reg), size(16), offset(0){};
  Operand(Register32Bits reg)
      : type(OperandType::REGISTER), reg(reg), size(32), offset(0){};
  Operand(Register64Bits reg)
      : type(OperandType::REGISTER), reg(reg), size(64), offset(0){};
  Operand(Register8Bits reg, int offset)
      : type(OperandType::OFFSET_REGISTER), reg(reg), size(8), offset(offset){};
  Operand(Register16Bits reg, int offset)
      : type(OperandType::OFFSET_REGISTER), reg(reg), size(16),
        offset(offset){};
  Operand(Register32Bits reg, int offset)
      : type(OperandType::OFFSET_REGISTER), reg(reg), size(32),
        offset(offset){};
  Operand(Register64Bits reg, int offset)
      : type(OperandType::OFFSET_REGISTER), reg(reg), size(64),
        offset(offset){};
  Operand(std::string value)
      : value(value), type(OperandType::CONSTANT), size(0), offset(0), reg(0){};
  int sizeBytes() const { return size / 8; }
  const int size;
  const int offset;
  const int reg;
  const OperandType type;
  const std::string value;

  operator Register8Bits() const {
    if (size != 8 || (type != OperandType::REGISTER &&
                      type != OperandType::OFFSET_REGISTER)) {
      throw std::runtime_error("invalid convertion");
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
};

class ATTPrinter {
public:
  ATTPrinter(const std::string &outputFilePath, const OS &os, const ARCH &arch)
      : outputFilePath(outputFilePath), targetOS(os), targetArch(arch){};

  void openFile();
  bool openedFile() const;
  void closeFile();
  void textHeader();
  void global(const std::string &label);
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
  std::string outputFilePath;
  OS targetOS;
  ARCH targetArch;
  void checkFile() const;
};

} // namespace codegen
} // namespace ni
