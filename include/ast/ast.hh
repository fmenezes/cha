#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ni {
namespace ast {
class Node {
public:
  virtual ~Node(){};
};

class NStatement : public Node {};

class NExpression : public NStatement {};

class NConstant : public NExpression {};

class NType : public Node {};

class NInteger : public NType {
public:
  NInteger(){};
};

class NConstantInteger : public NConstant {
public:
  const std::string value;
  NConstantInteger(const std::string &value) : value(value){};
};

class NBinaryOperation : public NExpression {
public:
  const std::unique_ptr<NExpression> left;
  const std::unique_ptr<NExpression> right;
  const std::string op;

  NBinaryOperation(const std::string &op, std::unique_ptr<NExpression> &left,
                   std::unique_ptr<NExpression> &right)
      : op(op), left(std::move(left)), right(std::move(right)){};
};

class NVariableDeclaration : public NStatement {
public:
  const std::string identifier;
  const std::unique_ptr<NType> type;
  NVariableDeclaration(const std::string &identifier,
                       std::unique_ptr<NType> &type)
      : identifier(identifier), type(std::move(type)){};
};

class NArgument : public Node {
public:
  const std::string identifier;
  const std::unique_ptr<NType> type;
  NArgument(const std::string &identifier, std::unique_ptr<NType> &type)
      : identifier(identifier), type(std::move(type)){};
};

class NVariableAssignment : public NStatement {
public:
  const std::string identifier;
  const std::unique_ptr<NExpression> value;
  NVariableAssignment(const std::string &identifier,
                      std::unique_ptr<NExpression> &value)
      : identifier(identifier), value(std::move(value)){};
};

class NVariableLookup : public NExpression {
public:
  const std::string identifier;
  NVariableLookup(const std::string &identifier) : identifier(identifier){};
};

class NFunctionDeclaration : public Node {
public:
  const std::string identifier;
  const std::unique_ptr<NType> returnType;
  const std::vector<std::unique_ptr<NArgument>> args;
  const std::vector<std::unique_ptr<NStatement>> body;
  NFunctionDeclaration(const std::string &identifier,
                       std::vector<std::unique_ptr<NStatement>> &body)
      : identifier(identifier), body(std::move(body)){};
  NFunctionDeclaration(const std::string &identifier,
                       std::vector<std::unique_ptr<NArgument>> &args,
                       std::vector<std::unique_ptr<NStatement>> &body)
      : identifier(identifier), args(std::move(args)), body(std::move(body)){};
  NFunctionDeclaration(const std::string &identifier,
                       std::unique_ptr<NType> &returnType,
                       std::vector<std::unique_ptr<NStatement>> &body)
      : identifier(identifier), returnType(std::move(returnType)),
        body(std::move(body)){};
  NFunctionDeclaration(const std::string &identifier,
                       std::vector<std::unique_ptr<NArgument>> &args,
                       std::unique_ptr<NType> &returnType,
                       std::vector<std::unique_ptr<NStatement>> &body)
      : identifier(identifier), returnType(std::move(returnType)),
        args(std::move(args)), body(std::move(body)){};
};

class NFunctionCall : public NExpression {
public:
  const std::string identifier;
  const std::vector<std::unique_ptr<NExpression>> params;
  NFunctionCall(const std::string &identifier) : identifier(identifier){};
  NFunctionCall(const std::string &identifier,
                std::vector<std::unique_ptr<NExpression>> &params)
      : identifier(identifier), params(std::move(params)){};
};

class NFunctionReturn : public NStatement {
public:
  std::unique_ptr<NExpression> value;
  NFunctionReturn(std::unique_ptr<NExpression> &value)
      : value(std::move(value)){};
  NFunctionReturn(){};
};

class NProgram : public Node {
public:
  std::vector<std::unique_ptr<NFunctionDeclaration>> instructions;
  NProgram(){};
};

class Parser {
public:
  NProgram &program;
  static void parse(const std::string &f, NProgram &p) {
    Parser parser(p);
    parser.parse(f);
  };
  void parse(const std::string &f);
  Parser(NProgram &p) : program(p){};
};
} // namespace ast
} // namespace ni
