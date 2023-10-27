#ifndef __NI_VALIDATE_H__
#define __NI_VALIDATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"

int ni_validate(ni_ast_node_list *ast);
void type_str(const ni_ast_type *ast_type, char **str);

#ifdef __cplusplus
}
#endif

#endif // __NI_VALIDATE_H__
