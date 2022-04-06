#include <map>
#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.hh"
#include "generated/location.hh"
#include "generated/parser.tab.hh"

std::string ni::ast::emit_location(const yy::location &loc) {
  std::string ret =
      std::to_string(loc.begin.line) + ":" + std::to_string(loc.begin.column);
  if (loc.begin.filename != nullptr) {
    ret = *loc.begin.filename + ":" + ret;
  }
  if (loc.begin.filename != nullptr && loc.end.filename != nullptr &&
      loc.begin.filename->compare(*loc.end.filename) != 0) {
    ret += "-" + *loc.end.filename + ":" + std::to_string(loc.end.line) + ":" +
           std::to_string(loc.end.column);
  } else if (loc.begin.line != loc.end.line) {
    ret += "-" + std::to_string(loc.end.line) + ":" +
           std::to_string(loc.end.column);
  } else if (loc.begin.column != loc.end.column) {
    ret += "-" + std::to_string(loc.end.column);
  }
  return ret;
}

void ni::ast::validator::visit(const ni::ast::function_call &node) {
  auto it = functions.find(node.identifier);
  if (it == functions.end()) {
    throw yy::parser::syntax_error(
        node.location, "function \"" + node.identifier + "\" not defined");
  }
  if (node.params.size() != it->second.args.size()) {
    throw yy::parser::syntax_error(
        node.location, "incorrect number of arguments, function \"" +
                           node.identifier + "\" expects " +
                           std::to_string(it->second.args.size()) +
                           " arguments, " + std::to_string(node.params.size()) +
                           " were provided");
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::variable_lookup &node) {
  auto it = vars.find(node.identifier);
  if (it == vars.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" not defined");
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::variable_assignment &node) {
  auto it = this->vars.find(node.identifier);
  if (it == this->vars.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" not defined");
  }
  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::variable_declaration &node) {
  auto it = this->vars.find(node.identifier);
  if (it != this->vars.end()) {
    throw yy::parser::syntax_error(node.location,
                                   "variable \"" + node.identifier +
                                       "\" already defined (at " +
                                       emit_location(it->second) + ")");
  }
  this->vars.insert({node.identifier, node.location});

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::function_declaration &node) {
  this->vars.clear();

  for (auto &arg : node.args) {
    auto it = this->vars.find(arg->identifier);
    if (it != this->vars.end()) {
      throw yy::parser::syntax_error(
          arg->location, "argument \"" + arg->identifier +
                             "\" already defined (at " +
                             ni::ast::emit_location(it->second) + ")");
    }
    this->vars.insert({arg->identifier, arg->location});
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::visit(const ni::ast::program &node) {
  for (auto &n : node.instructions) {
    auto it = this->functions.find(n->identifier);
    if (it != this->functions.end()) {
      throw yy::parser::syntax_error(
          n->location, "function \"" + n->identifier +
                           "\" already defined (at " +
                           emit_location(it->second.location) + ")");
    }
    this->functions.insert({n->identifier, *n});
  }

  ni::ast::visitor::visit(node);
}

void ni::ast::validator::validate(const ni::ast::program &node) {
  ni::ast::validator v;
  v.visit(node);
}
