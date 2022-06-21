#include <iostream>
#include <string>

#include "ast/ast.hh"
#include "ast/validator.hh"
#include "codegen/assembly/asm_codegen.hh"
#include "codegen/codegen.hh"
#include "codegen/ir/ir_codegen.hh"
#include "parse/syntax_parser.hh"

void printUsage(const std::string &app) {
  std::cerr << "Usage: " << app << " <format> <srcfile> <destfile>" << std::endl
            << "format: -asm for Assembly" << std::endl
            << "format: -ir for Intermediate Representation" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printUsage(argv[0]);
    return 1;
  }

  auto format = std::string(argv[1]);
  auto input = std::string(argv[2]);
  auto output = std::string(argv[3]);

  ni::parse::syntax_parser parser;
  ni::codegen::codegen *c = nullptr;
  try {
    parser.parse(input);
    ni::ast::validator::validate(*parser.prg);

    if (format.compare("-asm") == 0) {
      c = new ni::codegen::assembly::asm_codegen(*parser.prg);
    } else if (format.compare("-ir") == 0) {
      c = new ni::codegen::ir::ir_codegen(*parser.prg);
    } else {
      std::cerr << "Error: invalid format" << std::endl << std::endl;
      printUsage(argv[0]);
      return 1;
    }
    c->generate(output);
  } catch (const ni::parse::syntax_error &e) {
    std::cerr << e.loc.str() << ": error occurred: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "error occurred: " << e.what() << std::endl;
  }
  if (c != nullptr) {
    delete c;
  }
  return 0;
}
