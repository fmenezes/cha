#include <stdarg.h>
#include <stdio.h>

#include "log.h"

void log_error(const char *format, ...) {
  fprintf(stderr, "error: ");

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}