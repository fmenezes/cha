#ifndef __CHA_VALIDATE_H__
#define __CHA_VALIDATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"

#define TYPE_STR_LEN 30

int cha_validate(cha_ast_node_list *ast);
void type_str(char *out, const cha_ast_type *ast_type);

#ifdef __cplusplus
}
#endif

#endif // __CHA_VALIDATE_H__
