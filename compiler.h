#ifndef GUAVACOMPILER_H
#define GUAVACOMPILER_H

#include <stdio.h>
#include <stdbool.h>

enum
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};

enum
{
    TOKEN_TYPE_ID,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE
};

struct _pos 
{
    int line;
    int col;
    const char * filename;
} pos;

struct _token 
{
    int type;
    int flags;


    // to store the value of the token
    union
    {
        char cval;
        const char * sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long ll;
        void * any;
    };

    // true if there is a whitespace between the token and the next token
    bool whitespace;

    // ex: (5 + 10 + 20)
    const char * between_brackets;

} token;

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