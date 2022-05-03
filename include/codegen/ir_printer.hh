#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "codegen.hh"

namespace ni {
namespace codegen {
enum ir_operand_type { TEMPORARY, MEMORY, CONSTANT };

class ir_operand {
public:
  ir_operand(const ir_operand_type &ir_operand_type,
             const std::string &identifier)
      : _operand_type(ir_operand_type), _identifier(identifier){};

  std::string get_identifier() const { return _identifier; }
  ir_operand_type get_operand_type() const { return _operand_type; }

  std::string str() const {
    switch (_operand_type) {
    case ir_operand_type::CONSTANT:
      return "$" + _identifier;
    case ir_operand_type::TEMPORARY:
      return "%" + _identifier;
    case ir_operand_type::MEMORY:
    default:
      return _identifier;
    }
  }

private:
  std::string _identifier;
  ir_operand_type _operand_type;
};

class ir_printer {
public:
  ir_printer(std::ostream *out) : out(out){};

  void text_header();
  void global(const std::string &label);
  void alloc(const ir_operand &dst, const int b);
  void label(const std::string &label);
  void mov(const ir_operand &dst, const ir_operand &src);
  void add(const ir_operand &dst, const ir_operand &src);
  void sub(const ir_operand &dst, const ir_operand &src);
  void mul(const ir_operand &dst, const ir_operand &src);
  void jmp(const std::string &label);
  void call(const std::string &label);
  void push(const ir_operand &src);
  void pop(const ir_operand &dst);
  void syscall();
  void ret();

private:
  std::ostream *out;
};

} // namespace codegen
} // namespace ni
