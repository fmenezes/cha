#include "ast.hpp"
#include <algorithm>

namespace cha {

// AstType clone implementation
AstTypePtr AstType::clone() const {
  if (is_primitive()) {
    return std::make_unique<AstType>(location_, as_primitive());
  } else if (is_array()) {
    const auto &arr = as_array();
    return std::make_unique<AstType>(
        location_, Array{arr.element_type->clone(), arr.size});
  } else {
    return std::make_unique<AstType>(location_, as_identifier());
  }
}

// Utility function for cloning node lists
AstNodeList clone_node_list(const AstNodeList &list) {
  AstNodeList cloned;
  cloned.reserve(list.size());
  for (const auto &node : list) {
    cloned.push_back(node->clone());
  }
  return cloned;
}

// Clone implementations for all node types
AstNodePtr ConstantIntegerNode::clone() const {
  auto cloned = std::make_unique<ConstantIntegerNode>(location(), value_);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr ConstantUnsignedIntegerNode::clone() const {
  auto cloned =
      std::make_unique<ConstantUnsignedIntegerNode>(location(), value_);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr ConstantFloatNode::clone() const {
  auto cloned = std::make_unique<ConstantFloatNode>(location(), value_);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr ConstantBoolNode::clone() const {
  auto cloned = std::make_unique<ConstantBoolNode>(location(), value_);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr BinaryOpNode::clone() const {
  auto cloned = std::make_unique<BinaryOpNode>(location(), op_, left_->clone(),
                                               right_->clone());
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr VariableDeclarationNode::clone() const {
  auto cloned = std::make_unique<VariableDeclarationNode>(
      location(), identifier_, type_->clone(),
      value_ ? value_->clone() : nullptr);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr VariableAssignmentNode::clone() const {
  auto cloned = std::make_unique<VariableAssignmentNode>(
      location(), identifier_, value_->clone());
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr VariableLookupNode::clone() const {
  auto cloned = std::make_unique<VariableLookupNode>(location(), identifier_);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr ArgumentNode::clone() const {
  auto cloned =
      std::make_unique<ArgumentNode>(location(), identifier_, type_->clone());
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr BlockNode::clone() const {
  auto cloned =
      std::make_unique<BlockNode>(location(), clone_node_list(statements_));
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr FunctionDeclarationNode::clone() const {
  auto cloned = std::make_unique<FunctionDeclarationNode>(
      location(), identifier_, return_type_->clone(),
      clone_node_list(arguments_), clone_node_list(body_));
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr FunctionCallNode::clone() const {
  auto cloned = std::make_unique<FunctionCallNode>(location(), identifier_,
                                                   clone_node_list(arguments_));
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr FunctionReturnNode::clone() const {
  auto cloned = std::make_unique<FunctionReturnNode>(
      location(), value_ ? value_->clone() : nullptr);
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr IfNode::clone() const {
  auto cloned = std::make_unique<IfNode>(location(), condition_->clone(),
                                         clone_node_list(then_block_),
                                         clone_node_list(else_block_));
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

AstNodePtr ConstantDeclarationNode::clone() const {
  auto cloned = std::make_unique<ConstantDeclarationNode>(
      location(), identifier_, value_->clone());
  if (result_type()) {
    cloned->set_result_type(result_type()->clone());
  }
  return std::move(cloned);
}

// Constructor implementations with type setting
ConstantIntegerNode::ConstantIntegerNode(AstLocation loc, std::string value)
    : AstNode(std::move(loc)), value_(std::move(value)) {
  set_result_type(std::make_unique<AstType>(
      location(), AstType::Primitive{PrimitiveType::CONST_INT}));
}

ConstantUnsignedIntegerNode::ConstantUnsignedIntegerNode(AstLocation loc,
                                                         std::string value)
    : AstNode(std::move(loc)), value_(std::move(value)) {
  set_result_type(std::make_unique<AstType>(
      location(), AstType::Primitive{PrimitiveType::CONST_UINT}));
}

ConstantFloatNode::ConstantFloatNode(AstLocation loc, double value)
    : AstNode(std::move(loc)), value_(value) {
  set_result_type(std::make_unique<AstType>(
      location(), AstType::Primitive{PrimitiveType::CONST_FLOAT}));
}

ConstantBoolNode::ConstantBoolNode(AstLocation loc, bool value)
    : AstNode(std::move(loc)), value_(value) {
  set_result_type(std::make_unique<AstType>(
      location(), AstType::Primitive{PrimitiveType::BOOL}));
}

// Accept method implementations for visitor pattern
void ConstantIntegerNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void ConstantIntegerNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void ConstantUnsignedIntegerNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void ConstantUnsignedIntegerNode::accept(AstVisitor &visitor) {
  visitor.visit(*this);
}

void ConstantFloatNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void ConstantFloatNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void ConstantBoolNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void ConstantBoolNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void BinaryOpNode::accept(AstVisitor &visitor) const { visitor.visit(*this); }

void BinaryOpNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void VariableDeclarationNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void VariableDeclarationNode::accept(AstVisitor &visitor) {
  visitor.visit(*this);
}

void VariableAssignmentNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void VariableAssignmentNode::accept(AstVisitor &visitor) {
  visitor.visit(*this);
}

void VariableLookupNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void VariableLookupNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void ArgumentNode::accept(AstVisitor &visitor) const { visitor.visit(*this); }

void ArgumentNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void BlockNode::accept(AstVisitor &visitor) const { visitor.visit(*this); }

void BlockNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void FunctionDeclarationNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void FunctionDeclarationNode::accept(AstVisitor &visitor) {
  visitor.visit(*this);
}

void FunctionCallNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void FunctionCallNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void FunctionReturnNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void FunctionReturnNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void IfNode::accept(AstVisitor &visitor) const { visitor.visit(*this); }

void IfNode::accept(AstVisitor &visitor) { visitor.visit(*this); }

void ConstantDeclarationNode::accept(AstVisitor &visitor) const {
  visitor.visit(*this);
}

void ConstantDeclarationNode::accept(AstVisitor &visitor) {
  visitor.visit(*this);
}

} // namespace cha
