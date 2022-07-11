#include <string>
#include <fstream>

#include "ni/ast/ast.hh"
#include "test/test.hh"
#include "ni/codegen/codegen.hh"
#include "ni/codegen/assembly/att_printer.hh"


int test_codegen_assembly_attprinter_linux(int argc, char *argv[]) {
  auto c = ni::codegen::context(ni::codegen::os::LINUX, ni::codegen::arch::x86_64);
  auto printer = new ni::codegen::assembly::att_printer(c);
  std::string dir = makeTempDir();
  std::string filename = dir + "/test";

  printer->open_file(filename);

  printer->text_header();
  printer->label_start();
  printer->call("test");
  printer->global("test");
  printer->label("test");
  printer->mov(ni::codegen::assembly::register_64bits::RAX, 10);
  printer->mov(ni::codegen::assembly::register_32bits::EAX, 10);
  printer->add(ni::codegen::assembly::register_32bits::EDI, 20);
  printer->sub(ni::codegen::assembly::register_32bits::ECX, 30);
  printer->imul(ni::codegen::assembly::register_32bits::EDX, 40);
  printer->push(50);
  printer->syscall();
  printer->jmp("test2");
  printer->ret();

  printer->close_file();

  std::ifstream gotFile(filename);
  std::stringstream gotBuffer;
  gotBuffer << gotFile.rdbuf();
  gotFile.close();
  
  remove(filename.c_str());
  remove(dir.c_str());
  delete printer;

  std::ifstream expectedFile("codegen/assembly/attprinter_linux.s");
  std::stringstream expectedBuffer;
  expectedBuffer << expectedFile.rdbuf();
  expectedFile.close();

  if (expectedBuffer.str() != gotBuffer.str()) {
    std::cout << "expected: " << expectedBuffer.str() << std::endl;
    std::cout << "actual: " << gotBuffer.str() << std::endl;
    return 1;
  }

  return 0;
}
