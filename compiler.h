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

    // vector of tokens from lexical analysis.
    struct vector * tokenVec;
    struct vector * nodeVec;
    struct vector * nodeTreeVec;
    FILE * oFile;
};

enum
{
    PARSE_ALL_OK,
    PARSE_GENERAL_ERROR
};

enum
{
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_EXPRESSION_PARENTHESES,
    NODE_TYPE_NUMBER,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_STRING,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_VARIABLE_LIST,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_BODY,
    NODE_TYPE_STATEMENT_RETURN,
    NODE_TYPE_STATEMENT_IF,
    NODE_TYPE_STATEMENT_ELSE,
    NODE_TYPE_STATEMENT_WHILE,
    NODE_TYPE_STATEMENT_DO_WHILE,
    NODE_TYPE_STATEMENT_FOR,
    NODE_TYPE_STATEMENT_BREAK,
    NODE_TYPE_STATEMENT_CCONTINUE,
    NODE_TYPE_STATEMENT_SWITCH,
    NODE_TYPE_STATEMENT_CASE,
    NODE_TYPE_STATEMENT_DEFAULT,
    NODE_TYPE_STATEMENT_GOTO,

    NODE_TYPE_UNARY,
    NODE_TYPE_TERNARY,
    NODE_TYPE_LABEL,
    NODE_TYPE_STRUCT,
    NODE_TYPE_UNION,
    NODE_TYPE_BRACKET,
    NODE_TYPE_CAST,
    NODE_TYPE_BLANK
};


enum {
    NODE_FLAG_INSIDE_EXPRESSION = 0b000000001
};


struct node 
{
    int type;
    int flags;


    struct pos pos;

    struct nodeBinded
    {
        // pointer to body node
        struct node* owner;

        // pointer to the function this node is in
        struct node* function;
    } binded;

    union 
    {
        struct exp {
            struct node* left;
            struct node*right;
            const char* op;
        } exp;
    };

    union 
    {
        char cval;
        const char * sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
    };
};


int compileFile(const char * filename, const char * out_filename, int flags);
struct compileProcess * compileProcessCreate(const char * filename, const char * outFilename, int flags);


void compilerError(struct compileProcess * compiler, const char * msg, ...);
void compilerWarning(struct compileProcess * compiler, const char * msg, ...);
char compileProcessNextChar(struct lexProcess * lexProcess);
char compileProcessPeekChar(struct lexProcess * lexProcess);
void compileProcessPushChar(struct lexProcess * lexProcess, char c);


bool tokenIsKeyword(struct token * token, const char * value);
bool tokenIsSymbol(struct token* token, char c);
bool tokenIsNlOrCommentOrNewlineSeparator(struct token* token);

struct node* nodeCreate(struct node* _node);
struct node* nodePop();
struct node* nodePeek();
struct node* nodePeekOrNull();
void nodePush(struct node* node);
void nodeSetVector(struct vector* vec, struct vector* rootVec);
bool nodeIsExpressionable(struct node* node);
struct node* nodePeekExpressionableOrNull();


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
int parse(struct compileProcess * process);

void makeExpNode(struct node* leftNode, struct node* rightNode, const char* op);


#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATORS_IN_GROUP 12


enum
{
    ASSOCIATIVITY_LEFT_TO_RIGHT,
    ASSOCIATIVITY_RIGHT_TO_LEFT
};


struct expressionableOpPrecedenceGroup
{
    char* operators[MAX_OPERATORS_IN_GROUP];
    int associativity;
};

#endif