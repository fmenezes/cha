#include <iostream>
#include <string>

#include "ast/ast.hh"
#include "codegen/asmcodegen.hh"
#include "codegen/codegen.hh"

void printUsage(const std::string &app) {
  std::cerr << "Usage: " << app << " <format> <srcfile> <destfile>" << std::endl
            << "format: -asm for Assembly" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printUsage(argv[0]);
    return 1;
  }

  auto format = std::string(argv[1]);
  auto input = std::string(argv[2]);
  auto output = std::string(argv[3]);

  ni::ast::Parser parser;
  auto program = parser.parse(input);

  std::string error;
  ni::codegen::Codegen *c;
  if (format.compare("-asm") == 0) {
    c = new ni::codegen::ASMCodegen(*program);
  } else {
    std::cerr << "Error: invalid format" << std::endl << std::endl;
    printUsage(argv[0]);
    return 1;
  }
  c->codegen(output);
  delete c;
  return 0;
}
