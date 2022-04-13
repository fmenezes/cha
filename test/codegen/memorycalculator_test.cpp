#include <stdio.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>

#include "ast/ast.hh"
#include "ast/parser.hh"
#include "codegen/memory_calculator.hh"
#include "generated/parser.tab.hh"

namespace ni {
namespace test {
namespace {
class MemoryCalculatorTest : public ::testing::Test {

protected:
  std::string dir;
  std::string filename;

  MemoryCalculatorTest() {}

  virtual ~MemoryCalculatorTest() {}

  std::string makeTempDir() {
    char buf[] = "ni_unit_test_memorycalculatortest_XXXXXX";
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

  ni::ast::program *Parse(std::string contents) {
    std::ofstream myfile;
    myfile.open(this->filename);
    myfile << contents;
    myfile.close();

    ni::ast::parser parser;
    parser.parse(this->filename);
    return parser.prg.release();
  }

  void TearDown() override {
    remove(this->filename.c_str());
    remove(this->dir.c_str());
  }
};

TEST_F(MemoryCalculatorTest, NoMemory) {
  auto p = Parse("fun main() int {\n"
                 "  ret 1\n"
                 "}\n");
  auto &funMain = *p->instructions[0];
  EXPECT_EQ(ni::codegen::memory_calculator::calculare(funMain), 0);
}

TEST_F(MemoryCalculatorTest, OneArg) {
  auto p = Parse("fun main(i int) int {\n"
                 "  ret i\n"
                 "}\n");
  auto &funMain = *p->instructions[0];
  EXPECT_EQ(ni::codegen::memory_calculator::calculare(funMain), 4);
}

TEST_F(MemoryCalculatorTest, OneVar) {
  auto p = Parse("fun main() int {\n"
                 "  var i int\n"
                 "  ret i\n"
                 "}\n");
  auto &funMain = *p->instructions[0];
  EXPECT_EQ(ni::codegen::memory_calculator::calculare(funMain), 4);
}

TEST_F(MemoryCalculatorTest, OneArgOneVar) {
  auto p = Parse("fun main(i int) int {\n"
                 "  var j int\n"
                 "  ret i + j\n"
                 "}\n");
  auto &funMain = *p->instructions[0];
  EXPECT_EQ(ni::codegen::memory_calculator::calculare(funMain), 8);
}

} // namespace
} // namespace test
} // namespace ni
