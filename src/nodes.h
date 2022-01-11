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
        Node *right;
        std::string op;

        NUnaryOperation(const std::string &op, Node *right) : op(op), right(std::move(right)){};

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
}
