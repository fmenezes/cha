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

llvm::Value *ni::NInteger::codegen(llvm::LLVMContext *ctx) const
{
    return llvm::ConstantInt::get(*ctx, llvm::APSInt(this->value));
}

llvm::Value *ni::NUnaryOperation::codegen(llvm::LLVMContext *ctx) const
{
    return NULL;
}

llvm::Value *ni::NBinaryOperation::codegen(llvm::LLVMContext *ctx) const
{
    return NULL;
}

llvm::Value *ni::NVariableDeclaration::codegen(llvm::LLVMContext *ctx) const
{
    return NULL;
}

llvm::Value *ni::NVariableAssignment::codegen(llvm::LLVMContext *ctx) const
{
    return NULL;
}

llvm::Value *ni::NVariableLookup::codegen(llvm::LLVMContext *ctx) const
{
    return NULL;
}

llvm::Value *ni::NStatementList::codegen(llvm::LLVMContext *ctx) const
{
    llvm::Value *last;
    for (auto &statement : this->statements)
    {
        last = statement->codegen(ctx);
    }
    return last;
}

int ni::NProgram::codegen(std::string& error) const
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    llvm::LLVMContext *TheContext = new llvm::LLVMContext();
    llvm::Module *TheModule = new llvm::Module("main", *TheContext);
    llvm::IRBuilder<>* Builder = new llvm::IRBuilder<>(*TheContext);
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, error);
    if (!Target)
    {
        return 1;
    }

    auto CPU = "x86-64";
    auto Features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC)
    {
        std::stringstream e;
        e << "Could not open file: " << EC.message();
        error = e.str();
        return 1;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        std::stringstream e;
        e << "Could not open file: " << EC.message();
        error = e.str();
        return 1;
    }

    pass.run(*TheModule);
    dest.flush();

    return 0;
}
