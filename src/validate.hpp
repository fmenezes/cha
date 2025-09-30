#pragma once

#include "ast.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cha {

// Forward declarations
class SymbolTable;
class Validator;

// Exception class for validation errors
class ValidationError : public std::exception {
public:
  ValidationError(const AstLocation &location, const std::string &message)
      : location_(location), message_(message) {}

  const AstLocation &location() const { return location_; }
  const std::string &message() const { return message_; }
  const char *what() const noexcept override { return message_.c_str(); }

private:
  AstLocation location_;
  std::string message_;
};

// Symbol table entry
struct SymbolEntry {
  AstNodePtr node;

  explicit SymbolEntry(AstNodePtr n) : node(std::move(n)) {}
};

// Symbol table for managing scopes
class SymbolTable : public std::enable_shared_from_this<SymbolTable> {
public:
  explicit SymbolTable(std::shared_ptr<SymbolTable> parent = nullptr)
      : parent_(std::move(parent)) {}

  // Insert a symbol, returns false if already exists
  bool insert(const std::string &name, AstNodePtr node);

  // Lookup a symbol in this table and parent tables
  const SymbolEntry *lookup(const std::string &name) const;

  // Create a new child scope
  std::shared_ptr<SymbolTable> create_child_scope();

  std::shared_ptr<SymbolTable> parent_;

private:
  std::unordered_map<std::string, std::unique_ptr<SymbolEntry>> symbols_;
};

// Type utilities
class TypeUtils {
public:
  // Check if a type is numeric
  static bool is_numeric(PrimitiveType type);

  // Check if a type is signed integer
  static bool is_signed_int(PrimitiveType type);

  // Check if a type is unsigned integer
  static bool is_unsigned_int(PrimitiveType type);

  // Check if a type is floating point
  static bool is_float(PrimitiveType type);

  // Convert arithmetic types for binary operations
  static PrimitiveType convert_arithmetic_types(PrimitiveType left,
                                                PrimitiveType right);

  // Check assignment compatibility
  static bool is_assignment_compatible(PrimitiveType from, PrimitiveType to);

  // Check numeric comparison compatibility
  static bool is_numeric_comparison_compatible(PrimitiveType left,
                                               PrimitiveType right);

  // Check equality comparison compatibility
  static bool is_equality_comparison_compatible(PrimitiveType left,
                                                PrimitiveType right);

  // Convert type to string for error messages
  static std::string type_to_string(const AstType *type);
  static std::string type_to_string(PrimitiveType type);

  // Set type on constant nodes for type inference
  static void set_type_on_const(AstNode &node, PrimitiveType type);
};

// Main validator class
class Validator {
public:
  Validator();

  // Main validation entry point
  bool validate(const AstNodeList &ast);

  // Get collected errors
  const std::vector<ValidationError> &errors() const { return errors_; }

private:
  // Validation methods for different node types
  bool validate_top_level(const AstNodeList &nodes);
  bool validate_node_list(const AstNodeList &nodes);
  bool validate_node(const AstNode &node);

  bool validate_function_declaration(const FunctionDeclarationNode &node);
  bool validate_constant_declaration(const ConstantDeclarationNode &node);
  bool validate_variable_declaration(const VariableDeclarationNode &node);
  bool validate_argument(const ArgumentNode &node);
  bool validate_variable_assignment(const VariableAssignmentNode &node);
  bool validate_variable_lookup(
      VariableLookupNode &node);               // non-const for type setting
  bool validate_binary_op(BinaryOpNode &node); // non-const for type setting
  bool
  validate_function_call(FunctionCallNode &node); // non-const for type setting
  bool validate_function_return(const FunctionReturnNode &node);
  bool validate_if(const IfNode &node);
  bool validate_block(const BlockNode &node);

  // Helper methods
  bool check_type_assignment(AstNode &value_node, const AstType &expected_type);
  void create_stack_frame();
  void release_stack_frame();
  void add_error(const AstLocation &location, const std::string &message);

  // State
  std::shared_ptr<SymbolTable> symbol_table_;
  std::vector<ValidationError> errors_;
  const FunctionDeclarationNode *current_function_;
};

} // namespace cha
