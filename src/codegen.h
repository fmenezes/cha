#ifndef __CHA_CODEGEN_H__
#define __CHA_CODEGEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cha/cha.h"
#include <stdio.h>

int cha_ast_codegen(cha_ast_node_list *ast, cha_compile_format format,
                    const char *file_path);

#ifdef __cplusplus
}
#endif

#endif // __CHA_CODEGEN_H__
