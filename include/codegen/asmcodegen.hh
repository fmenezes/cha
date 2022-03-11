#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast.hh"
#include "codegen/attprinter.hh"
#include "codegen/codegen.hh"

namespace ni {
namespace codegen {
class ASMCodegen : public Codegen {
public:
  ASMCodegen(const ni::ast::NProgram &p) : Codegen(p){};
  ASMCodegen(const ni::ast::NProgram &p, const Context &context)
      : Codegen(p, context), printer(context){};
  virtual int codegen(const std::string &output, std::string &error);
  ~ASMCodegen() { this->clearVars(); };

private:
  ATTPrinter printer;
  std::map<std::string, Operand *> vars;
  std::string currentFunctionName;
  int currentStackPosition;
  void generateExitCall();
  void clearVars();
  Operand internalCodegen(const ni::ast::NProgram &node);
  Operand internalCodegen(const ni::ast::NStatement &node);
  Operand internalCodegen(const ni::ast::NConstant &node);
  Operand internalCodegen(const ni::ast::NExpression &node);
  Operand internalCodegen(const ni::ast::NFunctionDeclaration &node);
  Operand internalCodegen(const ni::ast::NFunctionCall &node);
  Operand internalCodegen(const ni::ast::NFunctionReturn &node);
  Operand internalCodegen(const ni::ast::NVariableLookup &node);
  Operand internalCodegen(const ni::ast::NVariableDeclaration &node);
  Operand internalCodegen(const ni::ast::NVariableAssignment &node);
  Operand internalCodegen(const ni::ast::NBinaryOperation &node);
  Operand internalCodegen(const ni::ast::NConstantInteger &node);
  Operand internalCodegen(const ni::ast::Node &node);
};
} // namespace codegen
} // namespace ni
