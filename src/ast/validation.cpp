#include <map>
#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.hh"
#include "location.hh"
#include "parser.tab.hh"

std::string ni::ast::emitLocation(const yy::location &loc) {
  std::string ret = *loc.begin.filename + ":" + std::to_string(loc.begin.line) +
                    ":" + std::to_string(loc.begin.column);
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

void ni::ast::Validator::visit(const ni::ast::NFunctionCall &node) {
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

  ni::ast::Visitor::visit(node);
}

void ni::ast::Validator::visit(const ni::ast::NVariableLookup &node) {
  auto it = vars.find(node.identifier);
  if (it == vars.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" not defined");
  }

  ni::ast::Visitor::visit(node);
}

void ni::ast::Validator::visit(
    const ni::ast::NVariableAssignment &node) {
  auto it = this->vars.find(node.identifier);
  if (it == this->vars.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" not defined");
  }
  ni::ast::Visitor::visit(node);
}

void ni::ast::Validator::visit(
    const ni::ast::NVariableDeclaration &node) {
  auto it = this->vars.find(node.identifier);
  if (it != this->vars.end()) {
    throw yy::parser::syntax_error(node.location,
                                   "variable \"" + node.identifier +
                                       "\" already defined (at " +
                                       emitLocation(it->second) + ")");
  }
  this->vars.insert({node.identifier, node.location});

  ni::ast::Visitor::visit(node);
}

void ni::ast::Validator::visit(
    const ni::ast::NFunctionDeclaration &node) {
  this->vars.clear();

  for (auto &arg : node.args) {
    auto it = this->vars.find(arg->identifier);
    if (it != this->vars.end()) {
      throw yy::parser::syntax_error(
          arg->location, "argument \"" + arg->identifier +
                             "\" already defined (at " +
                             ni::ast::emitLocation(it->second) + ")");
    }
    this->vars.insert({arg->identifier, arg->location});
  }

  ni::ast::Visitor::visit(node);
}

void ni::ast::Validator::visit(const ni::ast::NProgram &node) {
  for (auto &n : node.instructions) {
    auto it = this->functions.find(n->identifier);
    if (it != this->functions.end()) {
      throw yy::parser::syntax_error(
          n->location, "function \"" + n->identifier +
                           "\" already defined (at " +
                           emitLocation(it->second.location) + ")");
    }
    this->functions.insert({n->identifier, *n});
  }

  ni::ast::Visitor::visit(node);
}

void ni::ast::Validator::validate(const ni::ast::NProgram &node) {
  ni::ast::Validator v;
  v.visit(node);
}
