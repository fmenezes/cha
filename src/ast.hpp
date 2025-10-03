#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace cha {

// Location information
struct AstLocation {
  std::string file;
  int line_begin;
  int column_begin;
  int line_end;
  int column_end;

  AstLocation(const std::string &f, int lb, int cb, int le, int ce)
      : file(f), line_begin(lb), column_begin(cb), line_end(le),
        column_end(ce) {}

  AstLocation() : line_begin(0), column_begin(0), line_end(0), column_end(0) {}
};

// Forward declarations
class AstNode;
class AstType;
class AstVisitor;
using AstNodePtr = std::unique_ptr<AstNode>;
using AstTypePtr = std::unique_ptr<AstType>;
using AstNodeList = std::vector<AstNodePtr>;

// Enums
enum class PrimitiveType {
  UNDEF = -1,
  CONST_INT,
  INT,
  INT8,
  INT16,
  INT32,
  INT64,
  CONST_UINT,
  UINT,
  UINT8,
  UINT16,
  UINT32,
  UINT64,
  CONST_FLOAT,
  FLOAT16,
  FLOAT32,
  FLOAT64,
  BOOL
};

// Stream operator for PrimitiveType (for testing)
inline std::ostream &operator<<(std::ostream &os, PrimitiveType type) {
  switch (type) {
  case PrimitiveType::UNDEF:
    return os << "UNDEF";
  case PrimitiveType::CONST_INT:
    return os << "CONST_INT";
  case PrimitiveType::INT:
    return os << "INT";
  case PrimitiveType::INT8:
    return os << "INT8";
  case PrimitiveType::INT16:
    return os << "INT16";
  case PrimitiveType::INT32:
    return os << "INT32";
  case PrimitiveType::INT64:
    return os << "INT64";
  case PrimitiveType::CONST_UINT:
    return os << "CONST_UINT";
  case PrimitiveType::UINT:
    return os << "UINT";
  case PrimitiveType::UINT8:
    return os << "UINT8";
  case PrimitiveType::UINT16:
    return os << "UINT16";
  case PrimitiveType::UINT32:
    return os << "UINT32";
  case PrimitiveType::UINT64:
    return os << "UINT64";
  case PrimitiveType::CONST_FLOAT:
    return os << "CONST_FLOAT";
  case PrimitiveType::FLOAT16:
    return os << "FLOAT16";
  case PrimitiveType::FLOAT32:
    return os << "FLOAT32";
  case PrimitiveType::FLOAT64:
    return os << "FLOAT64";
  case PrimitiveType::BOOL:
    return os << "BOOL";
  default:
    return os << "UNKNOWN";
  }
}

enum class BinaryOperator {
  PLUS,
  MINUS,
  STAR,
  SLASH,
  EQUALS_EQUALS,
  NOT_EQUALS,
  GREATER_THAN,
  GREATER_THAN_OR_EQUALS,
  LESS_THAN,
  LESS_THAN_OR_EQUALS,
  AND,
  OR
};

// Type system
class AstType {
public:
  struct Primitive {
    PrimitiveType type;
    explicit Primitive(PrimitiveType t) : type(t) {}
  };

  struct Array {
    AstTypePtr element_type;
    int size;
    Array(AstTypePtr elem, int s) : element_type(std::move(elem)), size(s) {}
  };

  struct Identifier {
    std::string name;
    explicit Identifier(std::string n) : name(std::move(n)) {}
  };

private:
  AstLocation location_;
  std::variant<Primitive, Array, Identifier> type_data_;

public:
  AstType(AstLocation loc, Primitive p)
      : location_(std::move(loc)), type_data_(std::move(p)) {}
  AstType(AstLocation loc, Array a)
      : location_(std::move(loc)), type_data_(std::move(a)) {}
  AstType(AstLocation loc, Identifier i)
      : location_(std::move(loc)), type_data_(std::move(i)) {}

  const AstLocation &location() const { return location_; }

  bool is_primitive() const {
    return std::holds_alternative<Primitive>(type_data_);
  }
  bool is_array() const { return std::holds_alternative<Array>(type_data_); }
  bool is_identifier() const {
    return std::holds_alternative<Identifier>(type_data_);
  }

  const Primitive &as_primitive() const {
    return std::get<Primitive>(type_data_);
  }
  const Array &as_array() const { return std::get<Array>(type_data_); }
  const Identifier &as_identifier() const {
    return std::get<Identifier>(type_data_);
  }

  // Clone method for copying types
  AstTypePtr clone() const;
};

