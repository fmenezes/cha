#ifndef __NI_VALIDATE_H__
#define __NI_VALIDATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ni/ast.h"

int ni_validate(ni_ast_node_list *ast);

#ifdef __cplusplus
}
#endif

#endif // __NI_VALIDATE_H__
