#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"

#include <string.h>
#include <assert.h>


#define LEX_GETC_IF(buffer, c, exp)     \
    for (c = peekc(); exp; c = peekc()) \
    {                                   \
        buffer_write(buffer, c);        \
        nextc();                        \
    }                                   

static struct lexProcess * lexProcess;
static struct token tmpToken;
struct token * readNextToken();


static char peekc() {
    return lexProcess->function->peekChar(lexProcess);
}

static char nextc() {
    char c = lexProcess->function->nextChar(lexProcess);
    lexProcess->pos.col += 1;

    if (c == '\n') {
        lexProcess->pos.line += 1;
        lexProcess->pos.col = 1;
    }

    return c;
}


static void pushc(char c) {
    lexProcess->function->pushChar(lexProcess, c);
}

static struct pos lexFilePosition() {
    return lexProcess->pos;
}


const char * readNumberStr() {
    const char * num = NULL;
    struct buffer * buffer = buffer_create();
    char c = peekc();

    LEX_GETC_IF(buffer, c, (c >= '0' && c <= '9'));

    buffer_write(buffer, 0x00); // null terminator
    return buffer_ptr(buffer);
}

unsigned long long readNumber() {
    const char * s = readNumberStr();
    return atoll(s);
}


static struct token * lexerLastToken() {
    return vector_back_or_null(lexProcess->tokenVector);
}


static struct token * handleWhitespace() {

    struct token * lastToken = lexerLastToken();

    if (lastToken) {
        lastToken->whitespace = true;
    }

    nextc();
    return readNextToken();
}


struct token * tokenCreate(struct token * _token) {
    memcpy(&tmpToken, _token, sizeof(struct token));
    tmpToken.pos = lexFilePosition();

    return &tmpToken;
}

struct token * tokenMakeNumberForValue(unsigned long number) {
    return tokenCreate(&(struct token){.type=TOKEN_TYPE_NUMBER,.llnum=number});
}


struct token * tokenMakeNumber() {
    return tokenMakeNumberForValue(readNumber());
}


static struct token * tokenMakeString(char startDelim, char endDelim) {
    struct buffer * buf = buffer_create();

    assert(nextc() == startDelim);

    char c = nextc();

    for (; c != endDelim && c != EOF; c = nextc()) {
        if (c == '\\')
        {
            // we need to handle an escape character
            continue;
        }

        buffer_write(buf, c);
    }

    buffer_write(buf, 0x00);

    return tokenCreate(&(struct token){
        .type = TOKEN_TYPE_STRING, 
        .sval = buffer_ptr(buf)
        });
}


struct token * readNextToken() {
    struct token * token = NULL;

    char c = peekc();

    switch(c) 
    {
        NUMERIC_CASE:
            token = tokenMakeNumber();
            break;

        case '"':
            token = tokenMakeString('"', '"');

        // we dont care about whitespace, ignore them
        case ' ':
        case '\t':
            token = handleWhitespace();

        case EOF:
            break;

        default:
            compilerError(lexProcess->compiler, "Unexpected token");
    }

    return token;
}


int lex(struct lexProcess * process) {
    process->currentExpressionCount = 0;
    process->parenthesesBuffer = NULL;
    lexProcess = process;
    process->pos.filename = process->compiler->cFile.absPath;

    struct token * token = readNextToken();
    while(token) 
    {
        vector_push(process->tokenVector, token);
        token = readNextToken();
    }

    return LEXICAL_ANALYSIS_ALL_OK;
}