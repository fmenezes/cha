#include <stdio.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>

#include "generated/location.hh"
#include "ni/ast/ast.hh"
#include "ni/ast/validator.hh"
#include "ni/parse/syntax_parser.hh"

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

  ni::ast::program* Parse(std::string contents) {
    std::ofstream myfile;
    myfile.open(this->filename);
    myfile << contents;
    myfile.close();

    ni::parse::syntax_parser parser;
    parser.parse(this->filename);
    return parser.prg.release();
  }

  void TearDown() override {
    remove(this->filename.c_str());
    remove(this->dir.c_str());
  }
};

TEST_F(ASTValidatorTest, Passes) {
  auto p = Parse("fun main() int {\n"
                 "  ret 1\n"
                 "}\n");
  EXPECT_NO_THROW(ni::ast::validator::validate(*p));
}

TEST_F(ASTValidatorTest, DupFunctionTest) {
  auto p = Parse("fun test() {\n"
                 "  ret\n"
                 "}\n"
                 "fun test() {\n"
                 "  ret\n"
                 "}\n");
  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, FunctionNotFoundTest) {
  auto p = Parse("fun main() {\n"
                 "  test()\n"
                 "}\n");
  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, VarNotFoundTest) {
  auto p = Parse("fun main() {\n"
                 "  ret a\n"
                 "}\n");

  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, VarRedefinedTest) {
  auto p = Parse("fun main() {\n"
                 "  var a int\n"
                 "  var a int\n"
                 "}\n");

  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, VarRedefined2Test) {
  auto p = Parse("fun main(a int) {\n"
                 "  var a int\n"
                 "}\n");

  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, ArgMismatchTest) {
  auto p = Parse("fun main() {\n"
                 "  ret test(10)\n"
                 "}\n"
                 "fun test() {\n"
                 "  ret\n"
                 "}\n");

  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, ArgMismatch2Test) {
  auto p = Parse("fun main() {\n"
                 "  ret test()\n"
                 "}\n"
                 "fun test(i int) int {\n"
                 "  ret i\n"
                 "}\n");

  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

TEST_F(ASTValidatorTest, DupArgTest) {
  auto p = Parse("fun test(i int, i int) int {\n"
                 "  ret i\n"
                 "}\n");

  EXPECT_THROW(ni::ast::validator::validate(*p), ni::parse::syntax_error);
}

} // namespace
} // namespace test
} // namespace ni
