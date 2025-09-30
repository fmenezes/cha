#include "ast.hpp"
#include "parser.hpp"
#include <cassert>
#include <iostream>

using namespace cha;

void test_parse_simple_function() {
  std::cout << "Testing simple function parsing...\n";

  AstNodeList ast;
  int result = cha::parse("examples/test.cha", ast);

  assert(result == 0);
  assert(!ast.empty());

  std::cout << "Parsed " << ast.size() << " top-level nodes\n";

  // Check that we parsed at least one function
  bool found_main = false;
  bool found_any_function = false;

  for (const auto &node : ast) {
    if (auto func = dynamic_cast<const FunctionDeclarationNode *>(node.get())) {
      found_any_function = true;
      std::cout << "✓ Found function: " << func->identifier() << "\n";
      if (func->identifier() == "main") {
        found_main = true;
      }
    } else {
      std::cout << "? Found non-function node\n";
    }
  }

  assert(found_any_function);
  assert(found_main);
  std::cout << "✓ Simple function parsing test passed\n";
}

void test_parse_constants() {
  std::cout << "Testing constant parsing...\n";

  AstNodeList ast;
  int result = cha::parse("examples/test.cha", ast);

  assert(result == 0);
  assert(!ast.empty());

  // Look for constant declarations or constant values in the AST
  bool found_constant = false;
  for (const auto &node : ast) {
    if (auto func = dynamic_cast<const FunctionDeclarationNode *>(node.get())) {
      // Check function body for constants
      for (const auto &stmt : func->body()) {
        if (auto ret_stmt =
                dynamic_cast<const FunctionReturnNode *>(stmt.get())) {
          if (ret_stmt->value()) {
            if (auto const_int = dynamic_cast<const ConstantIntegerNode *>(
                    ret_stmt->value())) {
              found_constant = true;
              std::cout << "✓ Found integer constant: " << const_int->value()
                        << "\n";
            }
          }
        }
      }
    }
  }

  if (found_constant) {
    std::cout << "✓ Constant parsing test passed\n";
  } else {
    std::cout << "→ No constants found in this test file (this is okay)\n";
  }
}

void test_ast_structure() {
  std::cout << "Testing AST structure integrity...\n";

  AstNodeList ast;
  int result = cha::parse("examples/test.cha", ast);

  assert(result == 0);
  assert(!ast.empty());

  // Verify each node has proper location information
  for (const auto &node : ast) {
    const auto &loc = node->location();
    assert(!loc.file.empty());
    assert(loc.line_begin > 0);
    assert(loc.column_begin >= 0);
    assert(loc.line_end >= loc.line_begin);

    if (loc.line_end == loc.line_begin) {
      assert(loc.column_end >= loc.column_begin);
    }
  }

  std::cout << "✓ AST structure integrity test passed\n";
}

int main() {
  std::cout << "Running C++ Parser Tests...\n\n";

  try {
    test_parse_simple_function();
    test_parse_constants();
    test_ast_structure();

    std::cout << "\n✅ All C++ parser tests passed!\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "\n❌ Test failed with exception: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "\n❌ Test failed with unknown exception\n";
    return 1;
  }
}
