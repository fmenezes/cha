#include "ast.hpp"
#include <iostream>
#include <cassert>

using namespace cha;

void test_constant_nodes() {
    std::cout << "Testing constant nodes...\n";
    
    AstLocation loc("test.cha", 1, 1, 1, 10);
    
    // Test integer constant
    auto int_node = std::make_unique<ConstantIntegerNode>(loc, "42");
    auto int_const = dynamic_cast<const ConstantIntegerNode*>(int_node.get());
    assert(int_const != nullptr);
    assert(int_const->value() == "42");
    assert(int_const->result_type() != nullptr);
    assert(int_const->result_type()->is_primitive());
    assert(int_const->result_type()->as_primitive().type == PrimitiveType::CONST_INT);
    
    // Test boolean constant
    auto bool_node = std::make_unique<ConstantBoolNode>(loc, true);
    auto bool_const = dynamic_cast<const ConstantBoolNode*>(bool_node.get());
    assert(bool_const != nullptr);
    assert(bool_const->value() == true);
    
    // Test float constant
    auto float_node = std::make_unique<ConstantFloatNode>(loc, 3.14);
    auto float_const = dynamic_cast<const ConstantFloatNode*>(float_node.get());
    assert(float_const != nullptr);
    assert(float_const->value() == 3.14);
    
    std::cout << "✓ Constant nodes test passed\n";
}

void test_binary_op() {
    std::cout << "Testing binary operations...\n";
    
    AstLocation loc("test.cha", 2, 1, 2, 10);
    
    auto left = std::make_unique<ConstantIntegerNode>(loc, "10");
    auto right = std::make_unique<ConstantIntegerNode>(loc, "20");
    auto bin_op = std::make_unique<BinaryOpNode>(loc, Operator::ADD, std::move(left), std::move(right));
    
    auto bin_node = dynamic_cast<const BinaryOpNode*>(bin_op.get());
    assert(bin_node != nullptr);
    assert(bin_node->op() == Operator::ADD);
    
    auto left_const = dynamic_cast<const ConstantIntegerNode*>(&bin_node->left());
    auto right_const = dynamic_cast<const ConstantIntegerNode*>(&bin_node->right());
    assert(left_const != nullptr && left_const->value() == "10");
    assert(right_const != nullptr && right_const->value() == "20");
    
    std::cout << "✓ Binary operations test passed\n";
}

void test_variable_declaration() {
    std::cout << "Testing variable declarations...\n";
    
    AstLocation loc("test.cha", 3, 1, 3, 20);
    
    auto type = std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT});
    auto value = std::make_unique<ConstantIntegerNode>(loc, "100");
    auto var_decl = std::make_unique<VariableDeclarationNode>(loc, "myVar", std::move(type), std::move(value));
    
    auto var_node = dynamic_cast<const VariableDeclarationNode*>(var_decl.get());
    assert(var_node != nullptr);
    assert(var_node->identifier() == "myVar");
    assert(var_node->type().is_primitive());
    assert(var_node->type().as_primitive().type == PrimitiveType::INT);
    assert(var_node->value() != nullptr);
    
    auto value_const = dynamic_cast<const ConstantIntegerNode*>(var_node->value());
    assert(value_const != nullptr && value_const->value() == "100");
    
    std::cout << "✓ Variable declarations test passed\n";
}

void test_function_declaration() {
    std::cout << "Testing function declarations...\n";
    
    AstLocation loc("test.cha", 4, 1, 8, 1);
    
    // Create arguments
    AstNodeList args;
    args.push_back(std::make_unique<ArgumentNode>(loc, "x", 
        std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT})));
    args.push_back(std::make_unique<ArgumentNode>(loc, "y", 
        std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT})));
    
    // Create function body
    AstNodeList body;
    auto return_expr = std::make_unique<BinaryOpNode>(loc, Operator::ADD,
        std::make_unique<VariableLookupNode>(loc, "x"),
        std::make_unique<VariableLookupNode>(loc, "y")
    );
    body.push_back(std::make_unique<FunctionReturnNode>(loc, std::move(return_expr)));
    
    auto func_decl = std::make_unique<FunctionDeclarationNode>(loc, "add", 
        std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT}), 
        std::move(args), std::move(body));
    
    auto func_node = dynamic_cast<const FunctionDeclarationNode*>(func_decl.get());
    assert(func_node != nullptr);
    assert(func_node->identifier() == "add");
    assert(func_node->return_type().is_primitive());
    assert(func_node->return_type().as_primitive().type == PrimitiveType::INT);
    assert(func_node->arguments().size() == 2);
    assert(func_node->body().size() == 1);
    
    std::cout << "✓ Function declarations test passed\n";
}

void test_cloning() {
    std::cout << "Testing node cloning...\n";
    
    AstLocation loc("test.cha", 5, 1, 5, 10);
    
    auto original = std::make_unique<ConstantIntegerNode>(loc, "123");
    auto cloned = original->clone();
    
    auto orig_const = dynamic_cast<const ConstantIntegerNode*>(original.get());
    auto clone_const = dynamic_cast<const ConstantIntegerNode*>(cloned.get());
    
    assert(orig_const != nullptr);
    assert(clone_const != nullptr);
    assert(orig_const->value() == clone_const->value());
    assert(orig_const != clone_const); // Different objects
    
    std::cout << "✓ Node cloning test passed\n";
}

void test_types() {
    std::cout << "Testing type system...\n";
    
    AstLocation loc("test.cha", 6, 1, 6, 10);
    
    // Test primitive type
    auto int_type = std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::INT});
    assert(int_type->is_primitive());
    assert(!int_type->is_array());
    assert(!int_type->is_identifier());
    assert(int_type->as_primitive().type == PrimitiveType::INT);
    
    // Test array type
    auto elem_type = std::make_unique<AstType>(loc, AstType::Primitive{PrimitiveType::FLOAT32});
    auto array_type = std::make_unique<AstType>(loc, AstType::Array{std::move(elem_type), 10});
    assert(!array_type->is_primitive());
    assert(array_type->is_array());
    assert(!array_type->is_identifier());
    assert(array_type->as_array().size == 10);
    assert(array_type->as_array().element_type->is_primitive());
    assert(array_type->as_array().element_type->as_primitive().type == PrimitiveType::FLOAT32);
    
    // Test identifier type
    auto id_type = std::make_unique<AstType>(loc, AstType::Identifier{"MyCustomType"});
    assert(!id_type->is_primitive());
    assert(!id_type->is_array());
    assert(id_type->is_identifier());
    assert(id_type->as_identifier().name == "MyCustomType");
    
    std::cout << "✓ Type system test passed\n";
}

int main() {
    std::cout << "Running C++ AST tests...\n\n";
    
    try {
        test_constant_nodes();
        test_binary_op();
        test_variable_declaration();
        test_function_declaration();
        test_cloning();
        test_types();
        
        std::cout << "\n✅ All tests passed! C++ AST implementation is working correctly.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception\n";
        return 1;
    }
}
