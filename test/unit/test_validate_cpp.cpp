#include <iostream>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "../src/validate.hpp"
#include "../src/ast.hpp"

using namespace cha;

// Test helper macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "ASSERTION FAILED: " << #condition << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << "ASSERTION FAILED: !(" << #condition << ") at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "ASSERTION FAILED: " << #expected << " == " << #actual \
                      << " (expected: " << (expected) << ", actual: " << (actual) \
                      << ") at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

// Helper function to create a location
AstLocation make_test_location() {
    return AstLocation("test.cha", 1, 1, 1, 10);
}

// Helper function to create primitive types
std::unique_ptr<AstType> make_int_type() {
    return std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::INT));
}

std::unique_ptr<AstType> make_uint_type() {
    return std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::UINT));
}

std::unique_ptr<AstType> make_bool_type() {
    return std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::BOOL));
}

std::unique_ptr<AstType> make_float_type() {
    return std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::FLOAT32));
}

// Test TypeUtils functions
bool test_type_utils() {
    std::cout << "Testing TypeUtils..." << std::endl;
    
    // Test is_numeric
    ASSERT_TRUE(TypeUtils::is_numeric(PrimitiveType::INT));
    ASSERT_TRUE(TypeUtils::is_numeric(PrimitiveType::UINT));
    ASSERT_TRUE(TypeUtils::is_numeric(PrimitiveType::FLOAT32));
    ASSERT_FALSE(TypeUtils::is_numeric(PrimitiveType::BOOL));
    
    // Test is_signed_int
    ASSERT_TRUE(TypeUtils::is_signed_int(PrimitiveType::INT));
    ASSERT_TRUE(TypeUtils::is_signed_int(PrimitiveType::INT32));
    ASSERT_FALSE(TypeUtils::is_signed_int(PrimitiveType::UINT));
    ASSERT_FALSE(TypeUtils::is_signed_int(PrimitiveType::BOOL));
    
    // Test is_unsigned_int
    ASSERT_TRUE(TypeUtils::is_unsigned_int(PrimitiveType::UINT));
    ASSERT_TRUE(TypeUtils::is_unsigned_int(PrimitiveType::UINT32));
    ASSERT_FALSE(TypeUtils::is_unsigned_int(PrimitiveType::INT));
    ASSERT_FALSE(TypeUtils::is_unsigned_int(PrimitiveType::BOOL));
    
    // Test is_float
    ASSERT_TRUE(TypeUtils::is_float(PrimitiveType::FLOAT32));
    ASSERT_TRUE(TypeUtils::is_float(PrimitiveType::FLOAT64));
    ASSERT_FALSE(TypeUtils::is_float(PrimitiveType::INT));
    ASSERT_FALSE(TypeUtils::is_float(PrimitiveType::BOOL));
    
    // Test convert_arithmetic_types
    ASSERT_EQ(PrimitiveType::INT32, TypeUtils::convert_arithmetic_types(PrimitiveType::INT, PrimitiveType::INT32));
    ASSERT_EQ(PrimitiveType::UINT32, TypeUtils::convert_arithmetic_types(PrimitiveType::UINT, PrimitiveType::UINT32));
    ASSERT_EQ(PrimitiveType::FLOAT64, TypeUtils::convert_arithmetic_types(PrimitiveType::FLOAT32, PrimitiveType::FLOAT64));
    ASSERT_EQ(PrimitiveType::UNDEF, TypeUtils::convert_arithmetic_types(PrimitiveType::INT, PrimitiveType::UINT));
    
    // Test assignment compatibility
    ASSERT_TRUE(TypeUtils::is_assignment_compatible(PrimitiveType::INT, PrimitiveType::INT));
    ASSERT_TRUE(TypeUtils::is_assignment_compatible(PrimitiveType::CONST_INT, PrimitiveType::INT));
    ASSERT_FALSE(TypeUtils::is_assignment_compatible(PrimitiveType::INT, PrimitiveType::UINT));
    ASSERT_FALSE(TypeUtils::is_assignment_compatible(PrimitiveType::INT, PrimitiveType::BOOL));
    
    // Test comparison compatibility
    ASSERT_TRUE(TypeUtils::is_numeric_comparison_compatible(PrimitiveType::INT, PrimitiveType::INT32));
    ASSERT_TRUE(TypeUtils::is_equality_comparison_compatible(PrimitiveType::BOOL, PrimitiveType::BOOL));
    ASSERT_FALSE(TypeUtils::is_numeric_comparison_compatible(PrimitiveType::INT, PrimitiveType::BOOL));
    
    // Test type_to_string
    ASSERT_EQ(std::string("int"), TypeUtils::type_to_string(PrimitiveType::INT));
    ASSERT_EQ(std::string("bool"), TypeUtils::type_to_string(PrimitiveType::BOOL));
    ASSERT_EQ(std::string("float32"), TypeUtils::type_to_string(PrimitiveType::FLOAT32));
    
    std::cout << "TypeUtils tests passed!" << std::endl;
    return true;
}

