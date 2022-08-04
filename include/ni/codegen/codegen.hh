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
#include <llvm/Support/Host.h>
#include <llvm/Target/TargetMachine.h>

#include "ni/ast/ast.hh"
#include "ni/ast/visitor.hh"

namespace ni {
namespace codegen {

enum format { ASSEMBLY_CODE, OBJECT_FILE, LLVM_IR };

class codegen : public ni::ast::visitor {
public:
  codegen(const ni::ast::program &p)
      : program(p),
        target_triple(std::move(llvm::sys::getDefaultTargetTriple())){};
  codegen(const ni::ast::program &p, std::string target_triple)
      : program(p), target_triple(std::move(target_triple)){};
  void generate(const std::string &outputFile, format format);
  virtual ~codegen();

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
  std::string target_triple;
  const ni::ast::program &program;
  llvm::LLVMContext *ctx = nullptr;
  llvm::Module *mod = nullptr;
  llvm::IRBuilder<> *builder = nullptr;
  llvm::Value *return_operand = nullptr;
  std::map<std::string, std::tuple<llvm::Type *, llvm::AllocaInst *>> vars;
  std::map<std::string, llvm::Function *> funcs;
};
} // namespace codegen
} // namespace ni
