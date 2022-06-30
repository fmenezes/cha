#pragma once

#include "ast.hh"

namespace ni {
namespace ast {
class visitor {
public:
  visitor(){};

protected:
  virtual void visit(const node &node);
  virtual void visit(const program &node);
  virtual void visit(const statement &node);
  virtual void visit(const expression &node);
  virtual void visit(const constant &node);
  virtual void visit(const constant_integer &node);
  virtual void visit(const variable_declaration &node);
  virtual void visit(const variable_assignment &node);
  virtual void visit(const variable_lookup &node);
  virtual void visit(const binary_operation &node);
  virtual void visit(const function_declaration &node);
  virtual void visit(const function_call &node);
  virtual void visit(const function_return &node);
  virtual void visit(const block &node);
  virtual void visit(const argument &node);
};
} // namespace ast
} // namespace ni
