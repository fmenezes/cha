#include "validate.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <sstream>

namespace cha {

// SymbolTable implementation
bool SymbolTable::insert(const std::string &name, AstNodePtr node) {
  if (symbols_.find(name) != symbols_.end()) {
    return false; // Already exists
  }
  symbols_[name] = std::make_unique<SymbolEntry>(std::move(node));
  return true;
}

const SymbolEntry *SymbolTable::lookup(const std::string &name) const {
  auto it = symbols_.find(name);
  if (it != symbols_.end()) {
    return it->second.get();
  }

  if (parent_) {
    return parent_->lookup(name);
  }

  return nullptr;
}

std::shared_ptr<SymbolTable> SymbolTable::create_child_scope() {
  return std::make_shared<SymbolTable>(shared_from_this());
}

// TypeUtils implementation
bool TypeUtils::is_numeric(PrimitiveType type) {
  return is_signed_int(type) || is_unsigned_int(type) || is_float(type);
}

bool TypeUtils::is_signed_int(PrimitiveType type) {
  return type >= PrimitiveType::CONST_INT && type <= PrimitiveType::INT64;
}

bool TypeUtils::is_unsigned_int(PrimitiveType type) {
  return type >= PrimitiveType::CONST_UINT && type <= PrimitiveType::UINT64;
}

bool TypeUtils::is_float(PrimitiveType type) {
  return type == PrimitiveType::FLOAT16 || type == PrimitiveType::FLOAT32 ||
         type == PrimitiveType::FLOAT64 || type == PrimitiveType::CONST_FLOAT;
}

PrimitiveType TypeUtils::convert_arithmetic_types(PrimitiveType left,
                                                  PrimitiveType right) {
  if (is_float(left) && is_float(right)) {
    return (left > right) ? left : right;
  }

  if (is_signed_int(left) && is_signed_int(right)) {
    return (left > right) ? left : right;
  }

  if (is_unsigned_int(left) && is_unsigned_int(right)) {
    return (left > right) ? left : right;
  }

  return PrimitiveType::UNDEF; // Incompatible types
}

bool TypeUtils::is_assignment_compatible(PrimitiveType from, PrimitiveType to) {
  if (from == PrimitiveType::UNDEF || to == PrimitiveType::UNDEF) {
    return false;
  }

  // Same type assignments (except for non-assignable types)
  if (from == to) {
    return !(from == PrimitiveType::CONST_INT ||
             from == PrimitiveType::CONST_UINT ||
             from == PrimitiveType::CONST_FLOAT);
  }

  // Constants can be assigned to their corresponding types
  if (from == PrimitiveType::CONST_INT && is_signed_int(to))
    return true;
  if (from == PrimitiveType::CONST_UINT && is_unsigned_int(to))
    return true;
  if (from == PrimitiveType::CONST_FLOAT && is_float(to))
    return true;

  // Bool assignments are very restrictive
  if (to == PrimitiveType::BOOL) {
    return (from == PrimitiveType::BOOL);
  }

  // Check for compatible type ranges
  if (is_numeric(from) && is_numeric(to)) {
    // Allow if they're the same category (signed->signed, unsigned->unsigned)
    if (is_signed_int(from) && is_signed_int(to))
      return true;
    if (is_unsigned_int(from) && is_unsigned_int(to))
      return true;
    if (is_float(from) && is_float(to))
      return true;

    // Otherwise, mixed sign/type assignments are not allowed
    return false;
  }

  return false; // Incompatible types
}

bool TypeUtils::is_numeric_comparison_compatible(PrimitiveType left,
                                                 PrimitiveType right) {
  if (left == PrimitiveType::UNDEF || right == PrimitiveType::UNDEF) {
    return false;
  }

  // Same type comparisons
  if (left == right) {
    return !(left == PrimitiveType::CONST_INT ||
             left == PrimitiveType::CONST_UINT ||
             left == PrimitiveType::CONST_FLOAT);
  }

  // Numeric types can be compared
  if ((is_signed_int(left) || is_unsigned_int(left) || is_float(left)) &&
      (is_signed_int(right) || is_unsigned_int(right) || is_float(right))) {
    return true;
  }

  // Bool can be compared with bool
  if (left == PrimitiveType::BOOL && right == PrimitiveType::BOOL) {
    return true;
  }

  return false; // Incompatible
}

bool TypeUtils::is_equality_comparison_compatible(PrimitiveType left,
                                                  PrimitiveType right) {
  return is_numeric_comparison_compatible(left, right);
}

std::string TypeUtils::type_to_string(const AstType *type) {
  if (!type) {
    return "void";
  }

  if (type->is_primitive()) {
    return type_to_string(type->as_primitive().type);
  } else if (type->is_array()) {
    return type_to_string(type->as_array().element_type.get()) + "[]";
  } else if (type->is_identifier()) {
    return type->as_identifier().name;
  }

  return "unknown";
}

std::string TypeUtils::type_to_string(PrimitiveType type) {
  switch (type) {
  case PrimitiveType::UNDEF:
    return "undefined";
  case PrimitiveType::CONST_INT:
    return "c_int";
  case PrimitiveType::CONST_UINT:
    return "c_uint";
  case PrimitiveType::INT8:
    return "int8";
  case PrimitiveType::UINT8:
    return "uint8";
  case PrimitiveType::INT16:
    return "int16";
  case PrimitiveType::UINT16:
    return "uint16";
  case PrimitiveType::INT32:
    return "int32";
  case PrimitiveType::UINT32:
    return "uint32";
  case PrimitiveType::INT64:
    return "int64";
  case PrimitiveType::UINT64:
    return "uint64";
  case PrimitiveType::CONST_FLOAT:
    return "float";
  case PrimitiveType::FLOAT16:
    return "float16";
  case PrimitiveType::FLOAT32:
    return "float32";
  case PrimitiveType::FLOAT64:
    return "float64";
  case PrimitiveType::INT:
    return "int";
  case PrimitiveType::UINT:
    return "uint";
  case PrimitiveType::BOOL:
    return "bool";
  }
  return "unknown";
}

void TypeUtils::set_type_on_const(AstNode &node, PrimitiveType type) {
  if (node.result_type() && node.result_type()->is_primitive()) {
    PrimitiveType current = node.result_type()->as_primitive().type;
    if (current == PrimitiveType::CONST_INT ||
        current == PrimitiveType::CONST_UINT ||
        current == PrimitiveType::CONST_FLOAT) {
      node.set_result_type(
          std::make_unique<AstType>(node.location(), AstType::Primitive(type)));
    }
  }
}

// Validator implementation
Validator::Validator()
    : symbol_table_(std::make_shared<SymbolTable>()),
      current_function_(nullptr) {}

void Validator::validate(const AstNodeList &ast) {
  errors_.clear();
  symbol_table_ = std::make_shared<SymbolTable>();
  current_function_ = nullptr;

  try {
    validate_top_level(ast);

    // If we collected any errors, throw a MultipleValidationException
    if (!errors_.empty()) {
      throw MultipleValidationException(errors_);
    }
  } catch (const ValidationException &e) {
    // Re-throw single validation exceptions
    throw;
  }
}

void Validator::validate_top_level(const AstNodeList &nodes) {
  // First pass: register all function declarations
  for (const auto &node : nodes) {
    if (auto func_decl =
            dynamic_cast<const FunctionDeclarationNode *>(node.get())) {
      if (!symbol_table_->insert(func_decl->identifier(), node->clone())) {
        add_error(node->location(),
                  "'" + func_decl->identifier() + "' already defined");
      }
    }
  }

  // Second pass: validate all nodes
  for (const auto &node : nodes) {
    validate_node(*node);
  }
}

void Validator::validate_node_list(const AstNodeList &nodes) {
  for (const auto &node : nodes) {
    validate_node(*node);
  }
}

void Validator::validate_node(const AstNode &node) {
  if (auto func_decl = dynamic_cast<const FunctionDeclarationNode *>(&node)) {
    validate_function_declaration(*func_decl);
  } else if (auto const_decl =
                 dynamic_cast<const ConstantDeclarationNode *>(&node)) {
    validate_constant_declaration(*const_decl);
  } else if (auto var_decl =
                 dynamic_cast<const VariableDeclarationNode *>(&node)) {
    validate_variable_declaration(*var_decl);
  } else if (auto arg = dynamic_cast<const ArgumentNode *>(&node)) {
    validate_argument(*arg);
  } else if (auto var_assign =
                 dynamic_cast<const VariableAssignmentNode *>(&node)) {
    validate_variable_assignment(*var_assign);
  } else if (auto var_lookup = dynamic_cast<VariableLookupNode *>(
                 const_cast<AstNode *>(&node))) {
    validate_variable_lookup(*var_lookup);
  } else if (auto bin_op =
                 dynamic_cast<BinaryOpNode *>(const_cast<AstNode *>(&node))) {
    validate_binary_op(*bin_op);
  } else if (auto func_call = dynamic_cast<FunctionCallNode *>(
                 const_cast<AstNode *>(&node))) {
    validate_function_call(*func_call);
  } else if (auto func_ret = dynamic_cast<const FunctionReturnNode *>(&node)) {
    validate_function_return(*func_ret);
  } else if (auto if_node = dynamic_cast<const IfNode *>(&node)) {
    validate_if(*if_node);
  } else if (auto block = dynamic_cast<const BlockNode *>(&node)) {
    validate_block(*block);
  }

  // For other node types (constants, etc.), no validation needed
}

void Validator::validate_function_declaration(
    const FunctionDeclarationNode &node) {
  create_stack_frame();
  current_function_ = &node;

  // Validate arguments
  validate_node_list(node.arguments());

  // Validate body
  validate_node_list(node.body());

  current_function_ = nullptr;
  release_stack_frame();
}

void Validator::validate_constant_declaration(
    const ConstantDeclarationNode &node) {
  if (!symbol_table_->insert(node.identifier(), node.clone())) {
    add_error(node.location(),
              "constant '" + node.identifier() + "' already defined");
  }
}

void Validator::validate_variable_declaration(
    const VariableDeclarationNode &node) {
  if (node.value()) {
    validate_node(*node.value());
  }

  if (!symbol_table_->insert(node.identifier(), node.clone())) {
    add_error(node.location(),
              "variable '" + node.identifier() + "' already defined");
  }
}

void Validator::validate_argument(const ArgumentNode &node) {
  if (!symbol_table_->insert(node.identifier(), node.clone())) {
    add_error(node.location(),
              "argument '" + node.identifier() + "' already defined");
  }
}

void Validator::validate_variable_assignment(
    const VariableAssignmentNode &node) {
  const SymbolEntry *entry = symbol_table_->lookup(node.identifier());
  if (!entry) {
    add_error(node.location(),
              "variable '" + node.identifier() + "' not found");
    return;
  }

  validate_node(node.value());

  if (auto var_decl =
          dynamic_cast<const VariableDeclarationNode *>(entry->node.get())) {
    if (!check_type_assignment(const_cast<AstNode &>(node.value()),
                               var_decl->type())) {
      add_error(
          node.location(),
          "type mismatch expects '" +
              TypeUtils::type_to_string(&var_decl->type()) + "' passed '" +
              TypeUtils::type_to_string(node.value().result_type()) + "'");
    }
  }
}

void Validator::validate_variable_lookup(VariableLookupNode &node) {
  const SymbolEntry *entry = symbol_table_->lookup(node.identifier());
  if (!entry) {
    add_error(node.location(), "'" + node.identifier() + "' not found");
    return;
  }

  if (auto const_decl =
          dynamic_cast<const ConstantDeclarationNode *>(entry->node.get())) {
    // Replace lookup with constant value (this mirrors the C code behavior)
    if (const_decl->value().result_type()) {
      node.set_result_type(const_decl->value().result_type()->clone());
    }
  } else if (auto var_decl = dynamic_cast<const VariableDeclarationNode *>(
                 entry->node.get())) {
    // Copy type from variable declaration
    node.set_result_type(var_decl->type().clone());
  } else if (auto arg_node =
                 dynamic_cast<const ArgumentNode *>(entry->node.get())) {
    // Copy type from function argument
    node.set_result_type(arg_node->type().clone());
  } else {
    add_error(node.location(), "incompatible element found");
  }
}

void Validator::validate_binary_op(BinaryOpNode &node) {
  validate_node(node.left());
  validate_node(node.right());

  const AstType *left_type = node.left().result_type();
  const AstType *right_type = node.right().result_type();

  if (!left_type || !right_type || !left_type->is_primitive() ||
      !right_type->is_primitive()) {
    add_error(node.location(), "invalid operand types");
    return;
  }

  PrimitiveType left_prim = left_type->as_primitive().type;
  PrimitiveType right_prim = right_type->as_primitive().type;

  switch (node.op()) {
  case Operator::ADD:
  case Operator::SUBTRACT:
  case Operator::MULTIPLY:
  case Operator::DIVIDE: {
    PrimitiveType result_type =
        TypeUtils::convert_arithmetic_types(left_prim, right_prim);
    if (result_type == PrimitiveType::UNDEF) {
      add_error(node.location(),
                "incompatible types found for operation: '" +
                    TypeUtils::type_to_string(left_prim) + "', '" +
                    TypeUtils::type_to_string(right_prim) + "'");
      return;
    }
    node.set_result_type(std::make_unique<AstType>(
        node.location(), AstType::Primitive(result_type)));
    TypeUtils::set_type_on_const(const_cast<AstNode &>(node.left()),
                                 result_type);
    TypeUtils::set_type_on_const(const_cast<AstNode &>(node.right()),
                                 result_type);
    break;
  }
  case Operator::GREATER_THAN:
  case Operator::GREATER_THAN_OR_EQUALS:
  case Operator::LESS_THAN:
  case Operator::LESS_THAN_OR_EQUALS: {
    if (!TypeUtils::is_numeric_comparison_compatible(left_prim, right_prim)) {
      add_error(node.location(),
                "incompatible types found for operation: '" +
                    TypeUtils::type_to_string(left_prim) + "', '" +
                    TypeUtils::type_to_string(right_prim) + "'");
      return;
    }
    node.set_result_type(std::make_unique<AstType>(
        node.location(), AstType::Primitive(PrimitiveType::BOOL)));
    TypeUtils::set_type_on_const(const_cast<AstNode &>(node.left()),
                                 right_prim);
    TypeUtils::set_type_on_const(const_cast<AstNode &>(node.right()),
                                 left_prim);
    break;
  }
  case Operator::EQUALS_EQUALS:
  case Operator::NOT_EQUALS: {
    if (!TypeUtils::is_equality_comparison_compatible(left_prim, right_prim)) {
      add_error(node.location(),
                "incompatible types found for operation: '" +
                    TypeUtils::type_to_string(left_prim) + "', '" +
                    TypeUtils::type_to_string(right_prim) + "'");
      return;
    }
    node.set_result_type(std::make_unique<AstType>(
        node.location(), AstType::Primitive(PrimitiveType::BOOL)));
    TypeUtils::set_type_on_const(const_cast<AstNode &>(node.left()),
                                 right_prim);
    TypeUtils::set_type_on_const(const_cast<AstNode &>(node.right()),
                                 left_prim);
    break;
  }
  case Operator::AND:
  case Operator::OR: {
    if (left_prim != PrimitiveType::BOOL || right_prim != PrimitiveType::BOOL) {
      add_error(node.location(),
                "incompatible types found for operation: '" +
                    TypeUtils::type_to_string(left_prim) + "', '" +
                    TypeUtils::type_to_string(right_prim) + "'");
      return;
    }
    node.set_result_type(std::make_unique<AstType>(
        node.location(), AstType::Primitive(PrimitiveType::BOOL)));
    break;
  }
  }
}

void Validator::validate_function_call(FunctionCallNode &node) {
  const SymbolEntry *entry = symbol_table_->lookup(node.identifier());
  if (!entry) {
    add_error(node.location(),
              "function '" + node.identifier() + "' not found");
    return;
  }

  auto func_decl =
      dynamic_cast<const FunctionDeclarationNode *>(entry->node.get());
  if (!func_decl) {
    add_error(node.location(), "'" + node.identifier() + "' is not a function");
    return;
  }

  // Set return type
  node.set_result_type(func_decl->return_type().clone());

  // Check argument count
  if (node.arguments().size() != func_decl->arguments().size()) {
    std::stringstream ss;
    ss << "function '" << node.identifier() << "' expects ";
    if (func_decl->arguments().size() == 0) {
      ss << "no arguments";
    } else {
      ss << "arguments";
    }
    add_error(node.location(), ss.str());
    return;
  }

  // Validate arguments
  for (size_t i = 0; i < node.arguments().size(); ++i) {
    validate_node(*node.arguments()[i]);

    auto arg_decl =
        dynamic_cast<const ArgumentNode *>(func_decl->arguments()[i].get());
    if (arg_decl &&
        !check_type_assignment(*node.arguments()[i], arg_decl->type())) {
      add_error(
          node.arguments()[i]->location(),
          "type mismatch expects '" +
              TypeUtils::type_to_string(&arg_decl->type()) + "' passed '" +
              TypeUtils::type_to_string(node.arguments()[i]->result_type()) +
              "'");
    }
  }
}

void Validator::validate_function_return(const FunctionReturnNode &node) {
  if (node.value()) {
    validate_node(*node.value());
  }

  if (!current_function_) {
    add_error(node.location(), "return statement outside function");
    return;
  }

  // Check return type compatibility
  if (!node.value() && current_function_->return_type().is_primitive() &&
      current_function_->return_type().as_primitive().type !=
          PrimitiveType::UNDEF) {
    add_error(node.location(), "return value expected");
    return;
  }

  if (node.value() && current_function_->return_type().is_primitive() &&
      current_function_->return_type().as_primitive().type ==
          PrimitiveType::UNDEF) {
    add_error(node.location(), "return value not expected");
    return;
  }

  if (node.value() &&
      !check_type_assignment(const_cast<AstNode &>(*node.value()),
                             current_function_->return_type())) {
    add_error(node.location(),
              "return type mismatch expects '" +
                  TypeUtils::type_to_string(&current_function_->return_type()) +
                  "' passed '" +
                  TypeUtils::type_to_string(node.value()->result_type()) + "'");
  }
}

void Validator::validate_if(const IfNode &node) {
  validate_node(node.condition());

  if (node.condition().result_type() &&
      node.condition().result_type()->is_primitive()) {
    if (node.condition().result_type()->as_primitive().type !=
        PrimitiveType::BOOL) {
      add_error(node.condition().location(), "condition should return bool");
    }
  }

  create_stack_frame();
  validate_node_list(node.then_block());
  release_stack_frame();

  if (node.has_else()) {
    create_stack_frame();
    validate_node_list(node.else_block());
    release_stack_frame();
  }
}

void Validator::validate_block(const BlockNode &node) {
  validate_node_list(node.statements());
}

bool Validator::check_type_assignment(AstNode &value_node,
                                      const AstType &expected_type) {
  if (!value_node.result_type() || !expected_type.is_primitive()) {
    return false;
  }

  if (!value_node.result_type()->is_primitive()) {
    return false;
  }

  PrimitiveType from_type = value_node.result_type()->as_primitive().type;
  PrimitiveType to_type = expected_type.as_primitive().type;

  if (!TypeUtils::is_assignment_compatible(from_type, to_type)) {
    return false;
  }

  TypeUtils::set_type_on_const(value_node, to_type);
  return true;
}

void Validator::create_stack_frame() {
  symbol_table_ = symbol_table_->create_child_scope();
}

void Validator::release_stack_frame() {
  if (symbol_table_->parent_) {
    symbol_table_ = symbol_table_->parent_;
  }
}

void Validator::add_error(const AstLocation &location,
                          const std::string &message) {
  errors_.emplace_back(location, message);
}

} // namespace cha
