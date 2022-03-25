#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>

#include "codegen/attprinter.hh"

namespace ni {
namespace test {
namespace {
class ATTPrinterTest : public ::testing::Test {

protected:
  std::string filename;
  ni::codegen::ATTPrinter *printer;

  ATTPrinterTest() {}

  virtual ~ATTPrinterTest() {}

  std::string makeTempDir() {
    char buf[] = "niXXXXXX";
    char *tmp = mkdtemp(buf);
    if (tmp == nullptr) {
      throw std::runtime_error("unexpected error: " + std::to_string(errno));
    }
    return tmp;
  }

  void SetUp(const ni::codegen::Context &c) {
    this->printer = new ni::codegen::ATTPrinter(c);
    this->filename.append(makeTempDir());
    this->filename.append("test");

    printer->openFile(filename);
  }

  std::string readFile() {
    std::ifstream t(this->filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
  }

  void TearDown() override {
    printer->closeFile();
    delete printer;
  }
};

TEST_F(ATTPrinterTest, Linux) {
  this->SetUp(
      ni::codegen::Context(ni::codegen::OS::LINUX, ni::codegen::ARCH::x86_64));

  std::string expected;
  expected.append(".text\n");
  expected.append(".globl\t_start\n");
  expected.append("_start:\n");
  expected.append("\tcallq\ttest\n");
  expected.append(".globl\ttest\n");
  expected.append("test:\n");
  expected.append("\tmovq\t$10, %rax\n");
  expected.append("\tmovl\t$10, %eax\n");
  expected.append("\taddl\t$20, %edi\n");
  expected.append("\tsubl\t$30, %ecx\n");
  expected.append("\timull\t$40, %edx\n");
  expected.append("\tpushq\t$50\n");
  expected.append("\tsyscall\n");
  expected.append("\tjmp\ttest2\n");
  expected.append("\tretq\n");

  this->printer->textHeader();
  this->printer->labelStart();
  this->printer->call("test");
  this->printer->global("test");
  this->printer->label("test");
  this->printer->mov(ni::codegen::Register64Bits::RAX, 10);
  this->printer->mov(ni::codegen::Register32Bits::EAX, 10);
  this->printer->add(ni::codegen::Register32Bits::EDI, 20);
  this->printer->sub(ni::codegen::Register32Bits::ECX, 30);
  this->printer->imul(ni::codegen::Register32Bits::EDX, 40);
  this->printer->push(50);
  this->printer->syscall();
  this->printer->jmp("test2");
  this->printer->ret();
  EXPECT_EQ(this->readFile(), expected);
}

TEST_F(ATTPrinterTest, MacOS) {
  this->SetUp(
      ni::codegen::Context(ni::codegen::OS::MACOS, ni::codegen::ARCH::x86_64));

  std::string expected;
  expected.append(".section\t__TEXT,__text\n");
  expected.append(".globl\tstart\n");
  expected.append("start:\n");
  expected.append("\tcallq\t_test\n");
  expected.append(".globl\t_test\n");
  expected.append("_test:\n");
  expected.append("\tmovq\t$10, %rax\n");
  expected.append("\tmovl\t$10, %eax\n");
  expected.append("\taddl\t$20, %edi\n");
  expected.append("\tsubl\t$30, %ecx\n");
  expected.append("\timull\t$40, %edx\n");
  expected.append("\tpushq\t$50\n");
  expected.append("\tsyscall\n");
  expected.append("\tjmp\t_test2\n");
  expected.append("\tretq\n");

  this->printer->textHeader();
  this->printer->labelStart();
  this->printer->call("test");
  this->printer->global("test");
  this->printer->label("test");
  this->printer->mov(ni::codegen::Register64Bits::RAX, 10);
  this->printer->mov(ni::codegen::Register32Bits::EAX, 10);
  this->printer->add(ni::codegen::Register32Bits::EDI, 20);
  this->printer->sub(ni::codegen::Register32Bits::ECX, 30);
  this->printer->imul(ni::codegen::Register32Bits::EDX, 40);
  this->printer->push(50);
  this->printer->syscall();
  this->printer->jmp("test2");
  this->printer->ret();
  EXPECT_EQ(this->readFile(), expected);
}
} // namespace
} // namespace test
} // namespace ni
