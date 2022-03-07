#include <memory>
#include <sstream>

#include "ast.hh"
#include "codegen.hh"
#include "parser.tab.hh"

int ni::ast::NProgram::parse() {
  yy::parser p(*this);
  return p.parse();
}

ni::codegen::OS ni::codegen::Codegen::defaultOs() {
#if defined(__APPLE__) || defined(__MACH__)
  return ni::codegen::OS::MACOS;
#else
  return ni::codegen::OS::LINUX;
#endif
}

ni::codegen::ARCH ni::codegen::Codegen::defaultArch() {
  return ni::codegen::ARCH::x86_64;
}
