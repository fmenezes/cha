#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ni {
namespace codegen {
namespace ir {
enum ir_operand_type { TEMPORARY, MEMORY, CONSTANT, LABEL };

class ir_operand {
public:
  ir_operand(const ir_operand_type &ir_operand_type,
             const std::string &identifier)
      : _operand_type(ir_operand_type), _identifier(identifier){};

  std::string get_identifier() const { return _identifier; }
  ir_operand_type get_operand_type() const { return _operand_type; }

private:
  std::string _identifier;
  ir_operand_type _operand_type;
};

enum ir_operation {
  GLOBAL,
  ALLOC,
  MOV,
  ADD,
  SUB,
  MUL,
  JMP,
  CALL,
  PUSH,
  POP,
  RET,
  EXIT
};

class ir_instruction {
public:
  explicit ir_instruction(ir_operation operation)
      : operation(std::move(operation)){};
  ir_instruction(ir_operation operation, std::vector<ir_operand> operands)
      : operation(std::move(operation)), operands(std::move(operands)){};
  ir_instruction(std::string label, ir_operation operation,
                 std::vector<ir_operand> operands)
      : label(std::move(label)), operation(std::move(operation)),
        operands(std::move(operands)){};

  std::vector<ir_operand> operands;
  ir_operation operation;
  std::string label;
};

typedef std::vector<ir_instruction> ir_program;
} // namespace ir
} // namespace codegen
} // namespace ni
