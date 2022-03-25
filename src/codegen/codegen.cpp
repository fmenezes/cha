#include <memory>
#include <sstream>

#include "ast/ast.hh"
#include "codegen/codegen.hh"
#include "generated/parser.tab.hh"

ni::codegen::OS ni::codegen::Context::defaultOs() {
#if defined(__APPLE__) || defined(__MACH__)
  return ni::codegen::OS::MACOS;
#else
  return ni::codegen::OS::LINUX;
#endif
}

ni::codegen::ARCH ni::codegen::Context::defaultArch() {
  return ni::codegen::ARCH::x86_64;
}
