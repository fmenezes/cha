#pragma once

#include "ast.hpp"
#include "cha/cha.h"

namespace cha {

// Code generation class that implements visitor pattern
class CodeGenerator : public AstVisitor {
public:
    CodeGenerator() = default;
    ~CodeGenerator() = default;
    
    // Generate code from AST
    int generate(const AstNodeList& ast, CompileFormat format, const std::string& output_file);
    
    // Visitor pattern implementation - all return "not implemented"
    void visit(const ConstantIntegerNode& node) override;
    void visit(const ConstantUnsignedIntegerNode& node) override;
    void visit(const ConstantFloatNode& node) override;
    void visit(const ConstantBoolNode& node) override;
    void visit(const BinaryOpNode& node) override;
    void visit(const VariableDeclarationNode& node) override;
    void visit(const VariableAssignmentNode& node) override;
    void visit(const VariableLookupNode& node) override;
    void visit(const ArgumentNode& node) override;
    void visit(const BlockNode& node) override;
    void visit(const FunctionDeclarationNode& node) override;
    void visit(const FunctionCallNode& node) override;
    void visit(const FunctionReturnNode& node) override;
    void visit(const IfNode& node) override;
    void visit(const ConstantDeclarationNode& node) override;
    
private:
    // Error tracking
    bool has_error_ = false;
    
    // Helper method to visit any AST node
    void visit_node(const AstNode& node);
};

// Convenience function
int generate_code(const AstNodeList& ast, CompileFormat format, const std::string& output_file);

} // namespace cha
