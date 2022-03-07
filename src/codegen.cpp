#include <memory>
#include <sstream>

#include "ast.hh"
#include "codegen.hh"
#include "parser.tab.hh"

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
