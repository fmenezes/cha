#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Target/TargetMachine.h>

#include "ni/ast/ast.hh"
#include "ni/ast/visitor.hh"
#include "ni/codegen/context.hh"

namespace ni {
namespace codegen {
class codegen : public ni::ast::visitor {
public:
  codegen(const ni::ast::program &p) : program(p){};
  codegen(const ni::ast::program &p, const context &ctx)
      : program(p), ctx(ctx){};
  void generate(const std::string &outputFile, format format);
  virtual ~codegen();
  const context ctx;
  const ni::ast::program &program;

protected:
  void visit(const ni::ast::program &node) override;
  void visit(const ni::ast::function_declaration &node) override;
  void visit(const ni::ast::function_call &node) override;
  void visit(const ni::ast::function_return &node) override;
  void visit(const ni::ast::variable_lookup &node) override;
  void visit(const ni::ast::variable_declaration &node) override;
  void visit(const ni::ast::variable_assignment &node) override;
  void visit(const ni::ast::binary_operation &node) override;
  void visit(const ni::ast::constant_integer &node) override;

private:
  llvm::LLVMContext *llvm_ctx = nullptr;
  llvm::Module *mod = nullptr;
  llvm::IRBuilder<> *builder = nullptr;
  llvm::Value *return_operand = nullptr;
  std::map<std::string, std::tuple<llvm::Type *, llvm::AllocaInst *>> vars;
  std::map<std::string, llvm::Function *> funcs;
};
} // namespace codegen
} // namespace ni
