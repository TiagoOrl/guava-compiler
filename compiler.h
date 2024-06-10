#ifndef GUAVACOMPILER_H
#define GUAVACOMPILER_H

#include <stdio.h>

enum
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};

struct _compileProcess 
{
    int flags;
    struct _compileProcessInputFile 
    {
        FILE *fp;
        const char * absPath;
    } cFile;
    FILE * oFile;
};
typedef struct _compileProcess compileProcess;

int compileFile(const char * filename, const char * out_filename, int flags);
compileProcess * compileProcessCreate(const char * filename, const char * outFilename, int flags);

#endif