// Forward declarations for visitor pattern
class ConstantIntegerNode;
class ConstantUnsignedIntegerNode;
class ConstantFloatNode;
class ConstantBoolNode;
class BinaryOpNode;
class VariableDeclarationNode;
class VariableAssignmentNode;
class VariableLookupNode;
class ArgumentNode;
class BlockNode;
class FunctionDeclarationNode;
class FunctionCallNode;
class FunctionReturnNode;
class IfNode;
class ConstantDeclarationNode;

// Generic visitor interface
class AstVisitor {
public:
  virtual ~AstVisitor() = default;

  // Const visitor methods (for analysis/codegen that doesn't modify AST)
  virtual void visit(const ConstantIntegerNode &node) = 0;
  virtual void visit(const ConstantUnsignedIntegerNode &node) = 0;
  virtual void visit(const ConstantFloatNode &node) = 0;
  virtual void visit(const ConstantBoolNode &node) = 0;
  virtual void visit(const BinaryOpNode &node) = 0;
  virtual void visit(const VariableDeclarationNode &node) = 0;
  virtual void visit(const VariableAssignmentNode &node) = 0;
  virtual void visit(const VariableLookupNode &node) = 0;
  virtual void visit(const ArgumentNode &node) = 0;
  virtual void visit(const BlockNode &node) = 0;
  virtual void visit(const FunctionDeclarationNode &node) = 0;
  virtual void visit(const FunctionCallNode &node) = 0;
  virtual void visit(const FunctionReturnNode &node) = 0;
  virtual void visit(const IfNode &node) = 0;
  virtual void visit(const ConstantDeclarationNode &node) = 0;

  // Non-const visitor methods (for validation that modifies AST with types)
  virtual void visit(ConstantIntegerNode &node) {
    visit(const_cast<const ConstantIntegerNode &>(node));
  }
  virtual void visit(ConstantUnsignedIntegerNode &node) {
    visit(const_cast<const ConstantUnsignedIntegerNode &>(node));
  }
  virtual void visit(ConstantFloatNode &node) {
    visit(const_cast<const ConstantFloatNode &>(node));
  }
  virtual void visit(ConstantBoolNode &node) {
    visit(const_cast<const ConstantBoolNode &>(node));
  }
  virtual void visit(BinaryOpNode &node) {
    visit(const_cast<const BinaryOpNode &>(node));
  }
  virtual void visit(VariableDeclarationNode &node) {
    visit(const_cast<const VariableDeclarationNode &>(node));
  }
  virtual void visit(VariableAssignmentNode &node) {
    visit(const_cast<const VariableAssignmentNode &>(node));
  }
  virtual void visit(VariableLookupNode &node) {
    visit(const_cast<const VariableLookupNode &>(node));
  }
  virtual void visit(ArgumentNode &node) {
    visit(const_cast<const ArgumentNode &>(node));
  }
  virtual void visit(BlockNode &node) {
    visit(const_cast<const BlockNode &>(node));
  }
  virtual void visit(FunctionDeclarationNode &node) {
    visit(const_cast<const FunctionDeclarationNode &>(node));
  }
  virtual void visit(FunctionCallNode &node) {
    visit(const_cast<const FunctionCallNode &>(node));
  }
  virtual void visit(FunctionReturnNode &node) {
    visit(const_cast<const FunctionReturnNode &>(node));
  }
  virtual void visit(IfNode &node) { visit(const_cast<const IfNode &>(node)); }
  virtual void visit(ConstantDeclarationNode &node) {
    visit(const_cast<const ConstantDeclarationNode &>(node));
  }
};

// AST Node base class
class AstNode {
public:
  virtual ~AstNode() = default;

  const AstLocation &location() const { return location_; }
  void set_result_type(AstTypePtr type) { result_type_ = std::move(type); }
  const AstType *result_type() const { return result_type_.get(); }

  // Pure virtual clone method for copying nodes
  virtual AstNodePtr clone() const = 0;

  // Visitor pattern accept method
  virtual void accept(AstVisitor &visitor) const = 0;
  virtual void accept(AstVisitor &visitor) = 0; // Non-const version for
                                                // visitors that modify nodes

protected:
  explicit AstNode(AstLocation loc) : location_(std::move(loc)) {}

private:
  AstLocation location_;
  AstTypePtr result_type_;
};

// Concrete node types
class ConstantIntegerNode : public AstNode {
public:
  ConstantIntegerNode(AstLocation loc, std::string value);

  const std::string &value() const { return value_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string value_;
};

class ConstantUnsignedIntegerNode : public AstNode {
public:
  ConstantUnsignedIntegerNode(AstLocation loc, std::string value);

  const std::string &value() const { return value_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string value_;
};

class ConstantFloatNode : public AstNode {
public:
  ConstantFloatNode(AstLocation loc, double value);

  double value() const { return value_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  double value_;
};

class ConstantBoolNode : public AstNode {
public:
  ConstantBoolNode(AstLocation loc, bool value);

