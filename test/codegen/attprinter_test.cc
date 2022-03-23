#include <stdio.h>
#include <fstream>
#include <gtest/gtest.h>

#include "codegen/attprinter.hh"

namespace {
class ATTPrinterTest : public ::testing::Test {

protected:
  std::string filename;
  ni::codegen::ATTPrinter *printer;

  ATTPrinterTest() {}

  virtual ~ATTPrinterTest() {}

  virtual void SetUp(const ni::codegen::Context &c) {
    this->printer = new ni::codegen::ATTPrinter(c);
    this->filename.append(std::getenv("TMPDIR"));
    this->filename.append("/test");

    printer->openFile(filename);
  }

  std::string readFile() {
    std::ifstream t(this->filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
  }

  virtual void TearDown() {
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
} // namespace
