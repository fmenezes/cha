#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "codegen.h"
#include "log.h"
#include "parser.h"
#include "validate.h"

int main(int argc, char *argv[]) {
  if (argc == 2 && strcmp(argv[1], "--version") == 0) {
    fprintf(stderr, "%s %s\n", CMAKE_PROJECT_NAME, CMAKE_PROJECT_VERSION);
    return 0;
  }

  if (argc != 4) {
    fprintf(stderr, "Usage: --version | %s <format> <outputfile> <inputfile>\n",
            argv[0]);
    fprintf(stderr, "format: -s for Assembly Code\n");
    fprintf(stderr, "format: -c for Object File\n");
    fprintf(stderr, "format: -ll for LLVM IR\n");
    fprintf(stderr, "format: -o for Binary File\n");
    return 1;
  }

  char *format = argv[1];
  char *outputfile = argv[2];
  char *inputfile = argv[3];

  ni_compile_format compile_format;
  if (strcmp(format, "-s") == 0) {
    compile_format = NI_COMPILE_FORMAT_ASSEMBLY_FILE;
  } else if (strcmp(format, "-c") == 0) {
    compile_format = NI_COMPILE_FORMAT_OBJECT_FILE;
  } else if (strcmp(format, "-ll") == 0) {
    compile_format = NI_COMPILE_FORMAT_LLVM_IR;
  } else if (strcmp(format, "-o") == 0) {
    compile_format = NI_COMPILE_FORMAT_BINARY_FILE;
  } else {
    log_error("Invalid format: %s\n", format);
    return 1;
  }

  return ni_compile(inputfile, compile_format, outputfile);
}
