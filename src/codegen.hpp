#pragma once

#include "ast.hpp"
#include "cha/cha.hpp"
#include "exceptions.hpp"

#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>

#include <map>
#include <memory>

namespace cha {

// Code generation class that implements visitor pattern
class CodeGenerator : public AstVisitor {
public:
  CodeGenerator();
  ~CodeGenerator() = default;

  // Generate code from AST - throws CodeGenerationException on error
  void generate(const AstNodeList &ast, CompileFormat format,
                const std::string &output_file);

  // Visitor pattern implementation
  void visit(const ConstantIntegerNode &node) override;
  void visit(const ConstantUnsignedIntegerNode &node) override;
  void visit(const ConstantFloatNode &node) override;
  void visit(const ConstantBoolNode &node) override;
  void visit(const BinaryOpNode &node) override;
  void visit(const VariableDeclarationNode &node) override;
  void visit(const VariableAssignmentNode &node) override;
  void visit(const VariableLookupNode &node) override;
  void visit(const ArgumentNode &node) override;
  void visit(const BlockNode &node) override;
  void visit(const FunctionDeclarationNode &node) override;
  void visit(const FunctionCallNode &node) override;
  void visit(const FunctionReturnNode &node) override;
  void visit(const IfNode &node) override;
  void visit(const ConstantDeclarationNode &node) override;

private:
  // LLVM components
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;

  // Symbol table for variables and functions
  std::map<std::string, llvm::Value *> named_values_;
  std::map<std::string, llvm::Function *> functions_;

  // Current function being built
  llvm::Function *current_function_ = nullptr;

  // Stack of values from expression evaluation
  llvm::Value *current_value_ = nullptr;

  // Helper methods
  void visit_node(const AstNode &node);
  llvm::Type *get_llvm_type(const AstType &type);
  llvm::Type *primitive_to_llvm_type(PrimitiveType prim_type);
  void write_output(CompileFormat format, const std::string &output_file);
  void create_main_wrapper();
};

// Convenience function - throws CodeGenerationException on error
void generate_code(const AstNodeList &ast, CompileFormat format,
                   const std::string &output_file);

} // namespace cha
