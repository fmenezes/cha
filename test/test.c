#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cleanup();
int run_process(char *cmd, char **output);
void check_success(const char* file);
void check_failure(const char *file, const char *msg);

void cleanup() { remove("out.ll"); }

void check_failure(const char *file, const char* msg) {
  char cmd[1000];
  char *got_msg = NULL;
  sprintf(cmd, "ni -ll out.ll %s 2>&1", file);
  int ret = run_process(cmd, &got_msg);
  cleanup();
  if (ret == 0) {
    fprintf(stderr, "\"%s\" returned 0 expected non-zero\n", cmd);
    exit(1);
  }
  if (strstr(got_msg, msg) == NULL) {
    fprintf(stderr, "expected: \"%s\", got : \"%s\"\n", msg, got_msg);
    free(got_msg);
    exit(1);
  }
  free(got_msg);
  exit(0);
}

int run_process(char *cmd, char **output) {
  char buffer[10000];
  FILE *data = popen(cmd, "r");
  if (data == NULL) {
    if (output != NULL) {
      sprintf(*output, "no pipe\n");
    }
    return -1;
  }
  if (output != NULL && fgets(buffer, 10000, data) != NULL) {
    *output = strdup(buffer);
  }
  return pclose(data);
}

void check_success(const char* file) {
  char cmd[1000];
  sprintf(cmd, "ni -ll out.ll %s 2>&1", file);
  int ret = run_process(cmd, NULL);
  cleanup();
  if (ret != 0) {
    fprintf(stderr, "\"%s\" returned %d expected 0\n", cmd, ret);
    exit(1);
  }
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <test>\n", argv[0]);
    return 1;
  }

  char *inputPath = argv[1];
  
  if (strstr(inputPath, "parse_passes.ni") != NULL) {
    check_success(inputPath);
  }

  if (strstr(inputPath, "parse_failure.ni") != NULL) {
    check_failure(inputPath, "syntax error");
  }

  if (strstr(inputPath, "validation_passes.ni") != NULL) {
    check_success(inputPath);
  }

  if (strstr(inputPath, "validation_arg_dup.ni") != NULL) {
    check_failure(inputPath, "argument 'i' already defined");
  }

  if (strstr(inputPath, "validation_arg_mismatch.ni") != NULL) {
    check_failure(inputPath, "function 'test' expects no arguments");
  }

  if (strstr(inputPath, "validation_arg_mismatch2.ni") != NULL) {
    check_failure(inputPath, "function 'test' expects arguments");
  }

  if (strstr(inputPath, "validation_arg_mismatch3.ni") != NULL) {
    check_failure(inputPath, "type mismatch!!!");
  }

  if (strstr(inputPath, "validation_dup_function.ni") != NULL) {
    check_failure(inputPath, "function 'test' already defined");
  }

  if (strstr(inputPath, "validation_function_not_found.ni") != NULL) {
    check_failure(inputPath, "function 'test' not found");
  }

  if (strstr(inputPath, "validation_type_mismatch.ni") != NULL) {
    check_failure(inputPath, "type mismatch!!!");
  }

  if (strstr(inputPath, "validation_var_not_found.ni") != NULL) {
    check_failure(inputPath, "variable 'a' not found");
  }

  if (strstr(inputPath, "validation_var_redefined.ni") != NULL) {
    check_failure(inputPath, "variable 'a' already defined");
  }

  if (strstr(inputPath, "validation_var_redefined2.ni") != NULL) {
    check_failure(inputPath, "variable 'a' already defined");
  }

  fprintf(stderr, "\"%s\" is unknown\n", inputPath);
  return 1;
}
