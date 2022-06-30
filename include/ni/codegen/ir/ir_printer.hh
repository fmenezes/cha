#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "ni/codegen/codegen.hh"

namespace ni {
namespace codegen {
namespace ir {
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
      return "&" + _identifier;
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
  ir_printer(std::shared_ptr<std::ostream> out) : out(std::move(out)){};

  void global(const std::string &label);
  void alloc(const ir_operand &dst, const int b);
  void label(const std::string &label);
  void mov(const ir_operand &dst, const ir_operand &src);
  void add(const ir_operand &dst, const ir_operand &src,
           const ir_operand &src2);
  void sub(const ir_operand &dst, const ir_operand &src,
           const ir_operand &src2);
  void mul(const ir_operand &dst, const ir_operand &src,
           const ir_operand &src2);
  void jmp(const std::string &label);
  void call(const ir_operand &dst, const std::string &label);
  void push(const ir_operand &src);
  void pop(const ir_operand &dst);
  void exit(const ir_operand &src);
  void ret(const ir_operand &src);

private:
  std::shared_ptr<std::ostream> out;
};

} // namespace ir
} // namespace codegen
} // namespace ni
