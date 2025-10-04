#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "../src/ast.hpp"
#include "../src/exceptions.hpp"
#include "../src/validate.hpp"

using namespace cha;

// Helper function to create a location
AstLocation make_test_location() {
  return AstLocation("test.cha", 1, 1, 1, 10);
}

// Helper function to create primitive types
std::unique_ptr<AstType> make_int_type() {
  return std::make_unique<AstType>(make_test_location(),
                                   AstType::Primitive(PrimitiveType::INT));
}

std::unique_ptr<AstType> make_uint_type() {
  return std::make_unique<AstType>(make_test_location(),
                                   AstType::Primitive(PrimitiveType::UINT));
}

std::unique_ptr<AstType> make_bool_type() {
  return std::make_unique<AstType>(make_test_location(),
                                   AstType::Primitive(PrimitiveType::BOOL));
}

std::unique_ptr<AstType> make_float_type() {
  return std::make_unique<AstType>(make_test_location(),
                                   AstType::Primitive(PrimitiveType::FLOAT32));
}

// Test TypeUtils functions
TEST(ValidateTest, TypeUtils) {
  // Test is_numeric
  EXPECT_TRUE(TypeUtils::is_numeric(PrimitiveType::INT));
  EXPECT_TRUE(TypeUtils::is_numeric(PrimitiveType::UINT));
  EXPECT_TRUE(TypeUtils::is_numeric(PrimitiveType::FLOAT32));
  EXPECT_FALSE(TypeUtils::is_numeric(PrimitiveType::BOOL));

  // Test is_signed_int
  EXPECT_TRUE(TypeUtils::is_signed_int(PrimitiveType::INT));
  EXPECT_TRUE(TypeUtils::is_signed_int(PrimitiveType::INT32));
  EXPECT_FALSE(TypeUtils::is_signed_int(PrimitiveType::UINT));
  EXPECT_FALSE(TypeUtils::is_signed_int(PrimitiveType::BOOL));

  // Test is_unsigned_int
  EXPECT_TRUE(TypeUtils::is_unsigned_int(PrimitiveType::UINT));
  EXPECT_TRUE(TypeUtils::is_unsigned_int(PrimitiveType::UINT32));
  EXPECT_FALSE(TypeUtils::is_unsigned_int(PrimitiveType::INT));
  EXPECT_FALSE(TypeUtils::is_unsigned_int(PrimitiveType::BOOL));

  // Test is_float
  EXPECT_TRUE(TypeUtils::is_float(PrimitiveType::FLOAT32));
  EXPECT_TRUE(TypeUtils::is_float(PrimitiveType::FLOAT64));
  EXPECT_FALSE(TypeUtils::is_float(PrimitiveType::INT));
  EXPECT_FALSE(TypeUtils::is_float(PrimitiveType::BOOL));

  // Test convert_arithmetic_types
  EXPECT_EQ(PrimitiveType::INT32,
            TypeUtils::convert_arithmetic_types(PrimitiveType::INT,
                                                PrimitiveType::INT32));
  EXPECT_EQ(PrimitiveType::UINT32,
            TypeUtils::convert_arithmetic_types(PrimitiveType::UINT,
                                                PrimitiveType::UINT32));
  EXPECT_EQ(PrimitiveType::FLOAT64,
            TypeUtils::convert_arithmetic_types(PrimitiveType::FLOAT32,
                                                PrimitiveType::FLOAT64));
  EXPECT_EQ(PrimitiveType::UNDEF, TypeUtils::convert_arithmetic_types(
                                      PrimitiveType::INT, PrimitiveType::UINT));

  // Test assignment compatibility
  EXPECT_TRUE(TypeUtils::is_assignment_compatible(PrimitiveType::INT,
                                                  PrimitiveType::INT));
  EXPECT_TRUE(TypeUtils::is_assignment_compatible(PrimitiveType::CONST_INT,
                                                  PrimitiveType::INT));
  EXPECT_FALSE(TypeUtils::is_assignment_compatible(PrimitiveType::INT,
                                                   PrimitiveType::UINT));
  EXPECT_FALSE(TypeUtils::is_assignment_compatible(PrimitiveType::INT,
                                                   PrimitiveType::BOOL));

  // Test comparison compatibility
  EXPECT_TRUE(TypeUtils::is_numeric_comparison_compatible(
      PrimitiveType::INT, PrimitiveType::INT32));
  EXPECT_TRUE(TypeUtils::is_equality_comparison_compatible(
      PrimitiveType::BOOL, PrimitiveType::BOOL));
  EXPECT_FALSE(TypeUtils::is_numeric_comparison_compatible(
      PrimitiveType::INT, PrimitiveType::BOOL));

  // Test type_to_string
  EXPECT_EQ(std::string("int"), TypeUtils::type_to_string(PrimitiveType::INT));
  EXPECT_EQ(std::string("bool"),
            TypeUtils::type_to_string(PrimitiveType::BOOL));
  EXPECT_EQ(std::string("float32"),
            TypeUtils::type_to_string(PrimitiveType::FLOAT32));
}

