#include <gtest/gtest.h>
#include "ast.hpp"

using namespace cha;

TEST(AstTest, ConstantNodes) {
  AstLocation loc("test.cha", 1, 1, 1, 10);

  // Test integer constant
  auto int_node = std::make_unique<ConstantIntegerNode>(loc, "42");
  auto int_const = dynamic_cast<const ConstantIntegerNode *>(int_node.get());
  ASSERT_NE(int_const, nullptr);
  EXPECT_EQ(int_const->value(), "42");
  ASSERT_NE(int_const->result_type(), nullptr);
  EXPECT_TRUE(int_const->result_type()->is_primitive());
  EXPECT_EQ(int_const->result_type()->as_primitive().type,
            PrimitiveType::CONST_INT);

  // Test boolean constant
  auto bool_node = std::make_unique<ConstantBoolNode>(loc, true);
  auto bool_const = dynamic_cast<const ConstantBoolNode *>(bool_node.get());
  ASSERT_NE(bool_const, nullptr);
  EXPECT_EQ(bool_const->value(), true);

  // Test float constant
  auto float_node = std::make_unique<ConstantFloatNode>(loc, 3.14);
  auto float_const = dynamic_cast<const ConstantFloatNode *>(float_node.get());
  ASSERT_NE(float_const, nullptr);
  EXPECT_EQ(float_const->value(), 3.14);
}

TEST(AstTest, BinaryOperations) {
  AstLocation loc("test.cha", 2, 1, 2, 10);

  auto left = std::make_unique<ConstantIntegerNode>(loc, "10");
  auto right = std::make_unique<ConstantIntegerNode>(loc, "20");
  auto bin_op = std::make_unique<BinaryOpNode>(
      loc, Operator::ADD, std::move(left), std::move(right));

  auto bin_node = dynamic_cast<const BinaryOpNode *>(bin_op.get());
  ASSERT_NE(bin_node, nullptr);
  EXPECT_EQ(bin_node->op(), Operator::ADD);

  auto left_const =
      dynamic_cast<const ConstantIntegerNode *>(&bin_node->left());
  auto right_const =
      dynamic_cast<const ConstantIntegerNode *>(&bin_node->right());
  ASSERT_NE(left_const, nullptr);
  EXPECT_EQ(left_const->value(), "10");
  ASSERT_NE(right_const, nullptr);
  EXPECT_EQ(right_const->value(), "20");
}

TEST(AstTest, VariableDeclarations) {
  AstLocation loc("test.cha", 3, 1, 3, 20);

  auto type =
      std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT});
  auto value = std::make_unique<ConstantIntegerNode>(loc, "100");
  auto var_decl = std::make_unique<VariableDeclarationNode>(
      loc, "myVar", std::move(type), std::move(value));

  auto var_node = dynamic_cast<const VariableDeclarationNode *>(var_decl.get());
  ASSERT_NE(var_node, nullptr);
  EXPECT_EQ(var_node->identifier(), "myVar");
  EXPECT_TRUE(var_node->type().is_primitive());
  EXPECT_EQ(var_node->type().as_primitive().type, PrimitiveType::INT);
  ASSERT_NE(var_node->value(), nullptr);

  auto value_const =
      dynamic_cast<const ConstantIntegerNode *>(var_node->value());
  ASSERT_NE(value_const, nullptr);
  EXPECT_EQ(value_const->value(), "100");
}

TEST(AstTest, FunctionDeclarations) {
  AstLocation loc("test.cha", 4, 1, 8, 1);

  // Create arguments
  AstNodeList args;
  args.push_back(std::make_unique<ArgumentNode>(
      loc, "x",
      std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT})));
  args.push_back(std::make_unique<ArgumentNode>(
      loc, "y",
      std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT})));

  // Create function body
  AstNodeList body;
  auto return_expr = std::make_unique<BinaryOpNode>(
      loc, Operator::ADD, std::make_unique<VariableLookupNode>(loc, "x"),
      std::make_unique<VariableLookupNode>(loc, "y"));
  body.push_back(
      std::make_unique<FunctionReturnNode>(loc, std::move(return_expr)));

  auto func_decl = std::make_unique<FunctionDeclarationNode>(
      loc, "add",
      std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT}),
      std::move(args), std::move(body));

  auto func_node =
      dynamic_cast<const FunctionDeclarationNode *>(func_decl.get());
  ASSERT_NE(func_node, nullptr);
  EXPECT_EQ(func_node->identifier(), "add");
  EXPECT_TRUE(func_node->return_type().is_primitive());
  EXPECT_EQ(func_node->return_type().as_primitive().type, PrimitiveType::INT);
  EXPECT_EQ(func_node->arguments().size(), 2);
  EXPECT_EQ(func_node->body().size(), 1);
}

TEST(AstTest, NodeCloning) {
  AstLocation loc("test.cha", 5, 1, 5, 10);

  auto original = std::make_unique<ConstantIntegerNode>(loc, "123");
  auto cloned = original->clone();

  auto orig_const = dynamic_cast<const ConstantIntegerNode *>(original.get());
  auto clone_const = dynamic_cast<const ConstantIntegerNode *>(cloned.get());

  ASSERT_NE(orig_const, nullptr);
  ASSERT_NE(clone_const, nullptr);
  EXPECT_EQ(orig_const->value(), clone_const->value());
  EXPECT_NE(orig_const, clone_const); // Different objects
}

TEST(AstTest, TypeSystem) {
  AstLocation loc("test.cha", 6, 1, 6, 10);

  // Test primitive type
  auto int_type =
      std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT});
  EXPECT_TRUE(int_type->is_primitive());
  EXPECT_FALSE(int_type->is_array());
  EXPECT_FALSE(int_type->is_identifier());
  EXPECT_EQ(int_type->as_primitive().type, PrimitiveType::INT);

  // Test array type
  auto elem_type = std::make_unique<AstType>(
      loc, AstType::Primitive{PrimitiveType::FLOAT32});
  auto array_type =
      std::make_unique<AstType>(loc, AstType::Array{std::move(elem_type), 10});
  EXPECT_FALSE(array_type->is_primitive());
  EXPECT_TRUE(array_type->is_array());
  EXPECT_FALSE(array_type->is_identifier());
  EXPECT_EQ(array_type->as_array().size, 10);
  EXPECT_TRUE(array_type->as_array().element_type->is_primitive());
  EXPECT_EQ(array_type->as_array().element_type->as_primitive().type,
            PrimitiveType::FLOAT32);

  // Test identifier type
  auto id_type =
      std::make_unique<AstType>(loc, AstType::Identifier{"MyCustomType"});
  EXPECT_FALSE(id_type->is_primitive());
  EXPECT_FALSE(id_type->is_array());
  EXPECT_TRUE(id_type->is_identifier());
  EXPECT_EQ(id_type->as_identifier().name, "MyCustomType");
}
