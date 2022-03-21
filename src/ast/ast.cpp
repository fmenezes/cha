#include <map>
#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.hh"
#include "location.hh"
#include "parser.tab.hh"

extern FILE *yyin;

void ni::ast::Parser::parse(const std::string &f) {
  if (!(yyin = fopen(f.c_str(), "r"))) {
    throw std::runtime_error("cannot open " + f);
  }

  std::string *fp = new std::string(f);
  this->location = yy::location(fp);
  yy::parser p(*this);
  int r = p.parse();
  fclose(yyin);
  if (r != 0) {
    throw std::runtime_error("error parsing");
  }
}

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

void ni::ast::Validator::validate(const ni::ast::Node &node) {
  auto s = dynamic_cast<const ni::ast::NStatement *>(&node);
  if (s != nullptr) {
    return this->validate(*s);
  }

  auto fd = dynamic_cast<const ni::ast::NFunctionDeclaration *>(&node);
  if (fd != nullptr) {
    return this->validate(*fd);
  }

  auto p = dynamic_cast<const ni::ast::NProgram *>(&node);
  if (p != nullptr) {
    return this->validate(*p);
  }
}

void ni::ast::Validator::validate(const ni::ast::NStatement &node) {
  auto d = dynamic_cast<const ni::ast::NVariableDeclaration *>(&node);
  if (d != nullptr) {
    return this->validate(*d);
  }

  auto a = dynamic_cast<const ni::ast::NVariableAssignment *>(&node);
  if (a != nullptr) {
    return this->validate(*a);
  }

  auto fr = dynamic_cast<const ni::ast::NFunctionReturn *>(&node);
  if (fr != nullptr) {
    return this->validate(*fr);
  }

  auto e = dynamic_cast<const ni::ast::NExpression *>(&node);
  if (e != nullptr) {
    return this->validate(*e);
  }
}

void ni::ast::Validator::validate(const ni::ast::NExpression &node) {
  auto b = dynamic_cast<const ni::ast::NBinaryOperation *>(&node);
  if (b != nullptr) {
    return this->validate(*b);
  }

  auto l = dynamic_cast<const ni::ast::NVariableLookup *>(&node);
  if (l != nullptr) {
    return this->validate(*l);
  }

  auto fc = dynamic_cast<const ni::ast::NFunctionCall *>(&node);
  if (fc != nullptr) {
    return this->validate(*fc);
  }
}

void ni::ast::Validator::validate(const ni::ast::NFunctionCall &node) {
  auto it = functionNames.find(node.identifier);
  if (it == functionNames.end()) {
    throw yy::parser::syntax_error(node.location, "function \"" + node.identifier + "\" not defined");
  }
}

void ni::ast::Validator::validate(const ni::ast::NFunctionReturn &node) {
  if (node.value != nullptr) {
    this->validate(*node.value);
  }
}

void ni::ast::Validator::validate(const ni::ast::NVariableLookup &node) {
  auto it = vars.find(node.identifier);
  if (it == functionNames.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" not defined");
  }
}

void ni::ast::Validator::validate(const ni::ast::NVariableAssignment &node) {
  auto it = this->vars.find(node.identifier);
  if (it == this->vars.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" not defined");
  }
}

void ni::ast::Validator::validate(const ni::ast::NBinaryOperation &node) {
  this->validate(*node.left);
  this->validate(*node.right);
}

void ni::ast::Validator::validate(const ni::ast::NVariableDeclaration &node) {
  auto it = this->vars.find(node.identifier);
  if (it != this->vars.end()) {
    throw yy::parser::syntax_error(
        node.location, "variable \"" + node.identifier + "\" already defined (at " +  emitLocation(it->second) + ")");
  }
  this->vars.insert({node.identifier, node.location});
}

void ni::ast::Validator::validate(const ni::ast::NFunctionDeclaration &n) {
  this->vars.clear();

  for (auto &arg : n.args) {
    auto it = this->vars.find(arg->identifier);
    if (it != this->vars.end()) {
      throw yy::parser::syntax_error(
          arg->location, "argument \"" + arg->identifier +
                             "\" already defined (at " +
                             ni::ast::emitLocation(it->second) + ")");
    }
    this->vars.insert({arg->identifier, arg->location});
  }

  for (auto &node : n.body) {
    this->validate(*node);
  }
}

void ni::ast::Validator::validate(const ni::ast::NProgram &p) {
  for (auto &n : p.instructions) {
    auto it = this->functionNames.find(n->identifier);
    if (it != this->functionNames.end()) {
      throw yy::parser::syntax_error(n->location,
                                     "function \"" + n->identifier +
                                         "\" already defined (at " +
                                         emitLocation(it->second) + ")");
    }
    this->functionNames.insert({n->identifier, n->location});
  }

  for (auto &n : p.instructions) {
    validate(*n);
  }
}

void ni::ast::Validator::validate() { this->validate(this->program); }
