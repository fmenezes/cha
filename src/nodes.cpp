#include <sstream>
#include <iostream>

#include "nodes.h"
#include "parser.tab.h"

std::string ni::Node::to_string() const
{
    std::stringstream o;
    o << "[Node]";
    return o.str();
}

std::string ni::NInteger::to_string() const
{
    std::stringstream o;
    o << "[NInteger " << this->value << "]";
    return o.str();
}

std::string ni::NBinaryOperation::to_string() const
{
    std::stringstream o;
    o << "[NBinaryOperation " << this->left->to_string() << " " << this->op << " " << this->right->to_string() << "]";
    return o.str();
}

std::string ni::NUnaryOperation::to_string() const
{
    std::stringstream o;
    o << "[NUnaryOperation " << this->op << " " << this->value->to_string() << "]";
    return o.str();
}

std::string ni::NVariableDeclaration::to_string() const
{
    std::stringstream o;
    o << "[NVariableDeclaration " << this->identifier << "]";
    return o.str();
}

std::string ni::NVariableAssignment::to_string() const
{
    std::stringstream o;
    o << "[NVariableAssignment " << this->identifier << " " << this->value->to_string() << "]";
    return o.str();
}

std::string ni::NVariableLookup::to_string() const
{
    std::stringstream o;
    o << "[NVariableLookup " << this->identifier << "]";
    return o.str();
}

std::string ni::NStatementList::to_string() const
{
    std::stringstream o;
    o << "[NStatementList";
    for(auto i : this->statements) {
        o << "\n " << i->to_string();
    }
    o << "]";
    return o.str();
}

std::string ni::NProgram::to_string() const
{
    std::stringstream o;
    o << "[NProgram " << this->value->to_string() << "]";
    return o.str();
}

void ni::NProgram::parse() {
    yyparse();
}