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

#include "nodes.hh"
#include "parser.tab.hh"

int ni::NProgram::parse()
{
    yy::parser p(*this);
    return p.parse();
}

llvm::Value *ni::Codegen::internalCodegen(const ni::NProgram &p, const ni::Node &node){
    auto i = dynamic_cast<const ni::NInteger *>(&node);
    if (i != nullptr)
    {
        return this->internalCodegen(p, *i);
    }

    auto b = dynamic_cast<const ni::NBinaryOperation *>(&node);
    if (b != nullptr)
    {
        return this->internalCodegen(p, *b);
    }

    auto d = dynamic_cast<const ni::NVariableDeclaration *>(&node);
    if (d != nullptr)
    {
        return this->internalCodegen(p, *d);
    }

    auto a = dynamic_cast<const ni::NVariableAssignment *>(&node);
    if (a != nullptr)
    {
        return this->internalCodegen(p, *a);
    }

    auto l = dynamic_cast<const ni::NVariableLookup *>(&node);
    if (l != nullptr)
    {
        return this->internalCodegen(p, *l);
    }

    throw std::runtime_error("Unkown node type");
}

llvm::Value *ni::Codegen::internalCodegen(const ni::NProgram &p, const ni::NInteger &node)
{
    long long v = std::stoll(node.value);
    auto a = llvm::APInt(32, v);
    return llvm::ConstantInt::get(*this->llvmContext.get(), llvm::APSInt(a, false));
}

llvm::Value *ni::Codegen::internalCodegen(const ni::NProgram &p, const ni::NBinaryOperation &node)
{
    auto L = this->internalCodegen(p, *node.left.get());
    auto R = this->internalCodegen(p, *node.right.get());

    if (node.op.compare("+") == 0)
    {
        return this->llvmIRBuilder->CreateAdd(L, R, "addtmp");
    }
    else if (node.op.compare("-") == 0)
    {
        return this->llvmIRBuilder->CreateSub(L, R, "subtmp");
    }
    else if (node.op.compare("*") == 0)
    {
        return this->llvmIRBuilder->CreateMul(L, R, "multmp");
    }

    return NULL;
}

llvm::Value *ni::Codegen::internalCodegen(const ni::NProgram &p, const ni::NVariableDeclaration &node)
{
    std::unique_ptr<llvm::AllocaInst> allocaVar(this->llvmIRBuilder->CreateAlloca(llvm::Type::getInt32Ty(*this->llvmContext.get()), 0, node.identifier.c_str()));
    this->vars[node.identifier] = std::move(allocaVar);

    return this->llvmIRBuilder->CreateLoad(this->vars[node.identifier]->getAllocatedType(), this->vars[node.identifier].get(), node.identifier.c_str());
}

llvm::Value *ni::Codegen::internalCodegen(const ni::NProgram &p, const ni::NVariableAssignment &node)
{
    auto s = this->vars.find(node.identifier);
    if (s == this->vars.end())
    {
        std::cerr << node.identifier << " Not found." << std::endl;
        exit(1);
    }
    auto value = this->internalCodegen(p, *node.value.get());
    this->llvmIRBuilder->CreateStore(value, s->second.get());
    return value;
}

llvm::Value *ni::Codegen::internalCodegen(const ni::NProgram &p, const ni::NVariableLookup &node)
{
    auto s = this->vars.find(node.identifier);
    if (s == this->vars.end())
    {
        std::cerr << node.identifier << " Not found." << std::endl;
        exit(1);
    }
    return this->llvmIRBuilder->CreateLoad(s->second->getAllocatedType(), s->second.get(), node.identifier.c_str());
}

int ni::Codegen::codegen(std::string &error)
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
    for (auto &statement : program.instructions)
    {
        last = this->internalCodegen(this->program, *statement.get());
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

    return 0;
}
