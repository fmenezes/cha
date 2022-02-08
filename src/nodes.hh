#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"

namespace ni
{

    class Context
    {
    public:
        llvm::LLVMContext *ctx;
        llvm::raw_fd_ostream *llFile;
        llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter> *builder;
        std::map<std::string, llvm::AllocaInst *> vars;
        Context(llvm::raw_fd_ostream *llFile, llvm::LLVMContext *ctx, llvm::IRBuilder<> *builder) : llFile(llFile), ctx(ctx), builder(builder){};
    };
    class Node
    {
    public:
        virtual llvm::Value *codegen(Context *ctx) const = 0;

        virtual ~Node() {};
    };

    class NInteger : public Node
    {
    public:
        std::string value;
        NInteger(const std::string &value) : value(value){};

        virtual llvm::Value *codegen(Context *ctx) const;
    };

    class NUnaryOperation : public Node
    {
    public:
        Node *value;
        std::string op;

        NUnaryOperation(const std::string &op, Node *value) : op(op), value(std::move(value)){};

        virtual llvm::Value *codegen(Context *ctx) const;
    };

    class NBinaryOperation : public Node
    {
    public:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::string op;

        NBinaryOperation(const std::string &op, std::unique_ptr<Node> &left, std::unique_ptr<Node> &right) : op(op), left(std::move(left)), right(std::move(right)){};

        virtual llvm::Value *codegen(Context *ctx) const;
    };

    class NVariableDeclaration : public Node
    {
    public:
        std::string identifier;
        NVariableDeclaration(const std::string &identifier) : identifier(identifier){};

        virtual llvm::Value *codegen(Context *ctx) const;
    };

    class NVariableAssignment : public Node
    {
    public:
        std::string identifier;
        std::unique_ptr<Node> value;
        NVariableAssignment(const std::string &identifier, std::unique_ptr<Node> &value) : identifier(identifier), value(std::move(value)){};

        virtual llvm::Value *codegen(Context *ctx) const;
    };

    class NVariableLookup : public Node
    {
    public:
        std::string identifier;
        NVariableLookup(const std::string &identifier) : identifier(identifier){};

        virtual llvm::Value *codegen(Context *ctx) const;
    };

    class NProgram
    {
    public:
        std::vector<std::unique_ptr<Node>> instructions;

        int parse();
        int parse(const std::string &f);
        int codegen(std::string &error) const;
    };

}
