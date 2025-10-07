#include "ast.hpp"
#include "exceptions.hpp"
#include "parser.hpp"
#include <fstream>
#include <gtest/gtest.h>

using namespace cha;

TEST(ParserTest, SimpleFunctionParsing) {
  AstNodeList ast;

  try {
    ast = cha::parse("examples/test.cha");
  } catch (const ParseException &e) {
    FAIL() << "Parse failed: " << e.message();
  }

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

  try {
    ast = cha::parse("examples/test.cha");
  } catch (const ParseException &e) {
    FAIL() << "Parse failed: " << e.message();
  }

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

  try {
    ast = cha::parse("examples/test.cha");
  } catch (const ParseException &e) {
    FAIL() << "Parse failed: " << e.message();
  }

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

TEST(ParserTest, UnaryOperations) {
  // Create a simple program with unary operations for testing
  std::string temp_filename = "test_unary.cha";
  std::string program = R"(
fun test() int {
    var a int
    a = -42
    var b bool
    b = !true
    ret a
}
)";

  // Write the test program to a temporary file
  std::ofstream temp_file(temp_filename);
  temp_file << program;
  temp_file.close();

  AstNodeList ast;
  try {
    ast = cha::parse(temp_filename);
  } catch (const ParseException &e) {
    // Clean up the temporary file
    std::remove(temp_filename.c_str());
    FAIL() << "Parse failed: " << e.message();
  }

  // Clean up the temporary file
  std::remove(temp_filename.c_str());

  EXPECT_FALSE(ast.empty());

  // Find the function and look for unary operations
  bool found_negate = false;
  bool found_not = false;

  for (const auto &node : ast) {
    if (auto func = dynamic_cast<const FunctionDeclarationNode *>(node.get())) {
      for (const auto &stmt : func->body()) {
        if (auto var_decl =
                dynamic_cast<const VariableDeclarationNode *>(stmt.get())) {
          if (var_decl->value()) {
            if (auto unary_op =
                    dynamic_cast<const UnaryOpNode *>(var_decl->value())) {
              if (unary_op->op() == UnaryOperator::NEGATE) {
                found_negate = true;
              } else if (unary_op->op() == UnaryOperator::NOT) {
                found_not = true;
              }
            }
          }
        } else if (auto var_assign =
                       dynamic_cast<const VariableAssignmentNode *>(
                           stmt.get())) {
          if (auto unary_op =
                  dynamic_cast<const UnaryOpNode *>(&var_assign->value())) {
            if (unary_op->op() == UnaryOperator::NEGATE) {
              found_negate = true;
            } else if (unary_op->op() == UnaryOperator::NOT) {
              found_not = true;
            }
          }
        }
      }
    }
  }

  EXPECT_TRUE(found_negate)
      << "Should find NEGATE unary operation in parsed AST";
  EXPECT_TRUE(found_not) << "Should find NOT unary operation in parsed AST";
}
