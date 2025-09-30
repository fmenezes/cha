#pragma once

#include "ast.hpp"

namespace cha {

// Main parser function implemented in parser.y
int parse(const char *file, AstNodeList &out);

// Convenience overload for std::string
inline int parse(const std::string &file, AstNodeList &out) {
  return parse(file.c_str(), out);
}

} // namespace cha