  bool value() const { return value_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  bool value_;
};

class BinaryOpNode : public AstNode {
public:
  BinaryOpNode(AstLocation loc, BinaryOperator op, AstNodePtr left, AstNodePtr right)
      : AstNode(std::move(loc)), op_(op), left_(std::move(left)),
        right_(std::move(right)) {}

  BinaryOperator op() const { return op_; }
  const AstNode &left() const { return *left_; }
  const AstNode &right() const { return *right_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  BinaryOperator op_;
  AstNodePtr left_;
  AstNodePtr right_;
};

class VariableDeclarationNode : public AstNode {
public:
  VariableDeclarationNode(AstLocation loc, std::string identifier,
                          AstTypePtr type, AstNodePtr value = nullptr)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)),
        type_(std::move(type)), value_(std::move(value)) {}

  const std::string &identifier() const { return identifier_; }
  const AstType &type() const { return *type_; }
  const AstNode *value() const { return value_.get(); }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
  AstTypePtr type_;
  AstNodePtr value_;
};

class VariableAssignmentNode : public AstNode {
public:
  VariableAssignmentNode(AstLocation loc, std::string identifier,
                         AstNodePtr value)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)),
        value_(std::move(value)) {}

  const std::string &identifier() const { return identifier_; }
  const AstNode &value() const { return *value_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
  AstNodePtr value_;
};

class VariableLookupNode : public AstNode {
public:
  VariableLookupNode(AstLocation loc, std::string identifier)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)) {}

  const std::string &identifier() const { return identifier_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
};

class ArgumentNode : public AstNode {
public:
  ArgumentNode(AstLocation loc, std::string identifier, AstTypePtr type)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)),
        type_(std::move(type)) {}

  const std::string &identifier() const { return identifier_; }
  const AstType &type() const { return *type_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
  AstTypePtr type_;
};

class BlockNode : public AstNode {
public:
  BlockNode(AstLocation loc, AstNodeList statements)
      : AstNode(std::move(loc)), statements_(std::move(statements)) {}

  const AstNodeList &statements() const { return statements_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  AstNodeList statements_;
};

class FunctionDeclarationNode : public AstNode {
public:
  FunctionDeclarationNode(AstLocation loc, std::string identifier,
                          AstTypePtr return_type, AstNodeList arguments,
                          AstNodeList body)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)),
        return_type_(std::move(return_type)), arguments_(std::move(arguments)),
        body_(std::move(body)) {}

  const std::string &identifier() const { return identifier_; }
  const AstType &return_type() const { return *return_type_; }
  const AstNodeList &arguments() const { return arguments_; }
  const AstNodeList &body() const { return body_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
  AstTypePtr return_type_;
  AstNodeList arguments_;
  AstNodeList body_;
};

class FunctionCallNode : public AstNode {
public:
  FunctionCallNode(AstLocation loc, std::string identifier,
                   AstNodeList arguments)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)),
        arguments_(std::move(arguments)) {}

  const std::string &identifier() const { return identifier_; }
  const AstNodeList &arguments() const { return arguments_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
  AstNodeList arguments_;
};

class FunctionReturnNode : public AstNode {
public:
  FunctionReturnNode(AstLocation loc, AstNodePtr value = nullptr)
      : AstNode(std::move(loc)), value_(std::move(value)) {}

  const AstNode *value() const { return value_.get(); }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  AstNodePtr value_;
};

class IfNode : public AstNode {
public:
  IfNode(AstLocation loc, AstNodePtr condition, AstNodeList then_block,
         AstNodeList else_block = {})
      : AstNode(std::move(loc)), condition_(std::move(condition)),
        then_block_(std::move(then_block)), else_block_(std::move(else_block)) {
  }

  const AstNode &condition() const { return *condition_; }
  const AstNodeList &then_block() const { return then_block_; }
  const AstNodeList &else_block() const { return else_block_; }
  bool has_else() const { return !else_block_.empty(); }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  AstNodePtr condition_;
  AstNodeList then_block_;
  AstNodeList else_block_;
};

class ConstantDeclarationNode : public AstNode {
public:
  ConstantDeclarationNode(AstLocation loc, std::string identifier,
                          AstNodePtr value)
      : AstNode(std::move(loc)), identifier_(std::move(identifier)),
        value_(std::move(value)) {}

  const std::string &identifier() const { return identifier_; }
  const AstNode &value() const { return *value_; }
  AstNodePtr clone() const override;
  void accept(AstVisitor &visitor) const override;
  void accept(AstVisitor &visitor) override;

private:
  std::string identifier_;
  AstNodePtr value_;
};

// Utility functions for cloning lists
AstNodeList clone_node_list(const AstNodeList &list);

} // namespace cha
