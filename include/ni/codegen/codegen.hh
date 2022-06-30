#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ni/ast/ast.hh"

namespace ni {
namespace codegen {
enum os { MACOS, LINUX };

enum arch { x86_64 };

class context {
public:
  context() : context(default_os(), default_arch()){};
  context(const os &os, const arch &arch) : target_os(os), target_arch(arch){};

  const os target_os;
  const arch target_arch;

  static os default_os();
  static arch default_arch();
};

class codegen {
public:
  codegen(const ni::ast::program &p) : program(p){};
  codegen(const ni::ast::program &p, const context &ctx)
      : program(p), ctx(ctx){};
  virtual void generate(const std::string &output) = 0;
  virtual ~codegen() {}
  const context ctx;

protected:
  const ni::ast::program &program;
};
} // namespace codegen
} // namespace ni
