#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

namespace ni
{
    class NProgram;

    class Node
    {
    public:
        virtual ~Node() {};
    };

    class NInteger : public Node
    {
    public:
        std::string value;
        NInteger(const std::string &value) : value(value){};
    };

    class NBinaryOperation : public Node
    {
    public:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::string op;

        NBinaryOperation(const std::string &op, std::unique_ptr<Node> &left, std::unique_ptr<Node> &right) : op(op), left(std::move(left)), right(std::move(right)){};
    };

    class NVariableDeclaration : public Node
    {
    public:
        std::string identifier;
        NVariableDeclaration(const std::string &identifier) : identifier(identifier){};
    };

    class NVariableAssignment : public Node
    {
    public:
        std::string identifier;
        std::unique_ptr<Node> value;
        NVariableAssignment(const std::string &identifier, std::unique_ptr<Node> &value) : identifier(identifier), value(std::move(value)){};
    };

    class NVariableLookup : public Node
    {
    public:
        std::string identifier;
        NVariableLookup(const std::string &identifier) : identifier(identifier){};
    };

    class NProgram : public Node
    {
    public:
        std::vector<std::unique_ptr<Node>> instructions;
        int parse();
        int parse(const std::string &f);
    };

    class Codegen
    {
    public:
        Codegen(const NProgram &p) : program(p){};
        virtual int codegen(std::string &error) = 0;

    protected:
        const NProgram &program;
    };

    class LLVMCodegen : public Codegen
    {
    public:
        LLVMCodegen(const NProgram &p) : Codegen(p){};
        virtual int codegen(std::string &error);

    private:
        std::unique_ptr<llvm::LLVMContext> llvmContext;
        std::unique_ptr<llvm::Module> llvmModule;
        std::unique_ptr<llvm::IRBuilder<>> llvmIRBuilder;
        std::map<std::string, std::unique_ptr<llvm::AllocaInst>> vars;
        llvm::Value *internalCodegen(const ni::Node &node);
        llvm::Value *internalCodegen(const ni::NInteger &node);
        llvm::Value *internalCodegen(const ni::NBinaryOperation &node);
        llvm::Value *internalCodegen(const ni::NVariableAssignment &node);
        llvm::Value *internalCodegen(const ni::NVariableDeclaration &node);
        llvm::Value *internalCodegen(const ni::NVariableLookup &node);
        llvm::Value *internalCodegen(const ni::NProgram &node);
    };
}
