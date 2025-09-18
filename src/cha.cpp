#include "cha/cha.h"
#include "ast.hpp"
#include "codegen.hpp"
#include "log.hpp"
#include "parser.hpp"
#include "validate.hpp"

namespace cha {

int compile(const std::string& file, CompileFormat format,
            const std::string& output_file) {
  AstNodeList ast;
  
  if (parse(file, ast) != 0) {
    log_error("Could not parse file " + file);
    return 1;
  }
  
  Validator validator;
  if (!validator.validate(ast)) {
    // Errors already logged by validator
    return 1;
  }

  // Generate code
  int ret = generate_code(ast, format, output_file);
  if (ret != 0) {
    log_error("Code generation failed");
    return ret;
  }
  
  return 0;
}

} // namespace cha