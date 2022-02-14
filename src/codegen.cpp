#include <stdio.h>
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

#include "lld/Common/Driver.h"
#include "lld/Common/ErrorHandler.h"
#include "lld/Common/Memory.h"

#include "nodes.hh"
#include "parser.tab.hh"

int ni::NProgram::parse()
{
    yy::parser p(*this);
    return p.parse();
}

llvm::Value *ni::NInteger::codegen(ni::NProgram &p)
{
    long long v = std::stoll(this->value);
    auto a = llvm::APInt(32, v);
    return llvm::ConstantInt::get(*p.llvmContext.get(), llvm::APSInt(a, false));
}

llvm::Value *ni::NBinaryOperation::codegen(ni::NProgram &p)
{
    auto L = this->left->codegen(p);
    auto R = this->right->codegen(p);

    if (this->op.compare("+") == 0)
    {
        return p.llvmIRBuilder->CreateAdd(L, R, "addtmp");
    }
    else if (this->op.compare("-") == 0)
    {
        return p.llvmIRBuilder->CreateSub(L, R, "subtmp");
    }
    else if (this->op.compare("*") == 0)
    {
        return p.llvmIRBuilder->CreateMul(L, R, "multmp");
    }

    return NULL;
}

llvm::Value *ni::NVariableDeclaration::codegen(ni::NProgram &p)
{
    std::unique_ptr<llvm::AllocaInst> allocaVar(p.llvmIRBuilder->CreateAlloca(llvm::Type::getInt32Ty(*p.llvmContext.get()), 0, this->identifier.c_str()));
    p.vars[this->identifier] = std::move(allocaVar);

    return p.llvmIRBuilder->CreateLoad(p.vars[this->identifier]->getAllocatedType(), p.vars[this->identifier].get(), this->identifier.c_str());
}

llvm::Value *ni::NVariableAssignment::codegen(ni::NProgram &p)
{
    auto s = p.vars.find(this->identifier);
    if (s == p.vars.end())
    {
        std::cerr << this->identifier << " Not found." << std::endl;
        exit(1);
    }
    auto value = this->value->codegen(p);
    p.llvmIRBuilder->CreateStore(value, s->second.get());
    return value;
}

llvm::Value *ni::NVariableLookup::codegen(ni::NProgram &p)
{
    auto s = p.vars.find(this->identifier);
    if (s == p.vars.end())
    {
        std::cerr << this->identifier << " Not found." << std::endl;
        exit(1);
    }
    return p.llvmIRBuilder->CreateLoad(s->second->getAllocatedType(), s->second.get(), this->identifier.c_str());
}

std::string runCommand(const char *cmd)
{
    std::shared_ptr<FILE>
        pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get()))
    {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

std::string getMacOSSDK()
{
    return runCommand("xcrun --show-sdk-path");
}

std::string getMacOSSDKVersion()
{
    return runCommand("xcrun --show-sdk-version");
}

int ni::NProgram::codegen(std::string &error)
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    this->llvmContext = std::make_unique<llvm::LLVMContext>();
    this->llvmModule = std::make_unique<llvm::Module>("main", *this->llvmContext.get());
    this->llvmIRBuilder = std::make_unique<llvm::IRBuilder<>>(*this->llvmContext.get());
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    this->llvmModule->setTargetTriple(TargetTriple);
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

    this->llvmModule->setDataLayout(TheTargetMachine->createDataLayout());

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
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*this->llvmContext.get()), Args, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, 0, "main", this->llvmModule.get());

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*this->llvmContext.get(), "entry", F);
    this->llvmIRBuilder->SetInsertPoint(BB);

    llvm::Value *last;
    for (auto &statement : this->instructions)
    {
        last = statement->codegen(*this);
    }

    this->llvmIRBuilder->CreateRet(last);

    this->llvmModule->print(llDest, nullptr);

    pass.run(*this->llvmModule.get());

    dest.flush();
    dest.close();
    llDest.flush();
    llDest.close();

    this->vars.clear();
    this->llvmIRBuilder.release();
    this->llvmModule.release();
    this->llvmContext.release();

    auto outFilename = "a.out";
    llvm::raw_fd_ostream outDest(outFilename, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        std::stringstream e;
        e << "Could not open file: " << EC.message();
        error = e.str();
        return 1;
    }

    std::vector<const char *> args;
    if (TheTargetMachine->getTargetTriple().getOS() == llvm::Triple::OSType::Darwin)
    {
        args.push_back("-dynamic");
        args.push_back("-arch");
        args.push_back(TheTargetMachine->getTargetTriple().getArchName().str().c_str());
        args.push_back("-platform_version");
        args.push_back("macos");
        args.push_back(getMacOSSDKVersion().c_str());
        args.push_back(getMacOSSDKVersion().c_str());
        args.push_back("-o");
        args.push_back(outFilename);
        args.push_back(destFilename);
        args.push_back("-lc");
        args.push_back("-syslibroot");
        args.push_back(getMacOSSDK().c_str());
        lld::macho::link(args, false, llvm::outs(), llvm::errs());
    }
    else
    {
        args.push_back("-arch");
        args.push_back(TheTargetMachine->getTargetTriple().getArchName().str().c_str());
        args.push_back("-o");
        args.push_back(outFilename);
        args.push_back(destFilename);
        lld::elf::link(llvm::makeArrayRef(args), false, llvm::outs(), llvm::errs());
    }

    return 0;
}