// Test SymbolTable functionality
bool test_symbol_table() {
    std::cout << "Testing SymbolTable..." << std::endl;
    
    auto table = std::make_shared<SymbolTable>();
    
    // Test inserting a symbol
    auto var_node = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "test_var", make_int_type());
    
    ASSERT_TRUE(table->insert("test_var", var_node->clone()));
    
    // Test duplicate insertion fails
    ASSERT_FALSE(table->insert("test_var", var_node->clone()));
    
    // Test lookup
    const SymbolEntry* entry = table->lookup("test_var");
    ASSERT_TRUE(entry != nullptr);
    
    // Test lookup non-existent symbol
    const SymbolEntry* missing = table->lookup("missing_var");
    ASSERT_TRUE(missing == nullptr);
    
    // Test child scope
    auto child_table = table->create_child_scope();
    
    // Child should be able to see parent symbols
    const SymbolEntry* parent_entry = child_table->lookup("test_var");
    ASSERT_TRUE(parent_entry != nullptr);
    
    // Child can shadow parent symbols
    auto child_var = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "test_var", make_bool_type());
    ASSERT_TRUE(child_table->insert("test_var", child_var->clone()));
    
    std::cout << "SymbolTable tests passed!" << std::endl;
    return true;
}

// Test basic validation scenarios
bool test_basic_validation() {
    std::cout << "Testing basic validation..." << std::endl;
    
    Validator validator;
    
    // Test empty AST
    AstNodeList empty_ast;
    ASSERT_TRUE(validator.validate(empty_ast));
    ASSERT_EQ(0, validator.errors().size());
    
    // Test simple function declaration
    AstNodeList simple_func;
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "test_func", make_int_type(), 
        AstNodeList{}, AstNodeList{});
    simple_func.push_back(std::move(func));
    
    ASSERT_TRUE(validator.validate(simple_func));
    ASSERT_EQ(0, validator.errors().size());
    
    std::cout << "Basic validation tests passed!" << std::endl;
    return true;
}

// Test variable validation
bool test_variable_validation() {
    std::cout << "Testing variable validation..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    
    // Test function with variable declaration
    AstNodeList func_body;
    
    // Add variable declaration: int x = 42;
    auto const_val = std::make_unique<ConstantIntegerNode>(make_test_location(), "42");
    const_val->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    auto var_decl = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "x", make_int_type(), std::move(const_val));
    func_body.push_back(std::move(var_decl));
    
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "test_func", make_int_type(), 
        AstNodeList{}, std::move(func_body));
    ast.push_back(std::move(func));
    
    ASSERT_TRUE(validator.validate(ast));
    ASSERT_EQ(0, validator.errors().size());
    
    std::cout << "Variable validation tests passed!" << std::endl;
    return true;
}

// Test duplicate symbol detection
bool test_duplicate_symbols() {
    std::cout << "Testing duplicate symbol detection..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    
    // Create two functions with the same name
    auto func1 = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "duplicate_func", make_int_type(), 
        AstNodeList{}, AstNodeList{});
    
    auto func2 = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "duplicate_func", make_int_type(), 
        AstNodeList{}, AstNodeList{});
    
    ast.push_back(std::move(func1));
    ast.push_back(std::move(func2));
    
    ASSERT_FALSE(validator.validate(ast));
    ASSERT_EQ(1, validator.errors().size());
    ASSERT_TRUE(validator.errors()[0].message().find("already defined") != std::string::npos);
    
    std::cout << "Duplicate symbol detection tests passed!" << std::endl;
    return true;
}