// Test SymbolTable functionality
TEST(ValidateTest, SymbolTable) {
  auto table = std::make_shared<SymbolTable>();

  // Test inserting a symbol
  auto var_node = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "test_var", make_int_type());

  EXPECT_TRUE(table->insert("test_var", var_node->clone()));

  // Test duplicate insertion fails
  EXPECT_FALSE(table->insert("test_var", var_node->clone()));

  // Test lookup
  const SymbolEntry *entry = table->lookup("test_var");
  EXPECT_NE(entry, nullptr);

  // Test lookup non-existent symbol
  const SymbolEntry *missing = table->lookup("missing_var");
  EXPECT_EQ(missing, nullptr);

  // Test child scope
  auto child_table = table->create_child_scope();

  // Child should be able to see parent symbols
  const SymbolEntry *parent_entry = child_table->lookup("test_var");
  EXPECT_NE(parent_entry, nullptr);

  // Child can shadow parent symbols
  auto child_var = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "test_var", make_bool_type());
  EXPECT_TRUE(child_table->insert("test_var", child_var->clone()));
}

// Test basic validation scenarios
TEST(ValidateTest, BasicValidation) {
  Validator validator;

  // Test empty AST
  AstNodeList empty_ast;
  EXPECT_NO_THROW(validator.validate(empty_ast));

  // Test simple function declaration
  AstNodeList simple_func;
  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "test_func", make_int_type(), AstNodeList{},
      AstNodeList{});
  simple_func.push_back(std::move(func));

  EXPECT_NO_THROW(validator.validate(simple_func));
}

// Test variable validation
TEST(ValidateTest, VariableValidation) {
  Validator validator;
  AstNodeList ast;

  // Test function with variable declaration
  AstNodeList func_body;

  // Add variable declaration: int x = 42;
  auto const_val =
      std::make_unique<ConstantIntegerNode>(make_test_location(), 42);
  const_val->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  auto var_decl = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "x", make_int_type(), std::move(const_val));
  func_body.push_back(std::move(var_decl));

  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "test_func", make_int_type(), AstNodeList{},
      std::move(func_body));
  ast.push_back(std::move(func));

  EXPECT_NO_THROW(validator.validate(ast));
}

// Test duplicate symbol detection
TEST(ValidateTest, DuplicateSymbols) {
  Validator validator;
  AstNodeList ast;

  // Create two functions with the same name
  auto func1 = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "duplicate_func", make_int_type(), AstNodeList{},
      AstNodeList{});

  auto func2 = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "duplicate_func", make_int_type(), AstNodeList{},
      AstNodeList{});

  ast.push_back(std::move(func1));
  ast.push_back(std::move(func2));

  // Should throw ValidationException or MultipleValidationException
  EXPECT_THROW(validator.validate(ast), ChaException);
}

// Test binary operation validation
TEST(ValidateTest, BinaryOperations) {
  Validator validator;
  AstNodeList ast;
  AstNodeList func_body;

  // Create: int result = 5 + 10;
  auto left = std::make_unique<ConstantIntegerNode>(make_test_location(), 5);
  left->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  auto right = std::make_unique<ConstantIntegerNode>(make_test_location(), 10);
  right->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  auto bin_op =
      std::make_unique<BinaryOpNode>(make_test_location(), BinaryOperator::PLUS,
                                     std::move(left), std::move(right));

  auto var_decl = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "result", make_int_type(), std::move(bin_op));
  func_body.push_back(std::move(var_decl));

  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "test_func", make_int_type(), AstNodeList{},
      std::move(func_body));
  ast.push_back(std::move(func));

  EXPECT_NO_THROW(validator.validate(ast));
}

