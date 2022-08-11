#include <iostream>
#include <string>

#include "ni/ast/ast.hh"
#include "ni/ast/validator.hh"
#include "ni/codegen/codegen.hh"
#include "ni/parse/syntax_parser.hh"

void printUsage(const std::string &app) {
  std::cerr << "Usage: " << app << " <format> <outputfile> <inputfile>"
            << std::endl
            << "format: -s for Assembly Code" << std::endl
            << "format: -c for Object File" << std::endl
            << "format: -ll for LLVM IR" << std::endl
            << "format: -o for Binary" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printUsage(argv[0]);
    return 1;
  }

  auto format = std::string(argv[1]);
  auto outputFile = std::string(argv[2]);
  auto inputFile = std::string(argv[3]);

  ni::codegen::format f;
  if (format.compare("-s") == 0) {
    f = ni::codegen::format::ASSEMBLY_CODE;
  } else if (format.compare("-ll") == 0) {
    f = ni::codegen::format::LLVM_IR;
  } else if (format.compare("-c") == 0) {
    f = ni::codegen::format::OBJECT_FILE;
  } else if (format.compare("-o") == 0) {
    f = ni::codegen::format::BINARY_FILE;
  } else {
    std::cerr << "Error: invalid format" << std::endl << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  try {
    ni::parse::syntax_parser parser;
    parser.parse(inputFile);
    ni::ast::validator::validate(*parser.prg);
    ni::codegen::codegen c(*parser.prg);
    c.generate(outputFile, f);
  } catch (const ni::parse::syntax_error &e) {
    std::cerr << e.loc.str() << ": error occurred: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "error occurred: " << e.what() << std::endl;
  }
  return 0;
}
