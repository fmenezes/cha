#ifndef __NI_CODEGEN_H__
#define __NI_CODEGEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ni/ni.h"
#include <stdio.h>

int ni_ast_codegen(ni_ast_node_list *ast, ni_compile_format format,
                   const char *file_path);

#ifdef __cplusplus
}
#endif

#endif // __NI_CODEGEN_H__
