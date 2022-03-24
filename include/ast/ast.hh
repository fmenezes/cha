#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "location.hh"

namespace ni {
namespace ast {
class Node {
public:
  const yy::location location;
  Node(const yy::location &location) : location(location){};

  virtual ~Node(){};
};

class NStatement : public Node {
public:
  NStatement(const yy::location &location) : Node(location){};
};

class NExpression : public NStatement {
public:
  NExpression(const yy::location &location) : NStatement(location){};
};

class NConstant : public NExpression {
public:
  NConstant(const yy::location &location) : NExpression(location){};
};

class NType : public Node {
public:
  NType(const yy::location &location) : Node(location){};
};

class NInteger : public NType {
public:
  NInteger(const yy::location &location) : NType(location){};
};

class NConstantInteger : public NConstant {
public:
  const std::string value;
  NConstantInteger(const std::string &value, const yy::location &location)
      : value(value), NConstant(location){};
};

class NBinaryOperation : public NExpression {
public:
  const std::unique_ptr<NExpression> left;
  const std::unique_ptr<NExpression> right;
  const std::string op;

  NBinaryOperation(const std::string &op, std::unique_ptr<NExpression> &left,
                   std::unique_ptr<NExpression> &right,
                   const yy::location &location)
      : op(op), left(std::move(left)), right(std::move(right)),
        NExpression(location){};
};

class NVariableDeclaration : public NStatement {
public:
  const std::string identifier;
  const std::unique_ptr<NType> type;
  NVariableDeclaration(const std::string &identifier,
                       std::unique_ptr<NType> &type,
                       const yy::location &location)
      : identifier(identifier), type(std::move(type)), NStatement(location){};
};

class NArgument : public Node {
public:
  const std::string identifier;
  const std::unique_ptr<NType> type;
  NArgument(const std::string &identifier, std::unique_ptr<NType> &type,
            const yy::location &location)
      : identifier(identifier), type(std::move(type)), Node(location){};
};

class NVariableAssignment : public NStatement {
public:
  const std::string identifier;
  const std::unique_ptr<NExpression> value;
  NVariableAssignment(const std::string &identifier,
                      std::unique_ptr<NExpression> &value,
                      const yy::location &location)
      : identifier(identifier), value(std::move(value)), NStatement(location){};
};

class NVariableLookup : public NExpression {
public:
  const std::string identifier;
  NVariableLookup(const std::string &identifier, const yy::location &location)
      : identifier(identifier), NExpression(location){};
};

class NFunctionDeclaration : public Node {
public:
  const std::string identifier;
  const std::unique_ptr<NType> returnType;
  const std::vector<std::unique_ptr<NArgument>> args;
  const std::vector<std::unique_ptr<NStatement>> body;
  NFunctionDeclaration(const std::string &identifier,
                       std::vector<std::unique_ptr<NStatement>> &body,
                       const yy::location &location)
      : identifier(identifier), body(std::move(body)), Node(location){};
  NFunctionDeclaration(const std::string &identifier,
                       std::vector<std::unique_ptr<NArgument>> &args,
                       std::vector<std::unique_ptr<NStatement>> &body,
                       const yy::location &location)
      : identifier(identifier), args(std::move(args)), body(std::move(body)),
        Node(location){};
  NFunctionDeclaration(const std::string &identifier,
                       std::unique_ptr<NType> &returnType,
                       std::vector<std::unique_ptr<NStatement>> &body,
                       const yy::location &location)
      : identifier(identifier), returnType(std::move(returnType)),
        body(std::move(body)), Node(location){};
  NFunctionDeclaration(const std::string &identifier,
                       std::vector<std::unique_ptr<NArgument>> &args,
                       std::unique_ptr<NType> &returnType,
                       std::vector<std::unique_ptr<NStatement>> &body,
                       const yy::location &location)
      : identifier(identifier), returnType(std::move(returnType)),
        args(std::move(args)), body(std::move(body)), Node(location){};
};

class NFunctionCall : public NExpression {
public:
  const std::string identifier;
  const std::vector<std::unique_ptr<NExpression>> params;
  NFunctionCall(const std::string &identifier, const yy::location &location)
      : identifier(identifier), NExpression(location){};
  NFunctionCall(const std::string &identifier,
                std::vector<std::unique_ptr<NExpression>> &params,
                const yy::location &location)
      : identifier(identifier), params(std::move(params)),
        NExpression(location){};
};

class NFunctionReturn : public NStatement {
public:
  const std::unique_ptr<NExpression> value;
  NFunctionReturn(std::unique_ptr<NExpression> &value,
                  const yy::location &location)
      : value(std::move(value)), NStatement(location){};
  NFunctionReturn(const yy::location &location) : NStatement(location){};
};

class NProgram : public Node {
public:
  const std::vector<std::unique_ptr<NFunctionDeclaration>> instructions;
  NProgram(std::vector<std::unique_ptr<NFunctionDeclaration>> &instructions,
           const yy::location &location)
      : instructions(std::move(instructions)), Node(location){};
};

class Parser {
public:
  yy::location location;
  std::unique_ptr<NProgram> program;
  void parse(const std::string &f);
};

class Visitor {
public:
  Visitor(){};
  virtual void visit(const Node &node);
  virtual void visit(const NProgram &node);
  virtual void visit(const NStatement &node);
  virtual void visit(const NExpression &node);
  virtual void visit(const NConstant &node);
  virtual void visit(const NConstantInteger &node);
  virtual void visit(const NVariableDeclaration &node);
  virtual void visit(const NVariableAssignment &node);
  virtual void visit(const NVariableLookup &node);
  virtual void visit(const NBinaryOperation &node);
  virtual void visit(const NFunctionDeclaration &node);
  virtual void visit(const NFunctionCall &node);
  virtual void visit(const NFunctionReturn &node);
};

class ValidatorVisitor : public Visitor {
public:
  ValidatorVisitor(){};
  void visit(const NProgram &node) override;
  void visit(const NVariableDeclaration &node) override;
  void visit(const NVariableAssignment &node) override;
  void visit(const NVariableLookup &node) override;
  void visit(const NFunctionDeclaration &node) override;
  void visit(const NFunctionCall &node) override;

private:
  std::map<std::string, const NFunctionDeclaration &> functions;
  std::map<std::string, yy::location> vars;
};

class Validator {
public:
  static void validate(const NProgram &node);
};

std::string emitLocation(const yy::location &loc);
} // namespace ast
} // namespace ni
