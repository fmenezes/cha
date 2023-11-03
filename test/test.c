#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cleanup();
int run_process(char *cmd, char **output);
void check_bin(const char *file, const char *expected_file);
void check_success(const char *file, const char *expected_file);
void check_failure(const char *file, const char *expected_file,
                   const char *msg);

void cleanup() { remove("out.ll"); }

void check_failure(const char *file, const char *expected_file,
                   const char *msg) {
  if (strstr(file, expected_file) == NULL) {
    return;
  }

  char cmd[1000];
  char *got_msg = NULL;
  sprintf(cmd, "ni -ll out.ll %s 2>&1", file);
  int ret = run_process(cmd, &got_msg);
  cleanup();
  if (ret == 0) {
    fprintf(stderr, "\"%s\" returned 0 expected non-zero\n", cmd);
    free(got_msg);
    exit(1);
  }
  if (strstr(got_msg, msg) == NULL) {
    fprintf(stderr, "expected: \"%s\", got: \"%s\"\n", msg, got_msg);
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

void check_success(const char *file, const char *expected_file) {
  if (strstr(file, expected_file) == NULL) {
    return;
  }

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

void check_bin(const char *file, const char *expected_file) {
  if (strstr(file, expected_file) == NULL) {
    return;
  }

  char cmd[1000];
  sprintf(cmd, "ni -o out %s 2>&1", file);
  int ret = system(cmd);
  if (ret != 0) {
    fprintf(stderr, "\"%s\" returned %d expected 0\n", cmd, ret);
    exit(1);
  }
  ret = system("./out");
  remove("out");
  exit(ret);
}

int main(int argc, char *argv[], char **envp) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <test>\n", argv[0]);
    return 1;
  }

  char *inputPath = argv[1];

  check_bin(inputPath, "test_bin.ni");
  check_success(inputPath, "parse_passes.ni");
  check_failure(inputPath, "parse_failure.ni", "syntax error");
  check_success(inputPath, "validation_passes.ni");
  check_failure(inputPath, "validation_arg_dup.ni",
                "argument 'i' already defined");
  check_failure(inputPath, "validation_arg_mismatch.ni",
                "function 'test' expects no arguments");
  check_failure(inputPath, "validation_arg_mismatch2.ni",
                "function 'test' expects arguments");
  check_failure(inputPath, "validation_arg_mismatch3.ni",
                "type mismatch expects 'uint8' passed 'int32'");
  check_failure(inputPath, "validation_dup_function.ni",
                "function 'test' already defined");
  check_failure(inputPath, "validation_function_not_found.ni",
                "function 'test' not found");
  check_failure(inputPath, "validation_ret_mismatch.ni",
                "return type mismatch expects 'int32' passed 'uint8'");
  check_failure(inputPath, "validation_type_mismatch.ni",
                "type mismatch expects 'uint8' passed 'int32'");
  check_failure(inputPath, "validation_type_mismatch2.ni",
                "type mismatch expects 'uint8' passed 'int32'");
  check_failure(inputPath, "validation_var_not_found.ni",
                "variable 'a' not found");
  check_failure(inputPath, "validation_var_redefined.ni",
                "variable 'a' already defined");
  check_failure(inputPath, "validation_var_redefined2.ni",
                "variable 'a' already defined");

  fprintf(stderr, "\"%s\" is unknown\n", inputPath);
  return 1;
}
