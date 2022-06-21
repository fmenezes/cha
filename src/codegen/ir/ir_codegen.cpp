#include <exception>
#include <fstream>
#include <iostream>

#include "ast/ast.hh"
#include "codegen/codegen.hh"
#include "codegen/ir/ir_codegen.hh"
#include "codegen/memory_calculator.hh"

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
    printer->add(return_operand, raddr, laddr);
  } else if (node.op.compare("-") == 0) {
    printer->sub(return_operand, raddr, laddr);
  } else if (node.op.compare("*") == 0) {
    printer->mul(return_operand, raddr, laddr);
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
  printer->mov(s->second, return_operand);
  return_operand = s->second;
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::variable_declaration &node) {
  auto s = vars.find(node.identifier);
  if (s != vars.end()) {
    throw std::runtime_error(node.identifier + " already exists.");
  }

  return_operand = ir_operand(ir_operand_type::MEMORY, node.identifier);
  printer->alloc(return_operand, 4);
  vars.insert({node.identifier, return_operand});
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::function_declaration &node) {
  current_function_name = node.identifier;

  printer->global(current_function_name);
  printer->label(current_function_name);

  current_temprary_id = 0;
  vars.clear();

  for (int a = 0; a < node.args.size(); a++) {
    auto &arg = *node.args[a];

    auto addr =
        ir_operand(ir_operand_type::TEMPORARY, "ARG" + std::to_string(a));
    vars.insert({arg.identifier, addr});
  }

  ni::ast::visitor::visit(*node.body);

  printer->label(current_function_name + "_epilogue");
  printer->ret(return_operand);
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::function_call &node) {
  for (int i = 0; i < node.params.size(); i++) {
    auto &param = *node.params[i];
    ni::ast::visitor::visit(param);
    auto arg =
        ir_operand(ir_operand_type::TEMPORARY, "ARG" + std::to_string(i));
    printer->mov(arg, return_operand);
  }

  return_operand = ir_operand(ir_operand_type::TEMPORARY,
                              "TMP" + std::to_string(current_temprary_id++));

  printer->call(return_operand, node.identifier);
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::function_return &node) {
  if (node.value.get() != nullptr) {
    ni::ast::visitor::visit(*node.value);
  } else {
    return_operand = ir_operand(ir_operand_type::CONSTANT, "0");
  }
  printer->jmp(current_function_name + "_epilogue");
}

void ni::codegen::ir::ir_codegen::generate_exit_call() {
  printer->exit(return_operand);
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::program &node) {
  generate_start_function();
  ni::ast::visitor::visit(node);
}

void ni::codegen::ir::ir_codegen::generate_start_function() {
  printer->label("start");
  return_operand = ir_operand(ir_operand_type::TEMPORARY,
                              "TMP" + std::to_string(current_temprary_id++));
  printer->call(return_operand, "main");
  generate_exit_call();
}

void ni::codegen::ir::ir_codegen::close_file() {
  if (output_file->is_open())
    output_file->close();
}

void ni::codegen::ir::ir_codegen::generate(const std::string &output) {
  output_file = std::make_shared<std::ofstream>();
  output_file->open(output, std::ios::trunc);
  printer = std::make_unique<ir_printer>(output_file);

  visit(program);
}
