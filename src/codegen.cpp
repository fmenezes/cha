#include <memory>
#include <sstream>

#include "nodes.hh"
#include "parser.tab.hh"

int ni::NProgram::parse() {
  yy::parser p(*this);
  return p.parse();
}

ni::OS ni::Codegen::defaultOs() {
#if defined(__APPLE__) || defined(__MACH__)
  return ni::OS::MACOS;
#else
  return ni::OS::LINUX;
#endif
}

ni::ARCH ni::Codegen::defaultArch() { return ni::ARCH::x86_64; }
