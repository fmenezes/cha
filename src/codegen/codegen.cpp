#include <memory>
#include <sstream>

#include "ast/ast.hh"
#include "codegen/codegen.hh"
#include "generated/parser.tab.hh"

ni::codegen::os ni::codegen::context::default_os() {
#if defined(__APPLE__) || defined(__MACH__)
  return ni::codegen::os::MACOS;
#else
  return ni::codegen::OS::LINUX;
#endif
}

ni::codegen::arch ni::codegen::context::default_arch() {
  return ni::codegen::arch::x86_64;
}
