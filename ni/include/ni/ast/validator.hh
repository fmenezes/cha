#pragma once

#include <map>

#include "ast.hh"
#include "location.hh"
#include "visitor.hh"

namespace ni {
namespace ast {
class validator : public visitor {
public:
  static void validate(const program &node);

protected:
  void visit(const program &node) override;
  void visit(const variable_declaration &node) override;
  void visit(const variable_assignment &node) override;
  void visit(const variable_lookup &node) override;
  void visit(const function_declaration &node) override;
  void visit(const function_call &node) override;

private:
  std::map<std::string, const function_declaration &> functions;
  std::map<std::string, location> vars;
};
} // namespace ast
} // namespace ni
