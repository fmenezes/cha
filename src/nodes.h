#include <iostream>
#include <string>

namespace ni
{
    class Node
    {
    public:
        virtual std::string to_string() const;
    };

    class NInteger : public Node
    {
    public:
        std::string value;
        NInteger(const std::string &value) : value(value){};

        virtual std::string to_string() const;
    };

    class NUnaryOperation : public Node
    {
    public:
        Node *value;
        std::string op;

        NUnaryOperation(const std::string &op, Node *value) : op(op), value(std::move(value)){};

        virtual std::string to_string() const;
    };

    class NBinaryOperation : public Node
    {
    public:
        Node *left;
        Node *right;
        std::string op;

        NBinaryOperation(const std::string &op, Node *left, Node *right) : op(op), left(std::move(left)), right(std::move(right)){};

        virtual std::string to_string() const;
    };

    class NVariableDeclaration : public Node
    {
    public:
        std::string identifier;
        NVariableDeclaration(const std::string &identifier) : identifier(identifier){};

        virtual std::string to_string() const;
    };

    class NVariableAssignment : public Node
    {
    public:
        std::string identifier;
        Node *value;
        NVariableAssignment(const std::string &identifier, Node *value) : identifier(identifier), value(std::move(value)){};

        virtual std::string to_string() const;
    };

    class NVariableLookup : public Node
    {
    public:
        std::string identifier;
        NVariableLookup(const std::string &identifier) : identifier(identifier){};

        virtual std::string to_string() const;
    };
}
