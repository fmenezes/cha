#pragma once

#include "ast.hpp"

namespace cha {

// Main parser function implemented in parser.y - throws ParseException on error
void parse(const char *file, AstNodeList &out);

// Convenience overload for std::string
inline void parse(const std::string &file, AstNodeList &out) {
  parse(file.c_str(), out);
}

} // namespace cha
