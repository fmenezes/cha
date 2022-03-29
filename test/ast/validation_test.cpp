#include <stdio.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>

#include "ast/ast.hh"
#include "generated/location.hh"
#include "generated/parser.tab.hh"

namespace ni {
namespace test {
namespace {
class ASTValidatorTest : public ::testing::Test {

protected:
  std::string dir;
  std::string filename;

  ASTValidatorTest() {}

  virtual ~ASTValidatorTest() {}

  std::string makeTempDir() {
    char buf[] = "ni_unit_test_astvalidatortest_XXXXXX";
    char *tmp = mkdtemp(buf);
    if (tmp == nullptr) {
      throw std::runtime_error("unexpected error: " + std::to_string(errno));
    }
    return std::string("./") + tmp;
  }

  void SetUp() override {
    this->dir = makeTempDir();
    this->filename.append(this->dir);
    this->filename.append("/test");
  }

  ni::ast::NProgram* Parse(std::string contents) {
    std::ofstream myfile;
    myfile.open(this->filename);
    myfile << contents;
    myfile.close();

    ni::ast::Parser parser;
    parser.parse(this->filename);
    return parser.program.release();
  }

  void TearDown() override {
    remove(this->filename.c_str());
    remove(this->dir.c_str());
  }
};

TEST_F(ASTValidatorTest, DupFunctionTest) {
  auto p = Parse("fun test() {\n"
                 "  ret\n"
                 "}\n"
                 "fun test() {\n"
                 "  ret\n"
                 "}\n");
  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, FunctionNotFoundTest) {
  auto p = Parse("fun main() {\n"
                 "  test()\n"
                 "}\n");
  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, VarNotFoundTest) {
  auto p = Parse("fun main() {\n"
                 "  ret a\n"
                 "}\n");

  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, VarRedefinedTest) {
  auto p = Parse("fun main() {\n"
                 "  var a int\n"
                 "  var a int\n"
                 "}\n");

  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, VarRedefined2Test) {
  auto p = Parse("fun main(a int) {\n"
                 "  var a int\n"
                 "}\n");

  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, ArgMismatchTest) {
  auto p = Parse("fun main() {\n"
                 "  ret test(10)\n"
                 "}\n"
                 "fun test() {\n"
                 "  ret\n"
                 "}\n");

  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, ArgMismatch2Test) {
  auto p = Parse("fun main() {\n"
                 "  ret test()\n"
                 "}\n"
                 "fun test(i int) int {\n"
                 "  ret i\n"
                 "}\n");

  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

TEST_F(ASTValidatorTest, DupArgTest) {
  auto p = Parse("fun test(i int, i int) int {\n"
                 "  ret i\n"
                 "}\n");

  EXPECT_THROW(ni::ast::Validator::validate(*p), yy::parser::syntax_error);
}

} // namespace
} // namespace test
} // namespace ni
