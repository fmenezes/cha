#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"

void log_validation_error(ni_ast_location loc, const char *format, ...);
void log_error(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // __LOG_H__
