#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <memory>
#include <stdio.h>
#include <vector>

#include "ast/ast.hh"
#include "generated/location.hh"
#include "generated/parser.tab.hh"

namespace ni {
namespace test {
namespace {

TEST(ASTValidatorTest, DupFunctionTest) {
  std::vector<std::unique_ptr<ni::ast::NStatement>> body1;
  std::vector<std::unique_ptr<ni::ast::NStatement>> body2;
  std::vector<std::unique_ptr<ni::ast::NFunctionDeclaration>> instructions;
  instructions.push_back(std::make_unique<ni::ast::NFunctionDeclaration>(
      "test", body1, yy::location()));
  instructions.push_back(std::make_unique<ni::ast::NFunctionDeclaration>(
      "test", body2, yy::location()));
  ni::ast::NProgram p(instructions, yy::location());

  EXPECT_THROW(ni::ast::Validator::validate(p), yy::parser::syntax_error);
}
} // namespace
} // namespace test
} // namespace ni
