#ifndef __NI_VALIDATE_H__
#define __NI_VALIDATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"

#define TYPE_STR_LEN 30

int ni_validate(ni_ast_node_list *ast);
void type_str(char *out, const ni_ast_type *ast_type);

#ifdef __cplusplus
}
#endif

#endif // __NI_VALIDATE_H__
