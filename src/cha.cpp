#include "cha/cha.hpp"
#include "ast.hpp"
#include "codegen.hpp"
#include "exceptions.hpp"
#include "log.hpp"
#include "parser.hpp"
#include "validate.hpp"

namespace cha {

int compile(const std::string &file, CompileFormat format,
            const std::string &output_file) {
  AstNodeList ast;

  try {
    parse(file, ast);
  } catch (const ParseException &e) {
    log_error(e.message());
    return 1;
  }

  try {
    Validator validator;
    validator.validate(ast);
  } catch (const ValidationException &e) {
    log_error(e.message());
    return 1;
  } catch (const MultipleValidationException &e) {
    for (const auto &error : e.errors()) {
      log_error(error.message());
    }
    return 1;
  }

  // Generate code
  try {
    generate_code(ast, format, output_file);
  } catch (const CodeGenerationException &e) {
    log_error("Code generation failed: " + e.message());
    return 1;
  }

  return 0;
}

} // namespace cha