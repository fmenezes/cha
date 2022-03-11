#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/ast.hh"

namespace ni {
namespace codegen {
enum OS { MACOS, LINUX };

enum ARCH { x86_64 };

class Codegen {
public:
  Codegen(const ni::ast::NProgram &p)
      : Codegen(p, defaultOs(), defaultArch()){};
  Codegen(const ni::ast::NProgram &p, const OS &os, const ARCH &arch)
      : program(p), targetOS(os), targetArch(arch){};
  virtual int codegen(const std::string &output, std::string &error) = 0;
  virtual ~Codegen() {}
  static OS defaultOs();
  static ARCH defaultArch();

protected:
  const ni::ast::NProgram &program;
  OS targetOS;
  ARCH targetArch;
};
} // namespace codegen
} // namespace ni
