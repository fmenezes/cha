#include <exception>
#include <fstream>
#include <iostream>

#include "ast/ast.hh"
#include "codegen/codegen.hh"
#include "codegen/ir/ir_codegen.hh"
#include "codegen/memory_calculator.hh"

void ni::codegen::ir::ir_codegen::visit(const ni::ast::constant_integer &node) {
  this->return_operand = ni::codegen::ir::ir_operand(
      ni::codegen::ir::ir_operand_type::CONSTANT, node.value);
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::binary_operation &node) {
  ni::ast::visitor::visit(*node.left);
  ni::codegen::ir::ir_operand laddr = this->return_operand;

  ni::ast::visitor::visit(*node.right);
  ni::codegen::ir::ir_operand raddr = this->return_operand;

  if (node.op.compare("+") == 0) {
    this->printer->add(raddr, laddr);
  } else if (node.op.compare("-") == 0) {
    this->printer->sub(raddr, laddr);
  } else if (node.op.compare("*") == 0) {
    this->printer->mul(raddr, laddr);
  } else {
    throw std::runtime_error("invalid operation " + node.op);
  }
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::variable_lookup &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  this->return_operand = s->second;
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::variable_assignment &node) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    throw std::runtime_error(node.identifier + " not found.");
  }
  ni::ast::visitor::visit(*node.value);
  this->printer->mov(s->second, this->return_operand);
  this->return_operand = s->second;
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::variable_declaration &node) {
  auto s = this->vars.find(node.identifier);
  if (s != this->vars.end()) {
    throw std::runtime_error(node.identifier + " already exists.");
  }

  this->return_operand = ir_operand(ir_operand_type::MEMORY,
                                    std::to_string(this->current_memory_id++));
  this->vars.insert({node.identifier, this->return_operand});
}

void ni::codegen::ir::ir_codegen::visit(
    const ni::ast::function_declaration &node) {
  this->current_function_name = node.identifier;

  this->printer->global(this->current_function_name);
  this->printer->label(this->current_function_name);

  this->current_memory_id = 0;
  this->current_temprary_id = 0;
  this->vars.clear();

  for (int a = 0; a < node.args.size(); a++) {
    auto &arg = *node.args[a];

    auto addr = ir_operand(ir_operand_type::MEMORY, "ARG" + std::to_string(a));
    this->vars.insert({arg.identifier, addr});
  }

  ni::ast::visitor::visit(*node.body);

  this->printer->label(this->current_function_name + "_epilogue");
  this->printer->ret();
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::function_call &node) {
  for (int i = 0; i < node.params.size(); i++) {
    auto &param = *node.params[i];
    ni::ast::visitor::visit(param);
    auto arg =
        ir_operand(ir_operand_type::TEMPORARY, "ARG" + std::to_string(i));
    this->printer->mov(arg, this->return_operand);
  }

  this->printer->call(node.identifier);

  this->return_operand =
      ir_operand(ir_operand_type::TEMPORARY,
                 "TMP" + std::to_string(current_temprary_id++));
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::function_return &node) {
  if (node.value.get() != nullptr) {
    ni::ast::visitor::visit(*node.value);
    auto addr = this->return_operand;
  }
  this->printer->jmp(this->current_function_name + "_epilogue");
  this->return_operand =
      ir_operand(ir_operand_type::TEMPORARY,
                 "TMP" + std::to_string(current_temprary_id++));
}

void ni::codegen::ir::ir_codegen::generate_exit_call() {
  this->printer->exit(return_operand);
}

void ni::codegen::ir::ir_codegen::visit(const ni::ast::program &node) {
  this->printer->text_header();
  this->generate_start_function();
  ni::ast::visitor::visit(node);
}

void ni::codegen::ir::ir_codegen::generate_start_function() {
  this->printer->label("start");
  this->printer->call("main");
  this->generate_exit_call();
}

void ni::codegen::ir::ir_codegen::close_file() {
  if (this->output_file->is_open())
    this->output_file->close();
}

void ni::codegen::ir::ir_codegen::generate(const std::string &output) {
  this->output_file = std::make_shared<std::ofstream>();
  this->output_file->open(output, std::ios::trunc);
  this->printer = std::make_unique<ir_printer>(output_file);

  this->visit(this->program);
}
