#ifndef __NI_CODEGEN_H__
#define __NI_CODEGEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nic/ast.h"
#include <stdio.h>

enum ni_ast_codegen_format {
  LLVM_IR,
  ASSEMBLY,
  OBJECT_FILE,
};

typedef enum ni_ast_codegen_format ni_ast_codegen_format;

int ni_ast_codegen(ni_ast_node_list *ast, ni_ast_codegen_format format,
                   char *file_path);

#ifdef __cplusplus
}
#endif

#endif // __NI_CODEGEN_H__
