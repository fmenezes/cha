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
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "nodes.h"

llvm::Value *ni::NInteger::codegen(ni::Context *ctx) const
{
    return NULL; // llvm::ConstantInt::get(*ctx->ctx, llvm::APSInt(this->value));
}

llvm::Value *ni::NUnaryOperation::codegen(ni::Context *ctx) const
{
    return NULL;
}

llvm::Value *ni::NBinaryOperation::codegen(ni::Context *ctx) const
{
    auto L = this->left->codegen(ctx);
    auto R = this->right->codegen(ctx);
    if (this->op.compare("+") == 0)
    {
        return ctx->builder->CreateAdd(L, R, "addtmp");
    }
    else if (this->op.compare("-") == 0)
    {
        return ctx->builder->CreateSub(L, R, "subtmp");
    }
    else if (this->op.compare("*") == 0)
    {
        return ctx->builder->CreateMul(L, R, "multmp");
    }
    return NULL;
}

llvm::Value *ni::NVariableDeclaration::codegen(ni::Context *ctx) const
{
    // llvm::AllocaInst *Alloca = ctx->builder->CreateAlloca(llvm::Type::getInt8Ty(*ctx->ctx), 0, this->identifier.c_str());
    return NULL;
}

llvm::Value *ni::NVariableAssignment::codegen(ni::Context *ctx) const
{
    return NULL;
}

llvm::Value *ni::NVariableLookup::codegen(ni::Context *ctx) const
{
    return NULL;
}

llvm::Value *ni::NStatementList::codegen(ni::Context *ctx) const
{
    llvm::Value *last;
    for (auto &statement : this->statements)
    {
        last = statement->codegen(ctx);
    }
    return last;
}

int ni::NProgram::codegen(std::string &error) const
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    llvm::LLVMContext *TheContext = new llvm::LLVMContext();
    llvm::Module *TheModule = new llvm::Module("main", *TheContext);
    llvm::IRBuilder<> *Builder = new llvm::IRBuilder<>(*TheContext);
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, error);
    if (!Target)
    {
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    auto destFilename = "output.o";
    llvm::raw_fd_ostream dest(destFilename, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        std::stringstream e;
        e << "Could not open file: " << EC.message();
        error = e.str();
        return 1;
    }

    auto llFilename = "output.ll";
    llvm::raw_fd_ostream llDest(llFilename, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        std::stringstream e;
        e << "Could not open file: " << EC.message();
        error = e.str();
        return 1;
    }

    llvm::legacy::PassManager pass;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile))
    {
        std::stringstream e;
        e << "Could not open file: " << EC.message();
        error = e.str();
        return 1;
    }

    std::vector<llvm::Type *> Args;

    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), Args, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, 0, "main", TheModule);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    std::vector<llvm::Type *> Args;

    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(*TheContext), Args, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, 0, "main", TheModule);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    ni::Context ctx(&llDest, TheContext, Builder);

    auto value = this->value->codegen(&ctx);

    Builder->CreateRetVoid();

    pass.run(*TheModule);

    TheModule->print(llDest, nullptr);

    dest.flush();
    dest.close();
    llDest.flush();
    llDest.close();

    return 0;
}
