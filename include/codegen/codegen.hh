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

class Context {
public:
  Context() : Context(defaultOs(), defaultArch()){};
  Context(const OS &os, const ARCH &arch)
      : targetOS(os), targetArch(arch){};

  const OS targetOS;
  const ARCH targetArch;

  static OS defaultOs();
  static ARCH defaultArch();
};

class Codegen {
public:
  Codegen(const ni::ast::NProgram &p)
      : program(p){};
  Codegen(const ni::ast::NProgram &p, const Context &context)
      : program(p), context(context){};
  virtual int codegen(const std::string &output, std::string &error) = 0;
  virtual ~Codegen() {}
  const Context context;

protected:
  const ni::ast::NProgram &program;
};
} // namespace codegen
} // namespace ni