// Test type mismatch detection
TEST(ValidateTest, TypeMismatches) {
  Validator validator;
  AstNodeList ast;
  AstNodeList func_body;

  // Create: int x; x = true; (should fail)
  auto var_decl = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "x", make_int_type());
  func_body.push_back(std::move(var_decl));

  auto bool_val =
      std::make_unique<ConstantBoolNode>(make_test_location(), true);
  bool_val->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::BOOL)));

  auto assignment = std::make_unique<VariableAssignmentNode>(
      make_test_location(), "x", std::move(bool_val));
  func_body.push_back(std::move(assignment));

  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "test_func", make_int_type(), AstNodeList{},
      std::move(func_body));
  ast.push_back(std::move(func));

  // Should throw an exception for type mismatch
  EXPECT_THROW(validator.validate(ast), ChaException);
}

// Test function call validation
TEST(ValidateTest, FunctionCalls) {
  Validator validator;
  AstNodeList ast;

  // Create function: int add(int a, int b) { return a + b; }
  AstNodeList args;
  auto arg1 = std::make_unique<ArgumentNode>(make_test_location(), "a",
                                             make_int_type());
  auto arg2 = std::make_unique<ArgumentNode>(make_test_location(), "b",
                                             make_int_type());
  args.push_back(std::move(arg1));
  args.push_back(std::move(arg2));

  AstNodeList body;
  auto var_a = std::make_unique<VariableLookupNode>(make_test_location(), "a");
  auto var_b = std::make_unique<VariableLookupNode>(make_test_location(), "b");
  auto add_op =
      std::make_unique<BinaryOpNode>(make_test_location(), BinaryOperator::PLUS,
                                     std::move(var_a), std::move(var_b));
  auto return_stmt = std::make_unique<FunctionReturnNode>(make_test_location(),
                                                          std::move(add_op));
  body.push_back(std::move(return_stmt));

  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "add", make_int_type(), std::move(args),
      std::move(body));
  ast.push_back(std::move(func));

  // Create main function that calls add
  AstNodeList main_body;
  AstNodeList call_args;
  auto const5 = std::make_unique<ConstantIntegerNode>(make_test_location(), 5);
  const5->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
  auto const10 =
      std::make_unique<ConstantIntegerNode>(make_test_location(), 10);
  const10->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  call_args.push_back(std::move(const5));
  call_args.push_back(std::move(const10));

  auto func_call = std::make_unique<FunctionCallNode>(
      make_test_location(), "add", std::move(call_args));
  auto result_var = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "result", make_int_type(), std::move(func_call));
  main_body.push_back(std::move(result_var));

  auto main_func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "main", make_int_type(), AstNodeList{},
      std::move(main_body));
  ast.push_back(std::move(main_func));

  EXPECT_NO_THROW(validator.validate(ast));
}

// Test if statement validation
TEST(ValidateTest, IfStatements) {
  Validator validator;
  AstNodeList ast;
  AstNodeList func_body;

  // Create: if (true) { int x = 5; }
  auto condition =
      std::make_unique<ConstantBoolNode>(make_test_location(), true);
  condition->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::BOOL)));

  AstNodeList then_block;
  auto const_val =
      std::make_unique<ConstantIntegerNode>(make_test_location(), 5);
  const_val->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  auto var_decl = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "x", make_int_type(), std::move(const_val));
  then_block.push_back(std::move(var_decl));

  auto if_stmt = std::make_unique<IfNode>(
      make_test_location(), std::move(condition), std::move(then_block));
  func_body.push_back(std::move(if_stmt));

  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "test_func", make_int_type(), AstNodeList{},
      std::move(func_body));
  ast.push_back(std::move(func));

  EXPECT_NO_THROW(validator.validate(ast));
}

// Test invalid if condition (non-boolean)
TEST(ValidateTest, InvalidIfCondition) {
  Validator validator;
  AstNodeList ast;
  AstNodeList func_body;

  // Create: if (42) { int x = 5; } (should fail)
  auto condition =
      std::make_unique<ConstantIntegerNode>(make_test_location(), 42);
  condition->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  AstNodeList then_block;
  auto const_val =
      std::make_unique<ConstantIntegerNode>(make_test_location(), 5);
  const_val->set_result_type(std::make_unique<AstType>(
      make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));

  auto var_decl = std::make_unique<VariableDeclarationNode>(
      make_test_location(), "x", make_int_type(), std::move(const_val));
  then_block.push_back(std::move(var_decl));

  auto if_stmt = std::make_unique<IfNode>(
      make_test_location(), std::move(condition), std::move(then_block));
  func_body.push_back(std::move(if_stmt));

  auto func = std::make_unique<FunctionDeclarationNode>(
      make_test_location(), "test_func", make_int_type(), AstNodeList{},
      std::move(func_body));
  ast.push_back(std::move(func));

  // Should throw exception for invalid condition type
  EXPECT_THROW(validator.validate(ast), ChaException);
}
