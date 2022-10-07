#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cleanup() { remove("out.ll"); }

int check_for_failure(const char *filePath) {
  char suffix[] = "_passes.ni";
  if (strlen(filePath) < strlen(suffix)) {
    return 1;
  }

  if (strcmp(filePath + strlen(filePath) - strlen(suffix), suffix) == 0) {
    return 0;
  }

  return 1;
}

int run_process(char *cmd, char **output) {
  char buffer[10000];
  FILE *data = popen(cmd, "r");
  if (data == NULL) {
    sprintf(*output, "no pipe\n");
    return -1;
  }
  if (fgets(buffer, 10000, data) != NULL) {
    *output = strdup(buffer);
  }
  return pclose(data);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <path/to/nic> <test>\n", argv[0]);
    return 1;
  }

  char *nicPath = argv[1];
  char *inputPath = argv[2];
  char cmd[1000];
  char *msg_from_cmd = NULL;
  int ret = 0;
  FILE *file = NULL;
  FILE *data = NULL;

  if ((file = fopen(inputPath, "r")) != NULL) {
    fclose(file);
  } else {
    fprintf(stderr, "could not find %s\n", inputPath);
    return 1;
  }

  sprintf(cmd, "%s -ll out.ll %s 2>&1", nicPath, inputPath);
  ret = run_process(cmd, &msg_from_cmd);
  cleanup();

  fprintf(stderr, "got: %s", msg_from_cmd);
  free(msg_from_cmd);
  if (check_for_failure(inputPath)) {
    if (ret == 0) {
      fprintf(stderr, "\"%s\" returned %d expected non zero\n", cmd, ret);
      return 1;
    }
  } else {
    if (ret != 0) {
      fprintf(stderr, "\"%s\" returned %d expected 0\n", cmd, ret);
      return 1;
    }
  }

  return 0;
}