// Test binary operation validation
bool test_binary_operations() {
    std::cout << "Testing binary operations..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    AstNodeList func_body;
    
    // Create: int result = 5 + 10;
    auto left = std::make_unique<ConstantIntegerNode>(make_test_location(), "5");
    left->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    auto right = std::make_unique<ConstantIntegerNode>(make_test_location(), "10");
    right->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    auto bin_op = std::make_unique<BinaryOpNode>(
        make_test_location(), Operator::ADD, std::move(left), std::move(right));
    
    auto var_decl = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "result", make_int_type(), std::move(bin_op));
    func_body.push_back(std::move(var_decl));
    
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "test_func", make_int_type(), 
        AstNodeList{}, std::move(func_body));
    ast.push_back(std::move(func));
    
    ASSERT_TRUE(validator.validate(ast));
    ASSERT_EQ(0, validator.errors().size());
    
    std::cout << "Binary operation tests passed!" << std::endl;
    return true;
}

// Test type mismatch detection
bool test_type_mismatches() {
    std::cout << "Testing type mismatch detection..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    AstNodeList func_body;
    
    // Create: int x; x = true; (should fail)
    auto var_decl = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "x", make_int_type());
    func_body.push_back(std::move(var_decl));
    
    auto bool_val = std::make_unique<ConstantBoolNode>(make_test_location(), true);
    bool_val->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::BOOL)));
    
    auto assignment = std::make_unique<VariableAssignmentNode>(
        make_test_location(), "x", std::move(bool_val));
    func_body.push_back(std::move(assignment));
    
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "test_func", make_int_type(), 
        AstNodeList{}, std::move(func_body));
    ast.push_back(std::move(func));
    
    ASSERT_FALSE(validator.validate(ast));
    ASSERT_TRUE(validator.errors().size() > 0);
    ASSERT_TRUE(validator.errors()[0].message().find("type mismatch") != std::string::npos);
    
    std::cout << "Type mismatch detection tests passed!" << std::endl;
    return true;
}

// Test function call validation
bool test_function_calls() {
    std::cout << "Testing function call validation..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    
    // Create function: int add(int a, int b) { return a + b; }
    AstNodeList args;
    auto arg1 = std::make_unique<ArgumentNode>(make_test_location(), "a", make_int_type());
    auto arg2 = std::make_unique<ArgumentNode>(make_test_location(), "b", make_int_type());
    args.push_back(std::move(arg1));
    args.push_back(std::move(arg2));
    
    AstNodeList body;
    auto var_a = std::make_unique<VariableLookupNode>(make_test_location(), "a");
    auto var_b = std::make_unique<VariableLookupNode>(make_test_location(), "b");
    auto add_op = std::make_unique<BinaryOpNode>(
        make_test_location(), Operator::ADD, std::move(var_a), std::move(var_b));
    auto return_stmt = std::make_unique<FunctionReturnNode>(make_test_location(), std::move(add_op));
    body.push_back(std::move(return_stmt));
    
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "add", make_int_type(), std::move(args), std::move(body));
    ast.push_back(std::move(func));
    
    // Create main function that calls add
    AstNodeList main_body;
    AstNodeList call_args;
    auto const5 = std::make_unique<ConstantIntegerNode>(make_test_location(), "5");
    const5->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    auto const10 = std::make_unique<ConstantIntegerNode>(make_test_location(), "10");
    const10->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    call_args.push_back(std::move(const5));
    call_args.push_back(std::move(const10));
    
    auto func_call = std::make_unique<FunctionCallNode>(
        make_test_location(), "add", std::move(call_args));
    auto result_var = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "result", make_int_type(), std::move(func_call));
    main_body.push_back(std::move(result_var));
    
    auto main_func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "main", make_int_type(), AstNodeList{}, std::move(main_body));
    ast.push_back(std::move(main_func));
    
    ASSERT_TRUE(validator.validate(ast));
    ASSERT_EQ(0, validator.errors().size());
    
    std::cout << "Function call validation tests passed!" << std::endl;
    return true;
}

