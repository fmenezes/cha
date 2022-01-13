#include <iostream>
#include <string>
#include <vector>

#include <llvm/IR/Value.h>

namespace ni
{
    class Node
    {
    public:
        virtual std::string to_string() const = 0;
        virtual llvm::Value* codegen() const = 0;
    };

    class NInteger : public Node
    {
    public:
        std::string value;
        NInteger(const std::string &value) : value(value){};

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NUnaryOperation : public Node
    {
    public:
        Node *value;
        std::string op;

        NUnaryOperation(const std::string &op, Node *value) : op(op), value(std::move(value)){};

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NBinaryOperation : public Node
    {
    public:
        Node *left;
        Node *right;
        std::string op;

        NBinaryOperation(const std::string &op, Node *left, Node *right) : op(op), left(std::move(left)), right(std::move(right)){};

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NVariableDeclaration : public Node
    {
    public:
        std::string identifier;
        NVariableDeclaration(const std::string &identifier) : identifier(identifier){};

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NStatementList : public Node
    {
    public:
        std::vector<Node *> statements;
        NStatementList(Node *statement)
        {
            statements.push_back(statement);
        }

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NVariableAssignment : public Node
    {
    public:
        std::string identifier;
        Node *value;
        NVariableAssignment(const std::string &identifier, Node *value) : identifier(identifier), value(std::move(value)){};

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NVariableLookup : public Node
    {
    public:
        std::string identifier;
        NVariableLookup(const std::string &identifier) : identifier(identifier){};

        virtual std::string to_string() const;
        virtual llvm::Value *codegen() const;
    };

    class NProgram
    {
    public:
        Node *value;
        NProgram(Node *value) : value(std::move(value)){};

        std::string to_string() const;

        void parse();
        llvm::Value *codegen() const;
    };
}
