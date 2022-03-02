#include "nodes.hh"
#include <iostream>
#include <string>

void printUsage(const std::string &app) {
  std::cerr << "Usage: " << app << " <format> <srcfile> <destfile>" << std::endl
            << "format: -asm for Assembly" << std::endl;
}

int main(int argc, char *argv[]) {
  ni::NProgram program;
  int op;
  if (argc != 4) {
    printUsage(argv[0]);
    return 1;
  }

  auto format = std::string(argv[1]);
  auto input = std::string(argv[2]);
  auto output = std::string(argv[3]);

  op = program.parse(input);
  if (op != 0) {
    return 1;
  }

  std::string error;
  ni::Codegen *c;
  if (format.compare("-asm") == 0) {
    c = new ni::ASMCodegen(program);
  } else {
    std::cerr << "Error: invalid format" << std::endl << std::endl;
    printUsage(argv[0]);
    return 1;
  }
  if (c->codegen(output, error) == 1) {
    delete c;
    std::cerr << error << std::endl;
    return 1;
  }
  delete c;
  return 0;
}
