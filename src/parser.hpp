#pragma once

#include "ast.hpp"

namespace cha {

// Main parser function implemented in parser.y - throws ParseException on error
AstNodeList parse(const char *file);

// Convenience overload for std::string
inline AstNodeList parse(const std::string &file) {
  return parse(file.c_str());
}

} // namespace cha
