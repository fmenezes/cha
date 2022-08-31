#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cleanup() { system("rm -f out.ll"); } //POSIX ONLY

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

int main(int argc, char *argv[]) {
  char cmd[1000];
  int ret = 0;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <path/to/nic> <test>\n", argv[0]);
    return 1;
  }

  char *nicPath = argv[1];
  char *inputPath = argv[2];

  FILE *file;
  if ((file = fopen(inputPath, "r")) != NULL) {
    fclose(file);
  } else {
    fprintf(stderr, "could not find %s\n", inputPath);
    return 1;
  }

  sprintf(cmd, "%s -ll out.ll %s", nicPath, inputPath);
  ret = system(cmd);

  if (check_for_failure(inputPath)) {
    if (ret == 0) {
      cleanup();
      fprintf(stderr, "\"%s\" returned %d expected non zero\n", cmd, ret);
      return 1;
    }
  } else {
    if (ret != 0) {
      cleanup();
      fprintf(stderr, "\"%s\" returned %d expected 0\n", cmd, ret);
      return 1;
    }
  }

  cleanup();
  return 0;
}
