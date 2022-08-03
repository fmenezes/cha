#pragma once

namespace ni {
namespace codegen {
enum os { MACOS, LINUX };

enum arch { x86_64 };

enum format { ASSEMBLY_CODE, OBJECT_FILE, LLVM_IR };

os default_os();
arch default_arch();

class context {
public:
  context() : context(default_os(), default_arch()){};
  context(const os &os, const arch &arch) : target_os(os), target_arch(arch){};

  const os target_os;
  const arch target_arch;
};
} // namespace codegen
} // namespace ni
