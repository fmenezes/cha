#include <iostream>
#include <string>
#include <vector>

#include "cha/cha.hpp"

int main(int argc, char *argv[]) {
  // Convert C-style args to modern C++ vector
  std::vector<std::string> args(argv, argv + argc);

  if (args.size() == 2 && args[1] == "--version") {
    std::cerr << CMAKE_PROJECT_NAME << " " << CMAKE_PROJECT_VERSION
              << std::endl;
    return 0;
  }

  if (args.size() != 4) {
    std::cerr << "Usage: --version | " << args[0]
              << " <format> <outputfile> <inputfile>" << std::endl;
    std::cerr << "format: -s for Assembly Code" << std::endl;
    std::cerr << "format: -c for Object File" << std::endl;
    std::cerr << "format: -ll for LLVM IR" << std::endl;
    std::cerr << "format: -o for Binary File" << std::endl;
    return 1;
  }

  const std::string &format = args[1];
  const std::string &outputfile = args[2];
  const std::string &inputfile = args[3];

  cha::CompileFormat compile_format;
  if (format == "-s") {
    compile_format = cha::CompileFormat::ASSEMBLY_FILE;
  } else if (format == "-c") {
    compile_format = cha::CompileFormat::OBJECT_FILE;
  } else if (format == "-ll") {
    compile_format = cha::CompileFormat::LLVM_IR;
  } else if (format == "-o") {
    compile_format = cha::CompileFormat::BINARY_FILE;
  } else {
    std::cerr << "invalid format: " << format << std::endl;
    return 1;
  }

  return cha::compile(inputfile, compile_format, outputfile);
}
