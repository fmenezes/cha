#include "ast/ast.hh"

void ni::ast::Visitor::visit(const ni::ast::Node &node) {
  auto s = dynamic_cast<const ni::ast::NStatement *>(&node);
  if (s != nullptr) {
    return this->visit(*s);
  }

  auto fd = dynamic_cast<const ni::ast::NFunctionDeclaration *>(&node);
  if (fd != nullptr) {
    return this->visit(*fd);
  }

  auto p = dynamic_cast<const ni::ast::NProgram *>(&node);
  if (p != nullptr) {
    return this->visit(*p);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NStatement &node) {
  auto d = dynamic_cast<const ni::ast::NVariableDeclaration *>(&node);
  if (d != nullptr) {
    return this->visit(*d);
  }

  auto a = dynamic_cast<const ni::ast::NVariableAssignment *>(&node);
  if (a != nullptr) {
    return this->visit(*a);
  }

  auto fr = dynamic_cast<const ni::ast::NFunctionReturn *>(&node);
  if (fr != nullptr) {
    return this->visit(*fr);
  }

  auto e = dynamic_cast<const ni::ast::NExpression *>(&node);
  if (e != nullptr) {
    return this->visit(*e);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NExpression &node) {
  auto c = dynamic_cast<const ni::ast::NConstant *>(&node);
  if (c != nullptr) {
    return this->visit(*c);
  }

  auto b = dynamic_cast<const ni::ast::NBinaryOperation *>(&node);
  if (b != nullptr) {
    return this->visit(*b);
  }

  auto l = dynamic_cast<const ni::ast::NVariableLookup *>(&node);
  if (l != nullptr) {
    return this->visit(*l);
  }

  auto fc = dynamic_cast<const ni::ast::NFunctionCall *>(&node);
  if (fc != nullptr) {
    return this->visit(*fc);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NConstant &node) {
  auto i = dynamic_cast<const ni::ast::NConstantInteger *>(&node);
  if (i != nullptr) {
    return this->visit(*i);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NProgram &node) {
  for (auto &it : node.instructions) {
    this->visit(*it);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NVariableDeclaration &node) {}
void ni::ast::Visitor::visit(const ni::ast::NVariableAssignment &node) {
  this->visit(*node.value);
}
void ni::ast::Visitor::visit(const ni::ast::NVariableLookup &node) {}
void ni::ast::Visitor::visit(const ni::ast::NBinaryOperation &node) {
  this->visit(*node.left);
  this->visit(*node.right);
}
void ni::ast::Visitor::visit(const ni::ast::NFunctionDeclaration &node) {
  for (auto &it : node.body) {
    this->visit(*it);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NFunctionCall &node) {
  for (auto &it : node.params) {
    this->visit(*it);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NFunctionReturn &node) {
  if (node.value != nullptr) {
    this->visit(*node.value);
  }
}
void ni::ast::Visitor::visit(const ni::ast::NConstantInteger &node) {}
