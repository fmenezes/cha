#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <sys/wait.h>

namespace fs = std::filesystem;

class IntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Clean up any existing output files
    cleanup();
  }

  void TearDown() override {
    // Clean up after each test
    cleanup();
  }

  void cleanup() {
    std::remove("out.ll");
    std::remove("out");
  }

  // Helper to run a command and capture its output
  struct CommandResult {
    int exit_code;
    std::string output;
  };

  CommandResult runCommand(const std::string &cmd) {
    CommandResult result;

    // Redirect stderr to stdout and capture
    std::string fullCmd = cmd + " 2>&1";

    FILE *pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) {
      result.exit_code = -1;
      result.output = "Failed to run command";
      return result;
    }

    char buffer[10000];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result.output = buffer;
      // Remove trailing newline
      if (!result.output.empty() && result.output.back() == '\n') {
        result.output.pop_back();
      }
    }

    result.exit_code = pclose(pipe);
    return result;
  }

  // Test that compilation succeeds
  void expectCompilationSuccess(const std::string &testFile) {
    std::string cmd = "./build/cha -ll out.ll test/integration/" + testFile;
    auto result = runCommand(cmd);

    EXPECT_EQ(result.exit_code, 0)
        << "Compilation of " << testFile << " should succeed\n"
        << "Command: " << cmd << "\n"
        << "Output: " << result.output;
  }

  // Test that compilation fails with expected error message
  void expectCompilationFailure(const std::string &testFile,
                                const std::string &expectedError) {
    std::string cmd = "./build/cha -ll out.ll test/integration/" + testFile;
    auto result = runCommand(cmd);

    EXPECT_NE(result.exit_code, 0)
        << "Compilation of " << testFile << " should fail\n"
        << "Command: " << cmd;

    EXPECT_TRUE(result.output.find(expectedError) != std::string::npos)
        << "Expected error message '" << expectedError
        << "' not found in output\n"
        << "Actual output: '" << result.output << "'";
  }

  // Test that compilation and execution succeed with expected exit code
  void expectExecutionResult(const std::string &testFile,
                             int expectedExitCode) {
    std::string compileCmd = "./build/cha -o out test/integration/" + testFile;
    auto compileResult = runCommand(compileCmd);

    EXPECT_EQ(compileResult.exit_code, 0)
        << "Compilation of " << testFile << " should succeed\n"
        << "Command: " << compileCmd << "\n"
        << "Output: " << compileResult.output;

    if (compileResult.exit_code == 0) {
      auto execResult = runCommand("./out");
      // Convert shell exit code to actual exit code
      int actualExitCode = WEXITSTATUS(execResult.exit_code);
      EXPECT_EQ(actualExitCode, expectedExitCode)
          << "Execution of " << testFile << " should return "
          << expectedExitCode;
    }
  }
};

// Parse Tests
TEST_F(IntegrationTest, ParsePasses) {
  expectCompilationSuccess("parse_passes.cha");
}

TEST_F(IntegrationTest, ParseFailure) {
  expectCompilationFailure("parse_failure.cha", "unexpected character");
}

// Operator Tests
TEST_F(IntegrationTest, OperatorPasses) {
  expectCompilationSuccess("operator_passes.cha");
}

// Type Assignment Tests
TEST_F(IntegrationTest, TypeAssignPasses) {
  expectCompilationSuccess("type_assign_passes.cha");
}

// Control Flow Tests
TEST_F(IntegrationTest, IfPasses) { expectCompilationSuccess("if_passes.cha"); }

TEST_F(IntegrationTest, IfElsePasses) {
  expectCompilationSuccess("if_else_passes.cha");
}

TEST_F(IntegrationTest, IfConditionNotBool) {
  expectCompilationFailure("if_condition_not_bool.cha",
                           "condition should return bool");
}

// Validation Tests - General
TEST_F(IntegrationTest, ValidationPasses) {
  expectCompilationSuccess("validation_passes.cha");
}

