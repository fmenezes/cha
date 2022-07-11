#include <unistd.h>
#include <string>
#include <fstream>
#include <memory>

#include "ni/ast/ast.hh"
#include "test/test.hh"
#include "ni/codegen/memory_calculator.hh"


int test_codegen_memorycalculator_one_var(int argc, char *argv[]) {
  auto p = parse("codegen/memorycalculator_one_var.ni");

  auto &funMain = *p->instructions[0];

  int expected = 4;
  auto got = ni::codegen::memory_calculator::calculare(funMain);

  if (expected != got) {
    std::cerr << "expected " << expected << " got " << got  << std::endl;
    return 1;
  }

  return 0;
}
