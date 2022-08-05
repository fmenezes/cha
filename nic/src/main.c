#include <stdlib.h>
#include <stdio.h>

#include "nic/ast.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return 1;
    }

    ni_ast_node_list *ast = ni_ast_parse(file);
    fclose(file);
    if (ast == NULL) {
      return 1;
    }

    ni_ast_dump(ast);

    return 0;
}
