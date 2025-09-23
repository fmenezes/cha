#include "codegen.hpp"
#include "log.hpp"

namespace cha {

int CodeGenerator::generate(const AstNodeList& ast, CompileFormat format, const std::string& output_file) {
    has_error_ = false;
    
    // Visit all top-level nodes
    for (const auto& node : ast) {
        visit_node(*node);
        if (has_error_) {
            return 1;
        }
    }
    
    return 0;
}

void CodeGenerator::visit_node(const AstNode& node) {
    node.accept(*this);
}

// Visitor implementations - all return "not implemented" for now
void CodeGenerator::visit(const ConstantIntegerNode& node) {
    log_error("Code generation not implemented for ConstantIntegerNode");
    has_error_ = true;
}

void CodeGenerator::visit(const ConstantUnsignedIntegerNode& node) {
    log_error("Code generation not implemented for ConstantUnsignedIntegerNode");
    has_error_ = true;
}

void CodeGenerator::visit(const ConstantFloatNode& node) {
    log_error("Code generation not implemented for ConstantFloatNode");
    has_error_ = true;
}

void CodeGenerator::visit(const ConstantBoolNode& node) {
    log_error("Code generation not implemented for ConstantBoolNode");
    has_error_ = true;
}

void CodeGenerator::visit(const BinaryOpNode& node) {
    log_error("Code generation not implemented for BinaryOpNode");
    has_error_ = true;
}

void CodeGenerator::visit(const VariableDeclarationNode& node) {
    log_error("Code generation not implemented for VariableDeclarationNode");
    has_error_ = true;
}

void CodeGenerator::visit(const VariableAssignmentNode& node) {
    log_error("Code generation not implemented for VariableAssignmentNode");
    has_error_ = true;
}

void CodeGenerator::visit(const VariableLookupNode& node) {
    log_error("Code generation not implemented for VariableLookupNode");
    has_error_ = true;
}

void CodeGenerator::visit(const ArgumentNode& node) {
    log_error("Code generation not implemented for ArgumentNode");
    has_error_ = true;
}

void CodeGenerator::visit(const BlockNode& node) {
    log_error("Code generation not implemented for BlockNode");
    has_error_ = true;
}

void CodeGenerator::visit(const FunctionDeclarationNode& node) {
    log_error("Code generation not implemented for FunctionDeclarationNode");
    has_error_ = true;
}

void CodeGenerator::visit(const FunctionCallNode& node) {
    log_error("Code generation not implemented for FunctionCallNode");
    has_error_ = true;
}

void CodeGenerator::visit(const FunctionReturnNode& node) {
    log_error("Code generation not implemented for FunctionReturnNode");
    has_error_ = true;
}

void CodeGenerator::visit(const IfNode& node) {
    log_error("Code generation not implemented for IfNode");
    has_error_ = true;
}

void CodeGenerator::visit(const ConstantDeclarationNode& node) {
    log_error("Code generation not implemented for ConstantDeclarationNode");
    has_error_ = true;
}

int generate_code(const AstNodeList& ast, CompileFormat format, const std::string& output_file) {
    CodeGenerator generator;
    return generator.generate(ast, format, output_file);
}

} // namespace cha
