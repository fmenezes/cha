#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "ni/codegen/codegen.hh"
#include "ni/codegen/ir/ir_common.hh"

namespace ni {
namespace codegen {
namespace ir {
class ir_printer {
public:
  ir_printer(ir_program program) : program(std::move(program)){};

  void print(std::ostream &out) const;

private:
  ir_program program;
};

} // namespace ir
} // namespace codegen
} // namespace ni

std::ostream &operator<<(std::ostream &os,
                         const ni::codegen::ir::ir_printer &printer);
