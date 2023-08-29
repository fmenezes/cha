#ifndef __NI_H__
#define __NI_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ni_compile_format {
  NI_COMPILE_FORMAT_LLVM_IR,
  NI_COMPILE_FORMAT_ASSEMBLY_FILE,
  NI_COMPILE_FORMAT_OBJECT_FILE,
  NI_COMPILE_FORMAT_BINARY_FILE,
} ni_compile_format;

int ni_compile(const char *file, ni_compile_format format,
                   const char *output_file);

#ifdef __cplusplus
}
#endif

#endif // __NI_H__
