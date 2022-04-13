#include "ast/visitor.hh"
#include "ast/ast.hh"

void ni::ast::visitor::visit(const ni::ast::node &node) {
  auto a = dynamic_cast<const ni::ast::argument *>(&node);
  if (a != nullptr) {
    return this->visit(*a);
  }

  auto s = dynamic_cast<const ni::ast::statement *>(&node);
  if (s != nullptr) {
    return this->visit(*s);
  }

  auto b = dynamic_cast<const ni::ast::block *>(&node);
  if (b != nullptr) {
    return this->visit(*b);
  }

  auto fd = dynamic_cast<const ni::ast::function_declaration *>(&node);
  if (fd != nullptr) {
    return this->visit(*fd);
  }

  auto p = dynamic_cast<const ni::ast::program *>(&node);
  if (p != nullptr) {
    return this->visit(*p);
  }
}
void ni::ast::visitor::visit(const ni::ast::statement &node) {
  auto d = dynamic_cast<const ni::ast::variable_declaration *>(&node);
  if (d != nullptr) {
    return this->visit(*d);
  }

  auto a = dynamic_cast<const ni::ast::variable_assignment *>(&node);
  if (a != nullptr) {
    return this->visit(*a);
  }

  auto fr = dynamic_cast<const ni::ast::function_return *>(&node);
  if (fr != nullptr) {
    return this->visit(*fr);
  }

  auto e = dynamic_cast<const ni::ast::expression *>(&node);
  if (e != nullptr) {
    return this->visit(*e);
  }
}
void ni::ast::visitor::visit(const ni::ast::expression &node) {
  auto c = dynamic_cast<const ni::ast::constant *>(&node);
  if (c != nullptr) {
    return this->visit(*c);
  }

  auto b = dynamic_cast<const ni::ast::binary_operation *>(&node);
  if (b != nullptr) {
    return this->visit(*b);
  }

  auto l = dynamic_cast<const ni::ast::variable_lookup *>(&node);
  if (l != nullptr) {
    return this->visit(*l);
  }

  auto fc = dynamic_cast<const ni::ast::function_call *>(&node);
  if (fc != nullptr) {
    return this->visit(*fc);
  }
}
void ni::ast::visitor::visit(const ni::ast::constant &node) {
  auto i = dynamic_cast<const ni::ast::constant_integer *>(&node);
  if (i != nullptr) {
    return this->visit(*i);
  }
}
void ni::ast::visitor::visit(const ni::ast::program &node) {
  for (auto &it : node.instructions) {
    this->visit(*it);
  }
}
void ni::ast::visitor::visit(const ni::ast::variable_declaration &node) {}
void ni::ast::visitor::visit(const ni::ast::variable_assignment &node) {
  this->visit(*node.value);
}
void ni::ast::visitor::visit(const ni::ast::variable_lookup &node) {}
void ni::ast::visitor::visit(const ni::ast::argument &node) {}
void ni::ast::visitor::visit(const ni::ast::binary_operation &node) {
  this->visit(*node.left);
  this->visit(*node.right);
}
void ni::ast::visitor::visit(const ni::ast::function_declaration &node) {
  for (auto &it : node.args) {
    this->visit(*it);
  }
  this->visit(*node.body);
}
void ni::ast::visitor::visit(const ni::ast::block &node) {
  for (auto &it : node.statements) {
    this->visit(*it);
  }
}
void ni::ast::visitor::visit(const ni::ast::function_call &node) {
  for (auto &it : node.params) {
    this->visit(*it);
  }
}
void ni::ast::visitor::visit(const ni::ast::function_return &node) {
  if (node.value != nullptr) {
    this->visit(*node.value);
  }
}
void ni::ast::visitor::visit(const ni::ast::constant_integer &node) {}
