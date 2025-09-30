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
  sprintf(cmd, "cha -ll out.ll %s 2>&1", file);
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
  sprintf(cmd, "cha -ll out.ll %s 2>&1", file);
  char *got_msg = NULL;
  int ret = run_process(cmd, &got_msg);
  cleanup();
  if (ret != 0) {
    fprintf(stderr, "\"%s\" returned %d expected 0\nerror message:%s\n", cmd,
            ret, got_msg);
    free(got_msg);
    exit(1);
  }
  free(got_msg);
  exit(0);
}

void check_bin(const char *file, const char *expected_file) {
  if (strstr(file, expected_file) == NULL) {
    return;
  }

  char cmd[1000];
  sprintf(cmd, "cha -o out %s 2>&1", file);
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

  check_bin(inputPath, "test_bin.cha");

  check_success(inputPath, "parse_passes.cha");
  check_success(inputPath, "operator_passes.cha");
  check_success(inputPath, "type_assign_passes.cha");
  check_success(inputPath, "validation_passes.cha");
  check_success(inputPath, "if_passes.cha");
  check_success(inputPath, "if_else_passes.cha");

  check_failure(inputPath, "parse_failure.cha", "syntax error");
  check_failure(inputPath, "validation_arg_dup.cha",
                "argument 'i' already defined");
  check_failure(inputPath, "validation_arg_mismatch.cha",
                "function 'test' expects no arguments");
  check_failure(inputPath, "validation_arg_mismatch2.cha",
                "function 'test' expects arguments");
  check_failure(inputPath, "validation_arg_mismatch3.cha",
                "type mismatch expects 'uint8' passed 'int'");
  check_failure(inputPath, "validation_dup_function.cha",
                "'test' already defined");
  check_failure(inputPath, "validation_function_not_found.cha",
                "function 'test' not found");
  check_failure(inputPath, "validation_ret_mismatch.cha",
                "return type mismatch expects 'int' passed 'uint8'");
  check_failure(inputPath, "validation_type_mismatch.cha",
                "type mismatch expects 'int' passed 'uint8'");
  check_failure(inputPath, "validation_type_mismatch2.cha",
                "incompatible types found for operation: 'int', 'uint8'");
  check_failure(inputPath, "validation_var_not_found.cha", "'a' not found");
  check_failure(inputPath, "validation_var_redefined.cha",
                "variable 'a' already defined");
  check_failure(inputPath, "validation_var_redefined2.cha",
                "variable 'a' already defined");
  check_failure(inputPath, "if_condition_not_bool.cha",
                "condition should return bool");

  fprintf(stderr, "\"%s\" is unknown\n", inputPath);
  return 1;
}
