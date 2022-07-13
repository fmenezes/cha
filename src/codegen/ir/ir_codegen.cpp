#include <exception>
#include <fstream>
#include <iostream>

#include "ni/ast/ast.hh"
#include "ni/codegen/codegen.hh"
#include "ni/codegen/ir/ir_codegen.hh"
#include "ni/codegen/ir/ir_printer.hh"
#include "ni/codegen/memory_calculator.hh"

void ni::codegen::ir::ir_codegen::visit(const ni::ast::constant_integer &node) {
  return_operand = ni::codegen::ir::ir_operand(
      ni::codegen::ir::ir_operand_type::CONSTANT, node.value);
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::binary_operation &node) {
  ni::ast::visitor::visit(*node.left);
  ni::codegen::ir::ir_operand laddr = return_operand;

  ni::ast::visitor::visit(*node.right);
  ni::codegen::ir::ir_operand raddr = return_operand;

  return_operand = ni::codegen::ir::ir_operand(
      ni::codegen::ir::ir_operand_type::TEMPORARY,
      "TMP" + std::to_string(current_temprary_id++));

  if (node.op.compare("+") == 0) {
    append_instruction(
        ir_instruction(ir_operation::ADD,
                       std::vector<ir_operand>{return_operand, raddr, laddr}));
  } else if (node.op.compare("-") == 0) {
    append_instruction(
        ir_instruction(ir_operation::SUB,
                       std::vector<ir_operand>{return_operand, raddr, laddr}));
  } else if (node.op.compare("*") == 0) {
    append_instruction(
        ir_instruction(ir_operation::MUL,
                       std::vector<ir_operand>{return_operand, raddr, laddr}));
  } else {
    throw std::runtime_error("invalid operation " + node.op);
  }
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::variable_lookup &node) {
  auto s = vars.find(node.identifier);
  if (s == vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  return_operand = s->second;
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::variable_assignment &node) {
  auto s = vars.find(node.identifier);
  if (s == vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  ni::ast::visitor::visit(*node.value);
  append_instruction(ir_instruction(
      ir_operation::MOV, std::vector<ir_operand>{s->second, return_operand}));
  return_operand = s->second;
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::variable_declaration &node) {
  auto s = vars.find(node.identifier);
  if (s != vars.end()) {
    throw std::runtime_error(node.identifier + " already exists.");
  }

  return_operand = ir_operand(ir_operand_type::MEMORY, node.identifier);
  append_instruction(ir_instruction(
      generate_label(), ir_operation::ALLOC,
      std::vector<ir_operand>{return_operand,
                              ir_operand(ir_operand_type::CONSTANT, "4")}));
  vars.insert({node.identifier, return_operand});
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::function_declaration &node) {
  current_function_name = node.identifier;

  append_instruction(
      ir_instruction(ir_operation::GLOBAL,
                     std::vector<ir_operand>{ir_operand(
                         ir_operand_type::LABEL, current_function_name)}));
  current_temprary_id = 0;
  vars.clear();

  for (int a = 0; a < node.args.size(); a++) {
    auto &arg = *node.args[a];

    auto addr =
        ir_operand(ir_operand_type::TEMPORARY, "ARG" + std::to_string(a));
    vars.insert({arg.identifier, addr});
  }

  first_instruction = true;
  ni::ast::visitor::visit(*node.body);
  first_instruction = false; // if body empty

  append_instruction(ir_instruction(current_function_name + "_epilogue",
                                    ir_operation::RET,
                                    std::vector<ir_operand>{return_operand}));
}

std::string ni::codegen::ir::ir_codegen::generate_label() {
  if (first_instruction) {
    first_instruction = false;
    return current_function_name;
  }
  return "";
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::function_call &node) {
  for (int i = 0; i < node.params.size(); i++) {
    auto &param = *node.params[i];
    ni::ast::visitor::visit(param);
    auto arg =
        ir_operand(ir_operand_type::TEMPORARY, "ARG" + std::to_string(i));
    append_instruction(ir_instruction(
        ir_operation::MOV, std::vector<ir_operand>{arg, return_operand}));
  }

  return_operand = ir_operand(ir_operand_type::TEMPORARY,
                              "TMP" + std::to_string(current_temprary_id++));

  append_instruction(ir_instruction(
      generate_label(), ir_operation::CALL,
      std::vector<ir_operand>{return_operand, ir_operand(ir_operand_type::LABEL,
                                                         node.identifier)}));
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::function_return &node) {
  if (node.value.get() != nullptr) {
    ni::ast::visitor::visit(*node.value);
  } else {
    return_operand = ir_operand(ir_operand_type::CONSTANT, "0");
  }
  append_instruction(ir_instruction(
      generate_label(), ir_operation::JMP,
      std::vector<ir_operand>{
          return_operand, ir_operand(ir_operand_type::LABEL,
                                     current_function_name + "_epilogue")}));
}

void ni::codegen::ir::ir_codegen::generate_exit_call() {
  append_instruction(ir_instruction(ir_operation::EXIT,
                                    std::vector<ir_operand>{return_operand}));
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::program &node) {
  generate_start_function();
  ni::ast::visitor::visit(node);
}

void ni::codegen::ir::ir_codegen::generate_start_function() {
  return_operand = ir_operand(ir_operand_type::TEMPORARY,
                              "TMP" + std::to_string(current_temprary_id++));

  append_instruction(ir_instruction(
      "start", ir_operation::CALL,
      std::vector<ir_operand>{return_operand,
                              ir_operand(ir_operand_type::LABEL, "main")}));
  generate_exit_call();
}

void ni::codegen::ir::ir_codegen::generate(const std::string &output) {
  visit(program);

  std::ofstream output_file;
  output_file.open(output, std::ios::trunc);
  output_file << ni::codegen::ir::ir_printer(instructions);
  output_file.close();
}

void ni::codegen::ir::ir_codegen::append_instruction(
    ni::codegen::ir::ir_instruction instruction) {
  instruction.label = generate_label();
  append_instruction(std::move(instruction));
}
