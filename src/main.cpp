#include <iostream>
#include <string>

#include "ast/ast.hh"
#include "codegen/asmcodegen.hh"
#include "codegen/codegen.hh"
#include "parser.tab.hh"

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
  ni::codegen::Codegen *c = nullptr;
  try {
    parser.parse(input);
    ni::ast::Validator::validate(*parser.program);
    
    if (format.compare("-asm") == 0) {
      c = new ni::codegen::ASMCodegen(*parser.program);
    } else {
      std::cerr << "Error: invalid format" << std::endl << std::endl;
      printUsage(argv[0]);
      return 1;
    }
    c->codegen(output);
  } catch (const yy::parser::syntax_error &e) {
    std::cerr << ni::ast::emitLocation(e.location)
              << ": error occurred: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "error occurred: " << e.what() << std::endl;
  }
  if (c != nullptr) {
    delete c;
  }
  return 0;
}
