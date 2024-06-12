#ifndef GUAVACOMPILER_H
#define GUAVACOMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include "helpers/vector.h"
#include "helpers/buffer.h"

enum
{
    LEXICAL_ANALYSIS_ALL_OK,
    LEXICAL_ANALYSIS_INPUT_ERROR
};

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

struct pos 
{
    int line;
    int col;
    const char * filename;
};

struct token 
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

};


struct lexProcess;
typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lexProcess* process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lexProcess* process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lexProcess* process, char c);

struct lexProcessFunctions
{
    LEX_PROCESS_NEXT_CHAR nextChar;
    LEX_PROCESS_PEEK_CHAR peekChar;
    LEX_PROCESS_PUSH_CHAR pushChar;
};

struct lexProcess 
{
    struct pos pos;
    struct vector * tokenVector;
    struct compileProcess * compiler;

    int currentExpressionCount;
    struct buffer * parenthesesBuffer;
    struct lexProcessFunctions * function;

    void * private;
    
};

struct compileProcess
{
    int flags;

    struct pos pos;
    struct _compileProcessInputFile 
    {
        FILE *fp;
        const char * absPath;
    } cFile;
    FILE * oFile;
};

int compileFile(const char * filename, const char * out_filename, int flags);
struct compileProcess * compileProcessCreate(const char * filename, const char * outFilename, int flags);


char compileProcessNextChar(struct lexProcess * lexProcess);
char compileProcessPeekChar(struct lexProcess * lexProcess);
void compileProcessPushChar(struct lexProcess * lexProcess, char c);



struct lexProcess * lexProcessCreate(
    struct compileProcess * compiler, 
    struct lexProcessFunctions* functions,
    void * private
);
void lexProcessFree(struct lexProcess * process);
void * lexProcessPrivate(struct lexProcess * process);
struct vector * lexProcessTokens(struct lexProcess * process);

int lex(struct lexProcess * process);


#endif