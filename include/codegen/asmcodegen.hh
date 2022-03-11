#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast.hh"
#include "codegen/codegen.hh"

namespace ni {
namespace codegen {
class ASMCodegen : public Codegen {
public:
  ASMCodegen(const ni::ast::NProgram &p) : Codegen(p){};
  ASMCodegen(const ni::ast::NProgram &p, const Context &context)
      : Codegen(p, context){};
  virtual int codegen(const std::string &output, std::string &error);

private:
  std::ofstream *outputFile;
  std::map<std::string, std::string> vars;
  std::string currentFunctionName;
  int currentStackPosition;
  int generateTextSection();
  int generateExitCall();
  std::string generateFunctionName(const std::string &name) const;
  void resetStackFrame();
  int generateFunction(const std::string &name);
  int generateFunctionPrologue(const int memorySize);
  int generateFunctionEpilogue(const std::string &name, const int memorySize);
  int internalCodegen(const ni::ast::NProgram &node, std::string &returnAddr);
  int internalCodegen(const ni::ast::NFunctionDeclaration &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NFunctionCall &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NFunctionReturn &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NVariableLookup &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NVariableDeclaration &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NVariableAssignment &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NBinaryOperation &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::NConstantInteger &node,
                      std::string &returnAddr);
  int internalCodegen(const ni::ast::Node &node, std::string &returnAddr);
};
} // namespace codegen
} // namespace ni
