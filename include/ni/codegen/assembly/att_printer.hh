#pragma once

#include <iostream>

#include "ni/codegen/assembly/asm_common.hh"
#include "ni/codegen/codegen.hh"

namespace ni {
namespace codegen {
namespace assembly {
class att_printer {
public:
  att_printer(context ctx, asm_program p)
      : ctx(std::move(ctx)), p(std::move(p)){};

  const context ctx;
  const asm_program p;

  void print(std::ostream &os) const;

private:
  std::string print_operand(const asm_operand &operand) const;
};
} // namespace assembly
} // namespace codegen
} // namespace ni

std::ostream &operator<<(std::ostream &os,
                         const ni::codegen::assembly::att_printer &printer);
