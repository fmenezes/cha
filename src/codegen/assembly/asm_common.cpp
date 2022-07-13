#include "ni/codegen/assembly/asm_common.hh"

int ni::codegen::assembly::register_size(
    ni::codegen::assembly::asm_register reg) {
  if (reg >= RAX) {
    return 64;
  } else if (reg >= EAX) {
    return 32;
  } else if (reg <= AX) {
    return 16;
  }
  return 8;
};

bool ni::codegen::assembly::is_directive(
    ni::codegen::assembly::asm_operation op) {
  return op < MOV;
}