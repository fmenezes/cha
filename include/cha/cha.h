#ifndef __CHA_H__
#define __CHA_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cha_compile_format {
  CHA_COMPILE_FORMAT_LLVM_IR,
  CHA_COMPILE_FORMAT_ASSEMBLY_FILE,
  CHA_COMPILE_FORMAT_OBJECT_FILE,
  CHA_COMPILE_FORMAT_BINARY_FILE,
} cha_compile_format;

int cha_compile(const char *file, cha_compile_format format,
               const char *output_file);

#ifdef __cplusplus
}
#endif

#endif // __CHA_H__
