#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

compileProcess * compileProcessCreate(const char * filename, const char * outFilename, int flags) {
    FILE * file  = fopen(filename, "r");

    if (!file) 
        return NULL;


    FILE * outFile = NULL;

    if (outFilename) {
        outFile = fopen(outFilename, "w");

    if (!outFile)
        return NULL;
    }
    
    compileProcess * process = calloc(1, sizeof(compileProcess));
    process->flags = flags;
    process->cFile.fp = file;
    process->oFile = outFile;

    return process;
}