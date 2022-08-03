#pragma once

#include <map>
#include <string>
#include <tuple>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "ni/ast/ast.hh"
#include "ni/ast/visitor.hh"
#include "ni/codegen/codegen.hh"

namespace ni {
namespace codegen {
namespace llvmir {
class llvmir_codegen : public ni::ast::visitor, public codegen {
public:
  llvmir_codegen(const ni::ast::program &p) : codegen(p){};
  void generate(const std::string &output) override;
  virtual ~llvmir_codegen() override;

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
  llvm::LLVMContext *ctx = nullptr;
  llvm::Module *mod = nullptr;
  llvm::IRBuilder<> *builder = nullptr;
  llvm::Value *return_operand = nullptr;
  std::map<std::string, std::tuple<llvm::Type *, llvm::AllocaInst *>> vars;
  std::map<std::string, llvm::Function *> funcs;
};

} // namespace llvmir
} // namespace codegen
} // namespace ni
