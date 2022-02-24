#include <memory>
#include <sstream>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "nodes.hh"

int ni::LLVMCodegen::internalCodegen(const ni::Node &node,
                                     llvm::Value **value) {
  auto i = dynamic_cast<const ni::NInteger *>(&node);
  if (i != nullptr) {
    return this->internalCodegen(*i, value);
  }

  auto b = dynamic_cast<const ni::NBinaryOperation *>(&node);
  if (b != nullptr) {
    return this->internalCodegen(*b, value);
  }

  auto d = dynamic_cast<const ni::NVariableDeclaration *>(&node);
  if (d != nullptr) {
    return this->internalCodegen(*d, value);
  }

  auto a = dynamic_cast<const ni::NVariableAssignment *>(&node);
  if (a != nullptr) {
    return this->internalCodegen(*a, value);
  }

  auto l = dynamic_cast<const ni::NVariableLookup *>(&node);
  if (l != nullptr) {
    return this->internalCodegen(*l, value);
  }

  auto p = dynamic_cast<const ni::NProgram *>(&node);
  if (p != nullptr) {
    return this->internalCodegen(*p, value);
  }

  std::cerr << "Unknown node type" << std::endl;
  return 1;
}

int ni::LLVMCodegen::internalCodegen(const ni::NInteger &node,
                                     llvm::Value **value) {
  long long v = std::stoll(node.value);
  auto a = llvm::APInt(32, v);
  *value =
      llvm::ConstantInt::get(*this->llvmContext.get(), llvm::APSInt(a, false));
  return 0;
}

int ni::LLVMCodegen::internalCodegen(const ni::NBinaryOperation &node,
                                     llvm::Value **value) {
  llvm::Value *L, *R;
  int ret;
  ret = this->internalCodegen(*node.left.get(), &L);
  if (ret != 0) {
    return ret;
  }
  ret = this->internalCodegen(*node.right.get(), &R);
  if (ret != 0) {
    return ret;
  }
  if (node.op.compare("+") == 0) {
    *value = this->llvmIRBuilder->CreateAdd(L, R, "addtmp");
    return 0;
  } else if (node.op.compare("-") == 0) {
    *value = this->llvmIRBuilder->CreateSub(L, R, "subtmp");
    return 0;
  } else if (node.op.compare("*") == 0) {
    *value = this->llvmIRBuilder->CreateMul(L, R, "multmp");
    return 0;
  }

  std::cerr << "Invalid operation " << node.op << std::endl;
  return 1;
}

int ni::LLVMCodegen::internalCodegen(const ni::NVariableDeclaration &node,
                                     llvm::Value **value) {
  llvm::AllocaInst *allocaVar(this->llvmIRBuilder->CreateAlloca(
      llvm::Type::getInt32Ty(*this->llvmContext.get()), 0,
      node.identifier.c_str()));
  this->vars[node.identifier] = allocaVar;

  *value = this->llvmIRBuilder->CreateLoad(
      this->vars[node.identifier]->getAllocatedType(),
      this->vars[node.identifier], node.identifier.c_str());
  return 0;
}

int ni::LLVMCodegen::internalCodegen(const ni::NVariableAssignment &node,
                                     llvm::Value **value) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    std::cerr << node.identifier << " Not found." << std::endl;
    return 1;
  }
  auto ret = this->internalCodegen(*node.value.get(), value);
  if (ret != 0) {
    return ret;
  }
  this->llvmIRBuilder->CreateStore(*value, s->second);
  return 0;
}

int ni::LLVMCodegen::internalCodegen(const ni::NVariableLookup &node,
                                     llvm::Value **value) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    std::cerr << node.identifier << " Not found." << std::endl;
    return 1;
  }
  *value = this->llvmIRBuilder->CreateLoad(s->second->getAllocatedType(),
                                           s->second, node.identifier.c_str());
  return 0;
}

int ni::LLVMCodegen::internalCodegen(const ni::NProgram &node,
                                     llvm::Value **value) {
  std::vector<llvm::Type *> Args;

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(*this->llvmContext.get()), Args, false);

  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, 0, "main", this->llvmModule.get());

  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*this->llvmContext.get(), "entry", F);
  this->llvmIRBuilder->SetInsertPoint(BB);

  llvm::Value *last;
  for (auto &statement : program.instructions) {
    auto ret = this->internalCodegen(*statement.get(), &last);
    if (ret != 0) {
      return ret;
    }
  }

  this->llvmIRBuilder->CreateRet(last);
  *value = last;

  return 0;
}

int ni::LLVMCodegen::codegen(const std::string &output, std::string &error) {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  this->llvmContext = std::make_unique<llvm::LLVMContext>();
  this->llvmModule =
      std::make_unique<llvm::Module>("main", *this->llvmContext.get());
  this->llvmIRBuilder =
      std::make_unique<llvm::IRBuilder<>>(*this->llvmContext.get());
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  this->llvmModule->setTargetTriple(TargetTriple);
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, error);
  if (!Target) {
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";
  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  auto TheTargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  this->llvmModule->setDataLayout(TheTargetMachine->createDataLayout());

  std::error_code EC;
  llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::OF_Text);
  if (EC) {
    std::stringstream e;
    e << "Could not open file: " << EC.message();
    error = e.str();
    return 1;
  }

  llvm::Value *v;
  auto ret = this->internalCodegen(this->program, &v);

  this->llvmModule->print(dest, nullptr);

  dest.flush();
  dest.close();

  this->vars.clear();
  this->llvmIRBuilder.release();
  this->llvmModule.release();
  this->llvmContext.release();

  return ret;
}
