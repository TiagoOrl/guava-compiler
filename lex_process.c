#include "compiler.h"
#include <stdlib.h>
#include "helpers/vector.h"

struct lexProcess * lexProcessCreate(
    struct compileProcess * compiler, 
    struct lexProcessFunctions* functions,
    void * private
) {
    struct lexProcess * process = calloc(1, sizeof(struct lexProcess));
    process->function = functions;
    process->tokenVector = vector_create(sizeof(struct token));
    process->compiler = compiler;
    process->private = private;
    process->pos.line = 1;
    process->pos.col = 1;

    return process;
}


void lexProcessFree(struct lexProcess * process) {
    vector_free(process->tokenVector);
    free(process);
}


void * lexProcessPrivate(struct lexProcess * process) {
    return process->private;
}

struct vector * lexProcessTokens(struct lexProcess * process) {
    return process->tokenVector;
}