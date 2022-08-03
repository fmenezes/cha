#include <map>
#include <tuple>

#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include "ni/ast/ast.hh"
#include "ni/codegen/codegen.hh"

void ni::codegen::codegen::visit(const ni::ast::constant_integer &node) {
  auto v = std::stol(node.value);
  return_operand =
      llvm::ConstantInt::getSigned(llvm::IntegerType::get(*llvm_ctx, 32), v);
}

void ni::codegen::codegen::visit(const ni::ast::binary_operation &node) {
  ni::ast::visitor::visit(*node.left);
  auto *left_operand = return_operand;

  ni::ast::visitor::visit(*node.right);
  auto *right_operand = return_operand;

  if (node.op.compare("+") == 0) {
    return_operand = builder->CreateAdd(left_operand, right_operand);
  } else if (node.op.compare("-") == 0) {
    return_operand = builder->CreateSub(left_operand, right_operand);
  } else if (node.op.compare("*") == 0) {
    return_operand = builder->CreateMul(left_operand, right_operand);
  } else {
    throw std::runtime_error("unsupported binary operation '" + node.op + "'");
  }
}

void ni::codegen::codegen::visit(const ni::ast::variable_lookup &node) {
  std::tuple<llvm::Type *, llvm::AllocaInst *> variable = vars[node.identifier];
  llvm::Type *varType = std::get<0>(variable);
  llvm::AllocaInst *varPtr = std::get<1>(variable);
  return_operand = builder->CreateLoad(varType, varPtr);
}

void ni::codegen::codegen::visit(const ni::ast::variable_assignment &node) {
  ni::ast::visitor::visit(*node.value);

  std::tuple<llvm::Type *, llvm::AllocaInst *> variable = vars[node.identifier];
  llvm::AllocaInst *varPtr = std::get<1>(variable);
  builder->CreateStore(return_operand, varPtr);
}

void ni::codegen::codegen::visit(const ni::ast::variable_declaration &node) {
  vars.insert({node.identifier,
               std::make_tuple<llvm::Type *, llvm::AllocaInst *>(
                   llvm::Type::getInt32Ty(*llvm_ctx),
                   builder->CreateAlloca(llvm::Type::getInt32Ty(*llvm_ctx), 0,
                                         node.identifier))});
}

void ni::codegen::codegen::visit(const ni::ast::function_declaration &node) {

  vars.clear();

  llvm::Type *function_return_type = nullptr;
  if (node.return_type == nullptr) {
    function_return_type = llvm::Type::getVoidTy(*llvm_ctx);
  } else {
    function_return_type = llvm::Type::getInt32Ty(*llvm_ctx);
  }

  std::vector<llvm::Type *> function_arg_types;
  for (auto &arg : node.args) {
    function_arg_types.push_back(llvm::Type::getInt32Ty(*llvm_ctx));
  }

  llvm::FunctionType *func_type =
      llvm::FunctionType::get(function_return_type, function_arg_types, false);

  llvm::Function *F = llvm::Function::Create(
      func_type, llvm::Function::ExternalLinkage, node.identifier, *mod);
  int i = 0;
  for (auto &arg : F->args()) {
    arg.setName(node.args[i++]->identifier);
  }
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(*llvm_ctx, "entry", F);
  builder->SetInsertPoint(BB);

  for (auto &arg : F->args()) {
    llvm::Type *varType = llvm::Type::getInt32Ty(*llvm_ctx);
    llvm::AllocaInst *varPtr = builder->CreateAlloca(varType, 0, arg.getName());
    builder->CreateStore(&arg, varPtr);
    vars.insert({(std::string)arg.getName(),
                 std::make_tuple<llvm::Type *, llvm::AllocaInst *>(
                     std::move(varType), std::move(varPtr))});
  }

  ni::ast::visitor::visit(*node.body);

  if (llvm::verifyFunction(*F, &llvm::errs())) {
    F->eraseFromParent();
    throw std::runtime_error("function '" + node.identifier + "' is invalid");
  }

  funcs.insert({node.identifier, std::move(F)});
}

void ni::codegen::codegen::visit(const ni::ast::function_call &node) {

  std::vector<llvm::Value *> params;
  for (auto &param : node.params) {
    ni::ast::visitor::visit(*param);
    params.push_back(return_operand);
  }
  return_operand = builder->CreateCall(funcs[node.identifier], params);
}

void ni::codegen::codegen::visit(const ni::ast::function_return &node) {
  if (node.value == nullptr) {
    builder->CreateRetVoid();
    return;
  }
  ni::ast::visitor::visit(*node.value);
  builder->CreateRet(return_operand);
}

void ni::codegen::codegen::visit(const ni::ast::program &node) {
  for (auto &instruction : node.instructions) {
    ni::ast::visitor::visit(*instruction);
  }
}

ni::codegen::codegen::~codegen() {
  delete builder;
  delete mod;
  delete llvm_ctx;
}

void ni::codegen::codegen::generate(const std::string &output, format f) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetDisassembler();

  std::error_code ec;
  llvm::raw_fd_ostream file(output, ec, llvm::sys::fs::OpenFlags::OF_None);
  if (ec) {
    std::stringstream ss;
    ss << "could not open file '" << output << "': " << ec.message();
    throw std::runtime_error(ss.str());
  }

  llvm_ctx = new llvm::LLVMContext();
  mod = new llvm::Module("ni", *llvm_ctx);
  builder = new llvm::IRBuilder<>(*llvm_ctx);
  builder = new llvm::IRBuilder<>(*llvm_ctx);

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();

  auto CPU = "generic";
  auto Features = "";
  auto target_triple = llvm::sys::getDefaultTargetTriple();
  std::string Error;
  auto *target = llvm::TargetRegistry::lookupTarget(target_triple, Error);
  if (!target) {
    throw std::runtime_error("could not create target: " + Error);
  }
  auto *target_machine =
      target->createTargetMachine(target_triple, CPU, Features, opt, RM);

  mod->setDataLayout(target_machine->createDataLayout());
  mod->setTargetTriple(target_triple);

  ni::ast::visitor::visit(program);

  if (llvm::verifyModule(*mod, &llvm::errs())) {
    throw std::runtime_error("module verification failed");
  }

  if (f == ni::codegen::format::LLVM_IR) {
    mod->print(file, nullptr);
    file.flush();
    return;
  }

  // pass manager
  llvm::legacy::PassManager pass_manager;

  llvm::CodeGenFileType file_type = llvm::CGFT_ObjectFile;
  if (f == ni::codegen::format::ASSEMBLY_CODE) {
    file_type = llvm::CGFT_AssemblyFile;
  }

  if (target_machine->addPassesToEmitFile(pass_manager, file, nullptr,
                                          file_type)) {
    throw std::runtime_error("could not add passes to emit file");
  }

  pass_manager.run(*mod);
  file.flush();
}
