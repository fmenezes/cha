#include <map>
#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.hh"
#include "ast/validator.hh"
#include "parse/syntax_parser.hh"

void ni::ast::validator::visit(const ni::ast::function_call &node) {
  auto it = functions.find(node.identifier);
  if (it == functions.end()) {
    throw ni::parse::syntax_error(node.loc, "function \"" + node.identifier +
                                                "\" not defined");
  }
  if (node.params.size() != it->second.args.size()) {
    throw ni::parse::syntax_error(
        node.loc, "incorrect number of arguments, function \"" +
                      node.identifier + "\" expects " +
                      std::to_string(it->second.args.size()) + " arguments, " +
                      std::to_string(node.params.size()) + " were provided");
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::variable_lookup &node) {
  auto it = vars.find(node.identifier);
  if (it == vars.end()) {
    throw ni::parse::syntax_error(node.loc, "variable \"" + node.identifier +
                                                "\" not defined");
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::variable_assignment &node) {
  auto it = this->vars.find(node.identifier);
  if (it == this->vars.end()) {
    throw ni::parse::syntax_error(node.loc, "variable \"" + node.identifier +
                                                "\" not defined");
  }
  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::variable_declaration &node) {
  auto it = this->vars.find(node.identifier);
  if (it != this->vars.end()) {
    throw ni::parse::syntax_error(node.loc, "variable \"" + node.identifier +
                                                "\" already defined (at " +
                                                it->second.str() + ")");
  }
  this->vars.insert({node.identifier, node.loc});

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::function_declaration &node) {
  this->vars.clear();

  for (auto &arg : node.args) {
    auto it = this->vars.find(arg->identifier);
    if (it != this->vars.end()) {
      throw ni::parse::syntax_error(arg->loc, "argument \"" + arg->identifier +
                                                  "\" already defined (at " +
                                                  it->second.str() + ")");
    }
    this->vars.insert({arg->identifier, arg->loc});
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::program &node) {
  for (auto &n : node.instructions) {
    auto it = this->functions.find(n->identifier);
    if (it != this->functions.end()) {
      throw ni::parse::syntax_error(n->loc, "function \"" + n->identifier +
                                                "\" already defined (at " +
                                                it->second.loc.str() + ")");
    }
    this->functions.insert({n->identifier, *n});
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::validate(const ni::ast::program &node) {
  ni::ast::validator v;
  v.visit(node);
}
