#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

namespace ni {
class Node {
public:
  virtual ~Node(){};
};

class NInteger : public Node {
public:
  std::string value;
  NInteger(const std::string &value) : value(value){};
};

class NBinaryOperation : public Node {
public:
  std::unique_ptr<Node> left;
  std::unique_ptr<Node> right;
  std::string op;

  NBinaryOperation(const std::string &op, std::unique_ptr<Node> &left,
                   std::unique_ptr<Node> &right)
      : op(op), left(std::move(left)), right(std::move(right)){};
};

class NVariableDeclaration : public Node {
public:
  std::string identifier;
  NVariableDeclaration(const std::string &identifier)
      : identifier(identifier){};
};

class NVariableAssignment : public Node {
public:
  std::string identifier;
  std::unique_ptr<Node> value;
  NVariableAssignment(const std::string &identifier,
                      std::unique_ptr<Node> &value)
      : identifier(identifier), value(std::move(value)){};
};

class NVariableLookup : public Node {
public:
  std::string identifier;
  NVariableLookup(const std::string &identifier) : identifier(identifier){};
};

class NProgram : public Node {
public:
  std::vector<std::unique_ptr<Node>> instructions;
  int parse();
  int parse(const std::string &f);
};

enum OS { MACOS, LINUX };

enum ARCH { x86_64 };

class Codegen {
public:
  Codegen(const NProgram &p) : Codegen(p, defaultOs(), defaultArch()){};
  Codegen(const NProgram &p, const OS &os, const ARCH &arch)
      : program(p), targetOS(os), targetArch(arch){};
  virtual int codegen(const std::string &output, std::string &error) = 0;
  virtual ~Codegen() {}
  static OS defaultOs();
  static ARCH defaultArch();

protected:
  const NProgram &program;
  OS targetOS;
  ARCH targetArch;
};

class ASMCodegen : public Codegen {
public:
  ASMCodegen(const NProgram &p) : Codegen(p){};
  ASMCodegen(const NProgram &p, const OS &os, const ARCH &arch)
      : Codegen(p, os, arch){};
  virtual int codegen(const std::string &output, std::string &error);

private:
  std::ofstream *outputFile;
  std::map<std::string, int> vars;
  int currentStackPosition;
  int generateTextSection();
  int generateExitCall();
  int generateFunction(const std::string &name);
  int generateFunctionPrologue();
  int generateFunctionEpilogue();
  int internalCodegen(const ni::NProgram &node);
  int internalCodegen(const ni::NVariableLookup &node);
  int internalCodegen(const ni::NVariableDeclaration &node);
  int internalCodegen(const ni::NVariableAssignment &node);
  int internalCodegen(const ni::NBinaryOperation &node);
  int internalCodegen(const ni::NInteger &node);
  int internalCodegen(const ni::Node &node);
};
} // namespace ni
