#include "compiler.h"
#include "helpers/vector.h"

static struct lexProcess * lexProcess;

static char peekc() {
    return lexProcess->function->peekChar(lexProcess);
}

static void pushc(char c) {
    lexProcess->function->pushChar(lexProcess, c);
}

struct token * readNextToken() {
    struct token * token = NULL;

    char c = peekc();

    switch(c) 
    {
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