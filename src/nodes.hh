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
        virtual llvm::Value *codegen(NProgram &p) = 0;

        virtual ~Node() {};
    };

    class NInteger : public Node
    {
    public:
        std::string value;
        NInteger(const std::string &value) : value(value){};

        virtual llvm::Value *codegen(NProgram &p);
    };

    class NUnaryOperation : public Node
    {
    public:
        Node *value;
        std::string op;

        NUnaryOperation(const std::string &op, Node *value) : op(op), value(std::move(value)){};

        virtual llvm::Value *codegen(NProgram &p);
    };

    class NBinaryOperation : public Node
    {
    public:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::string op;

        NBinaryOperation(const std::string &op, std::unique_ptr<Node> &left, std::unique_ptr<Node> &right) : op(op), left(std::move(left)), right(std::move(right)){};

        virtual llvm::Value *codegen(NProgram &p);
    };

    class NVariableDeclaration : public Node
    {
    public:
        std::string identifier;
        NVariableDeclaration(const std::string &identifier) : identifier(identifier){};

        virtual llvm::Value *codegen(NProgram &p);
    };

    class NVariableAssignment : public Node
    {
    public:
        std::string identifier;
        std::unique_ptr<Node> value;
        NVariableAssignment(const std::string &identifier, std::unique_ptr<Node> &value) : identifier(identifier), value(std::move(value)){};

        virtual llvm::Value *codegen(NProgram &p);
    };

    class NVariableLookup : public Node
    {
    public:
        std::string identifier;
        NVariableLookup(const std::string &identifier) : identifier(identifier){};

        virtual llvm::Value *codegen(NProgram &p);
    };

    class NProgram
    {
    public:
        std::unique_ptr<llvm::LLVMContext> llvmContext;
        std::unique_ptr<llvm::Module> llvmModule;
        std::unique_ptr<llvm::IRBuilder<>> llvmIRBuilder;
        std::map<std::string, std::unique_ptr<llvm::AllocaInst>> vars;
        std::vector<std::unique_ptr<Node>> instructions;

        int parse();
        int parse(const std::string &f);
        int codegen(std::string &error);
    };

}
