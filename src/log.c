#include <stdarg.h>
#include <stdio.h>

#include "ast.h"
#include "log.h"

void log_validation_error(cha_ast_location loc, const char *format, ...) {
  fprintf(stderr, "[error] %s:%d:%d", loc.file, loc.line_begin,
          loc.column_begin);
  if (loc.line_begin != loc.line_end) {
    fprintf(stderr, "-%d:%d", loc.line_end, loc.column_end);
  } else if (loc.column_begin != loc.column_end) {
    fprintf(stderr, "-%d", loc.column_end);
  }

  fprintf(stderr, ": ");
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}

void log_error(const char *format, ...) {
  fprintf(stderr, "[error] ");

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}