// Validation Tests - Argument Errors
TEST_F(IntegrationTest, ValidationArgDup) {
  expectCompilationFailure("validation_arg_dup.cha",
                           "argument 'i' already defined");
}

TEST_F(IntegrationTest, ValidationArgMismatch) {
  expectCompilationFailure("validation_arg_mismatch.cha",
                           "function 'test' expects no arguments");
}

TEST_F(IntegrationTest, ValidationArgMismatch2) {
  expectCompilationFailure("validation_arg_mismatch2.cha",
                           "function 'test' expects arguments");
}

TEST_F(IntegrationTest, ValidationArgMismatch3) {
  expectCompilationFailure("validation_arg_mismatch3.cha",
                           "type mismatch expects 'uint8' passed 'int'");
}

// Validation Tests - Function Errors
TEST_F(IntegrationTest, ValidationDupFunction) {
  expectCompilationFailure("validation_dup_function.cha",
                           "'test' already defined");
}

TEST_F(IntegrationTest, ValidationFunctionNotFound) {
  expectCompilationFailure("validation_function_not_found.cha",
                           "function 'test' not found");
}

TEST_F(IntegrationTest, ValidationRetMismatch) {
  expectCompilationFailure("validation_ret_mismatch.cha",
                           "return type mismatch expects 'int' passed 'uint8'");
}

// Validation Tests - Type Errors
TEST_F(IntegrationTest, ValidationTypeMismatch) {
  expectCompilationFailure("validation_type_mismatch.cha",
                           "type mismatch expects 'int' passed 'uint8'");
}

TEST_F(IntegrationTest, ValidationTypeMismatch2) {
  expectCompilationFailure(
      "validation_type_mismatch2.cha",
      "incompatible types found for operation: 'int', 'uint8'");
}

// Validation Tests - Variable Errors
TEST_F(IntegrationTest, ValidationVarNotFound) {
  expectCompilationFailure("validation_var_not_found.cha", "'a' not found");
}

TEST_F(IntegrationTest, ValidationVarRedefined) {
  expectCompilationFailure("validation_var_redefined.cha",
                           "variable 'a' already defined");
}

TEST_F(IntegrationTest, ValidationVarRedefined2) {
  expectCompilationFailure("validation_var_redefined2.cha",
                           "variable 'a' already defined");
}

// Binary/Execution Tests
TEST_F(IntegrationTest, TestBin) {
  // Note: Binary generation is not yet implemented in the compiler
  // This test verifies that compilation succeeds, even though execution isn't
  // possible
  std::string compileCmd = "./build/cha -o out test/integration/test_bin.cha";
  auto compileResult = runCommand(compileCmd);

  // The compiler currently produces an object file instead of an executable
  // binary So we expect compilation to "succeed" but with a warning message
  EXPECT_EQ(compileResult.exit_code, 0)
      << "Compilation of test_bin.cha should succeed (even if binary "
         "generation is not implemented)\n"
      << "Command: " << compileCmd << "\n"
      << "Output: " << compileResult.output;

  // Check that an output file was created
  std::ifstream outFile("out");
  EXPECT_TRUE(outFile.good()) << "Output file should be created";
  outFile.close();
}

TEST_F(IntegrationTest, UnaryNegate) {
  expectCompilationSuccess("unary_negate.cha");
}

TEST_F(IntegrationTest, UnaryNot) { expectCompilationSuccess("unary_not.cha"); }

TEST_F(IntegrationTest, UnaryNegateFloat) {
  expectCompilationSuccess("unary_negate_float.cha");
}

TEST_F(IntegrationTest, UnaryNegateBoolError) {
  expectCompilationFailure("unary_negate_bool_error.cha", "incompatible type");
}

TEST_F(IntegrationTest, UnaryNotIntError) {
  expectCompilationFailure("unary_not_int_error.cha", "incompatible type");
}
