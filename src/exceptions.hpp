#pragma once

#include "ast.hpp"
#include <exception>
#include <optional>
#include <string>
#include <vector>

namespace cha {

// Base exception class for all Cha compiler errors
class ChaException : public std::exception {
public:
  explicit ChaException(const std::string &message) : message_(message) {}
  const char *what() const noexcept override { return message_.c_str(); }
  const std::string &message() const { return message_; }

protected:
  std::string message_;
};

// Exception class for validation errors
class ValidationException : public ChaException {
public:
  ValidationException(const AstLocation &location, const std::string &message)
      : ChaException(format_message(location, message)), location_(location) {}

  const AstLocation &location() const { return location_; }

private:
  AstLocation location_;

  static std::string format_message(const AstLocation &location,
                                    const std::string &message) {
    return location.file + ":" + std::to_string(location.line_begin) + ":" +
           std::to_string(location.column_begin) +
           ": validation error: " + message;
  }
};

// Exception class for parsing errors
class ParseException : public ChaException {
public:
  ParseException(const AstLocation &location, const std::string &message)
      : ChaException(format_message(location, message)), location_(location) {}

  const AstLocation &location() const { return location_; }

private:
  AstLocation location_;

  static std::string format_message(const AstLocation &location,
                                    const std::string &message) {
    return location.file + ":" + std::to_string(location.line_begin) + ":" +
           std::to_string(location.column_begin) + ": syntax error: " + message;
  }
};

// Exception class for code generation errors
class CodeGenerationException : public ChaException {
public:
  explicit CodeGenerationException(const std::string &message)
      : ChaException("code generation error: " + message) {}

  CodeGenerationException(const AstLocation &location,
                          const std::string &message)
      : ChaException(format_message(location, message)), location_(location) {}

  const AstLocation *location() const {
    return location_ ? &location_.value() : nullptr;
  }

private:
  std::optional<AstLocation> location_;

  static std::string format_message(const AstLocation &location,
                                    const std::string &message) {
    return location.file + ":" + std::to_string(location.line_begin) + ":" +
           std::to_string(location.column_begin) +
           ": code generation error: " + message;
  }
};

// Exception class for multiple validation errors
class MultipleValidationException : public ChaException {
public:
  explicit MultipleValidationException(
      const std::vector<ValidationException> &errors)
      : ChaException(format_multiple_errors(errors)), errors_(errors) {}

  const std::vector<ValidationException> &errors() const { return errors_; }

private:
  std::vector<ValidationException> errors_;

  static std::string
  format_multiple_errors(const std::vector<ValidationException> &errors) {
    if (errors.empty()) {
      return "validation failed";
    }
    if (errors.size() == 1) {
      return errors[0].message();
    }
    return "validation failed with " + std::to_string(errors.size()) +
           " errors";
  }
};

} // namespace cha
