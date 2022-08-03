#include "ni/codegen/context.hh"

ni::codegen::os ni::codegen::default_os() {
#if defined(__APPLE__) || defined(__MACH__)
  return ni::codegen::os::MACOS;
#else
  return ni::codegen::os::LINUX;
#endif
}

ni::codegen::arch ni::codegen::default_arch() {
  return ni::codegen::arch::x86_64;
}
