#ifndef __CHA_PARSER_H__
#define __CHA_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "ast.h"

int cha_parse(const char *file, cha_ast_node_list **out);

#ifdef __cplusplus
}
#endif

#endif // __CHA_PARSER_H__
