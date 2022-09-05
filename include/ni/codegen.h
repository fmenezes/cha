#ifndef __NI_CODEGEN_H__
#define __NI_CODEGEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ni/ast.h"
#include <stdio.h>

typedef enum ni_ast_codegen_format {
  NI_CODEGEN_FORMAT_LLVM_IR,
  NI_CODEGEN_FORMAT_ASSEMBLY_FILE,
  NI_CODEGEN_FORMAT_OBJECT_FILE,
  NI_CODEGEN_FORMAT_BINARY_FILE,
} ni_ast_codegen_format;

int ni_ast_codegen(ni_ast_node_list *ast, ni_ast_codegen_format format,
                   const char *file_path);

#ifdef __cplusplus
}
#endif

#endif // __NI_CODEGEN_H__
