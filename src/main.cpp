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

std::string emitLocation(const yy::parser::syntax_error &e) {
  std::string loc = *e.location.begin.filename + ":" +
                    std::to_string(e.location.begin.line) + ":" +
                    std::to_string(e.location.begin.column);
  if (e.location.begin.filename != nullptr &&
      e.location.end.filename != nullptr &&
      e.location.begin.filename->compare(*e.location.end.filename) != 0) {
    loc += "-" + *e.location.end.filename + ":" +
           std::to_string(e.location.end.line) + ":" +
           std::to_string(e.location.end.column);
  } else if (e.location.begin.line != e.location.end.line) {
    loc += "-" + std::to_string(e.location.end.line) + ":" +
           std::to_string(e.location.end.column);
  } else if (e.location.begin.column != e.location.end.column) {
    loc += "-" + std::to_string(e.location.end.column);
  }
  return loc + ":";
}

std::string emitSyntaxError(const yy::parser::syntax_error &e) {
  return emitLocation(e) + " Error: " + e.what();
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
  ni::codegen::Codegen *c;
  try {
    parser.parse(input);

    if (format.compare("-asm") == 0) {
      c = new ni::codegen::ASMCodegen(*parser.program);
    } else {
      std::cerr << "Error: invalid format" << std::endl << std::endl;
      printUsage(argv[0]);
      return 1;
    }
    c->codegen(output);
  } catch (const yy::parser::syntax_error &e) {
    std::cerr << emitSyntaxError(e) << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  if (c != nullptr) {
    delete c;
  }
  return 0;
}
