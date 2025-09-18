#ifndef __CHA_H__
#define __CHA_H__

#include <string>

namespace cha {

enum class CompileFormat {
  LLVM_IR,
  ASSEMBLY_FILE,
  OBJECT_FILE,
  BINARY_FILE,
};

int compile(const std::string& file, CompileFormat format,
            const std::string& output_file);

} // namespace cha

#endif // __CHA_H__
