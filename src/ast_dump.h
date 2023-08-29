#ifndef __NI_AST_DUMP_H__
#define __NI_AST_DUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "ast.h"

void ni_ast_dump(FILE *out, const ni_ast_node_list *ast);

#ifdef __cplusplus
}
#endif

#endif // __NI_AST_DUMP_H__
