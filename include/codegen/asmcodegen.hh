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
class ASMCodegen : public ni::ast::Visitor, public Codegen {
public:
  ASMCodegen(const ni::ast::NProgram &p) : Codegen(p){};
  ASMCodegen(const ni::ast::NProgram &p, const Context &context)
      : Codegen(p, context), printer(context){};
  void codegen(const std::string &output) override;

protected:
  void visit(const ni::ast::NProgram &node) override;
  void visit(const ni::ast::NFunctionDeclaration &node) override;
  void visit(const ni::ast::NFunctionCall &node) override;
  void visit(const ni::ast::NFunctionReturn &node) override;
  void visit(const ni::ast::NVariableLookup &node) override;
  void visit(const ni::ast::NVariableDeclaration &node) override;
  void visit(const ni::ast::NVariableAssignment &node) override;
  void visit(const ni::ast::NBinaryOperation &node) override;
  void visit(const ni::ast::NConstantInteger &node) override;

private:
  ATTPrinter printer;
  std::map<std::string, Operand> vars;
  std::string currentFunctionName;
  int currentStackPosition;
  void generateExitCall();
  void generateStartFunction();
  Operand returnOperand;
};
} // namespace codegen
} // namespace ni