// Test if statement validation
bool test_if_statements() {
    std::cout << "Testing if statement validation..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    AstNodeList func_body;
    
    // Create: if (true) { int x = 5; }
    auto condition = std::make_unique<ConstantBoolNode>(make_test_location(), true);
    condition->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::BOOL)));
    
    AstNodeList then_block;
    auto const_val = std::make_unique<ConstantIntegerNode>(make_test_location(), "5");
    const_val->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    auto var_decl = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "x", make_int_type(), std::move(const_val));
    then_block.push_back(std::move(var_decl));
    
    auto if_stmt = std::make_unique<IfNode>(
        make_test_location(), std::move(condition), std::move(then_block));
    func_body.push_back(std::move(if_stmt));
    
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "test_func", make_int_type(), 
        AstNodeList{}, std::move(func_body));
    ast.push_back(std::move(func));
    
    ASSERT_TRUE(validator.validate(ast));
    ASSERT_EQ(0, validator.errors().size());
    
    std::cout << "If statement validation tests passed!" << std::endl;
    return true;
}

// Test invalid if condition (non-boolean)
bool test_invalid_if_condition() {
    std::cout << "Testing invalid if condition..." << std::endl;
    
    Validator validator;
    AstNodeList ast;
    AstNodeList func_body;
    
    // Create: if (42) { int x = 5; } (should fail)
    auto condition = std::make_unique<ConstantIntegerNode>(make_test_location(), "42");
    condition->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    AstNodeList then_block;
    auto const_val = std::make_unique<ConstantIntegerNode>(make_test_location(), "5");
    const_val->set_result_type(std::make_unique<AstType>(make_test_location(), AstType::Primitive(PrimitiveType::CONST_INT)));
    
    auto var_decl = std::make_unique<VariableDeclarationNode>(
        make_test_location(), "x", make_int_type(), std::move(const_val));
    then_block.push_back(std::move(var_decl));
    
    auto if_stmt = std::make_unique<IfNode>(
        make_test_location(), std::move(condition), std::move(then_block));
    func_body.push_back(std::move(if_stmt));
    
    auto func = std::make_unique<FunctionDeclarationNode>(
        make_test_location(), "test_func", make_int_type(), 
        AstNodeList{}, std::move(func_body));
    ast.push_back(std::move(func));
    
    ASSERT_FALSE(validator.validate(ast));
    ASSERT_TRUE(validator.errors().size() > 0);
    ASSERT_TRUE(validator.errors()[0].message().find("condition should return bool") != std::string::npos);
    
    std::cout << "Invalid if condition tests passed!" << std::endl;
    return true;
}

// Main test runner
int main() {
    std::cout << "Running C++ validation tests..." << std::endl << std::endl;
    
    int passed = 0;
    int total = 0;
    
    struct TestCase {
        const char* name;
        bool (*func)();
    };
    
    TestCase tests[] = {
        {"TypeUtils", test_type_utils},
        {"SymbolTable", test_symbol_table},
        {"Basic Validation", test_basic_validation},
        {"Variable Validation", test_variable_validation},
        {"Duplicate Symbols", test_duplicate_symbols},
        {"Binary Operations", test_binary_operations},
        {"Type Mismatches", test_type_mismatches},
        {"Function Calls", test_function_calls},
        {"If Statements", test_if_statements},
        {"Invalid If Condition", test_invalid_if_condition}
    };
    
    for (const auto& test : tests) {
        total++;
        std::cout << "Running " << test.name << " test..." << std::endl;
        
        try {
            if (test.func()) {
                passed++;
                std::cout << "✓ " << test.name << " test passed" << std::endl;
            } else {
                std::cout << "✗ " << test.name << " test failed" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "✗ " << test.name << " test failed with exception: " << e.what() << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "Test Results: " << passed << "/" << total << " tests passed" << std::endl;
    
    if (passed == total) {
        std::cout << "All tests passed! ✓" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed! ✗" << std::endl;
        return 1;
    }
}
