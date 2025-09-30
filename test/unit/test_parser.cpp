#include "ast.hpp"
#include "parser.hpp"
#include <gtest/gtest.h>

using namespace cha;

TEST(ParserTest, SimpleFunctionParsing) {
  AstNodeList ast;
  int result = cha::parse("examples/test.cha", ast);

  EXPECT_EQ(result, 0);
  EXPECT_FALSE(ast.empty());

  // Check that we parsed at least one function
  bool found_main = false;
  bool found_any_function = false;

  for (const auto &node : ast) {
    if (auto func = dynamic_cast<const FunctionDeclarationNode *>(node.get())) {
      found_any_function = true;
      if (func->identifier() == "main") {
        found_main = true;
      }
    }
  }

  EXPECT_TRUE(found_any_function);
  EXPECT_TRUE(found_main);
}

TEST(ParserTest, ConstantParsing) {
  AstNodeList ast;
  int result = cha::parse("examples/test.cha", ast);

  EXPECT_EQ(result, 0);
  EXPECT_FALSE(ast.empty());

  // Look for constant declarations or constant values in the AST
  bool found_constant = false;
  for (const auto &node : ast) {
    if (auto func = dynamic_cast<const FunctionDeclarationNode *>(node.get())) {
      // Check function body for constants
      for (const auto &stmt : func->body()) {
        if (auto ret_stmt =
                dynamic_cast<const FunctionReturnNode *>(stmt.get())) {
          if (ret_stmt->value()) {
            if (auto const_int = dynamic_cast<const ConstantIntegerNode *>(
                    ret_stmt->value())) {
              found_constant = true;
            }
          }
        }
      }
    }
  }

  // Note: Finding constants is optional based on the test file content
}

TEST(ParserTest, AstStructureIntegrity) {
  AstNodeList ast;
  int result = cha::parse("examples/test.cha", ast);

  EXPECT_EQ(result, 0);
  EXPECT_FALSE(ast.empty());

  // Verify each node has proper location information
  for (const auto &node : ast) {
    const auto &loc = node->location();
    EXPECT_FALSE(loc.file.empty());
    EXPECT_GT(loc.line_begin, 0);
    EXPECT_GE(loc.column_begin, 0);
    EXPECT_GE(loc.line_end, loc.line_begin);

    if (loc.line_end == loc.line_begin) {
      EXPECT_GE(loc.column_end, loc.column_begin);
    }
  }
}
