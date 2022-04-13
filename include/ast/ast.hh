#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "location.hh"

namespace ni {
namespace ast {
class node {
public:
  const location loc;
  node(const location &location) : loc(location){};

  virtual ~node(){};
};

class statement : public node {
public:
  statement(const location &location) : node(location){};
};

class expression : public statement {
public:
  expression(const location &location) : statement(location){};
};

class constant : public expression {
public:
  constant(const location &location) : expression(location){};
};

class node_type : public node {
public:
  node_type(const location &location) : node(location){};
};

class integer : public node_type {
public:
  integer(const location &location) : node_type(location){};
};

class constant_integer : public constant {
public:
  const std::string value;
  constant_integer(const std::string &value, const location &location)
      : value(value), constant(location){};
};

class binary_operation : public expression {
public:
  const std::unique_ptr<expression> left;
  const std::unique_ptr<expression> right;
  const std::string op;

  binary_operation(const std::string &op, std::unique_ptr<expression> &left,
                   std::unique_ptr<expression> &right, const location &location)
      : op(op), left(std::move(left)), right(std::move(right)),
        expression(location){};
};

class variable_declaration : public statement {
public:
  const std::string identifier;
  const std::unique_ptr<node_type> type;
  variable_declaration(const std::string &identifier,
                       std::unique_ptr<node_type> &type,
                       const location &location)
      : identifier(identifier), type(std::move(type)), statement(location){};
};

class argument : public node {
public:
  const std::string identifier;
  const std::unique_ptr<node_type> type;
  argument(const std::string &identifier, std::unique_ptr<node_type> &type,
           const location &location)
      : identifier(identifier), type(std::move(type)), node(location){};
};

class variable_assignment : public statement {
public:
  const std::string identifier;
  const std::unique_ptr<expression> value;
  variable_assignment(const std::string &identifier,
                      std::unique_ptr<expression> &value,
                      const location &location)
      : identifier(identifier), value(std::move(value)), statement(location){};
};

class variable_lookup : public expression {
public:
  const std::string identifier;
  variable_lookup(const std::string &identifier, const location &location)
      : identifier(identifier), expression(location){};
};

class block : public node {
public:
  const std::vector<std::unique_ptr<statement>> statements;
  block(std::vector<std::unique_ptr<statement>> &statements,
        const location &location)
      : statements(std::move(statements)), node(location){};
};

class function_declaration : public node {
public:
  const std::string identifier;
  const std::unique_ptr<block> body;
  const std::unique_ptr<node_type> return_type;
  const std::vector<std::unique_ptr<argument>> args;
  function_declaration(const std::string &identifier,
                       std::unique_ptr<block> &body, const location &location)
      : identifier(identifier), body(std::move(body)), node(location){};
  function_declaration(const std::string &identifier,
                       std::vector<std::unique_ptr<argument>> &args,
                       std::unique_ptr<block> &body, const location &location)
      : identifier(identifier), args(std::move(args)), body(std::move(body)),
        node(location){};
  function_declaration(const std::string &identifier,
                       std::unique_ptr<node_type> &return_type,
                       std::unique_ptr<block> &body, const location &location)
      : identifier(identifier), return_type(std::move(return_type)),
        body(std::move(body)), node(location){};
  function_declaration(const std::string &identifier,
                       std::vector<std::unique_ptr<argument>> &args,
                       std::unique_ptr<node_type> &return_type,
                       std::unique_ptr<block> &body, const location &location)
      : identifier(identifier), return_type(std::move(return_type)),
        args(std::move(args)), body(std::move(body)), node(location){};
};

class function_call : public expression {
public:
  const std::string identifier;
  const std::vector<std::unique_ptr<expression>> params;
  function_call(const std::string &identifier, const location &location)
      : identifier(identifier), expression(location){};
  function_call(const std::string &identifier,
                std::vector<std::unique_ptr<expression>> &params,
                const location &location)
      : identifier(identifier), params(std::move(params)),
        expression(location){};
};

class function_return : public statement {
public:
  const std::unique_ptr<expression> value;
  function_return(std::unique_ptr<expression> &value, const location &location)
      : value(std::move(value)), statement(location){};
  function_return(const location &location) : statement(location){};
};

class program : public node {
public:
  const std::vector<std::unique_ptr<function_declaration>> instructions;
  program(std::vector<std::unique_ptr<function_declaration>> &instructions,
          const location &location)
      : instructions(std::move(instructions)), node(location){};
};
} // namespace ast
} // namespace ni
