#ifndef __NI_PARSER_H__
#define __NI_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "ast.h"

int ni_parse(const char *file, ni_ast_node_list **out);

#ifdef __cplusplus
}
#endif

#endif // __NI_PARSER_H__
