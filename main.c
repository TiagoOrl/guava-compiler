#include <stdio.h>
#include "compiler.h"



int main(int argc, char const *argv[])
{
    int res = compileFile("./test.c", "test", 0);

    if (res == COMPILER_FILE_COMPILED_OK) {
        printf("compiled OK\n");
    } else if (res == COMPILER_FAILED_WITH_ERRORS) {
        printf("compile failed with errors\n");
    } else {
        printf("Unkowm response for compile file\n");
    }
    return 0;
}
