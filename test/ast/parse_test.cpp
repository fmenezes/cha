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
class ASTParserTest : public ::testing::Test {

protected:
  std::string dir;
  std::string filename;

  ASTParserTest() {}

  virtual ~ASTParserTest() {}

  std::string makeTempDir() {
    char buf[] = "ni_unit_test_astparsertest_XXXXXX";
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

  void WriteFile(std::string contents) {
    std::ofstream myfile;
    myfile.open(this->filename);
    myfile << contents;
    myfile.close();
  }

  void TearDown() override {
    remove(this->filename.c_str());
    remove(this->dir.c_str());
  }
};

TEST_F(ASTParserTest, Success) {
  WriteFile("fun main() int {\n"
              "  ret 0\n"
              "}\n");
  ni::ast::parser p;
  EXPECT_NO_THROW(p.parse(this->filename));
}

TEST_F(ASTParserTest, Fail) {
  WriteFile("fun ma.in() int {\n");
  ni::ast::parser p;
  EXPECT_THROW(p.parse(this->filename), yy::parser::syntax_error);
}

} // namespace
} // namespace test
} // namespace ni
