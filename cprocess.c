#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "helpers/vector.h"

struct compileProcess * compileProcessCreate(const char * filename, const char * outFilename, int flags) {
    FILE * file  = fopen(filename, "r");

    if (!file) 
        return NULL;


    FILE * outFile = NULL;

    if (outFilename) {
        outFile = fopen(outFilename, "w");

    if (!outFile)
        return NULL;
    }
    
    struct compileProcess * process = calloc(1, sizeof(struct compileProcess));
    process->nodeVec = vector_create(sizeof(struct node*));
    process->nodeTreeVec = vector_create(sizeof(struct node*));
    
    process->flags = flags;
    process->cFile.fp = file;
    process->oFile = outFile;

    return process;
}


char compileProcessNextChar(struct lexProcess * lexProcess) {
    struct compileProcess * compiler = lexProcess->compiler;
    compiler->pos.col += 1;

    char c = getc(compiler->cFile.fp);

    if (c == '\n')
    {
        compiler->pos.line += 1;
        compiler->pos.col = 1;
    }

    return c;
}

char compileProcessPeekChar(struct lexProcess * lexProcess) {
    struct compileProcess * compiler = lexProcess->compiler;
    char c = getc(compiler->cFile.fp);
    ungetc(c, compiler->cFile.fp);

    return c;
}

void compileProcessPushChar(struct lexProcess * lexProcess, char c) {
    struct compileProcess * compiler = lexProcess->compiler;
    ungetc(c, compiler->cFile.fp);
}