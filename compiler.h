#ifndef GUAVACOMPILER_H
#define GUAVACOMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "helpers/vector.h"
#include "helpers/buffer.h"


#define S_EQ(str, str2) \
        (str && str2 && (strcmp(str, str2) == 0))

#define NUMERIC_CASE \
    case '0': \
    case '1': \
    case '2': \
    case '3': \
    case '4': \
    case '5': \
    case '6': \
    case '7': \
    case '8': \
    case '9' 


#define OPERATOR_CASE_EXCLUDING_DIVISION \
    case '+':                           \
    case '-':                           \
    case '*':                           \
    case '>':                           \
    case '<':                           \
    case '^':                           \
    case '%':                           \
    case '!':                           \
    case '=':                           \
    case '~':                           \
    case '|':                           \
    case '&':                           \
    case '(':                           \
    case '[':                           \
    case ',':                           \
    case '.':                           \
    case '?'                         



#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'




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
    TOKEN_TYPE_IDENTIFIER,
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

enum {
    NUMBER_TYPE_NORMAL,
    NUMBER_TYPE_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLE
};

struct token 
{
    int type;
    int flags;
    struct pos pos;

    // to store the value of the token
    union
    {
        char cval;
        const char * sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void * any;
    };

    struct tokenNUmber {
        int type;
    }number;

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


void compilerError(struct compileProcess * compiler, const char * msg, ...);
void compilerWarning(struct compileProcess * compiler, const char * msg, ...);
char compileProcessNextChar(struct lexProcess * lexProcess);
char compileProcessPeekChar(struct lexProcess * lexProcess);
void compileProcessPushChar(struct lexProcess * lexProcess, char c);
bool tokenIsKeyword(struct token * token, const char * value);



struct lexProcess * lexProcessCreate(
    struct compileProcess * compiler, 
    struct lexProcessFunctions* functions,
    void * private
);
void lexProcessFree(struct lexProcess * process);
void * lexProcessPrivate(struct lexProcess * process);
struct vector * lexProcessTokens(struct lexProcess * process);
struct lexProcess * tokensBuildForString(
    struct compileProcess * compilerProc,
    const char * str
    );

int lex(struct lexProcess * process);


#endif