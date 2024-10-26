#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"
#include <ctype.h>

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


static char assertNextChar(char c) {
    char nextC = nextc();

    assert(c == nextC);
    return nextC;
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

char lexGetEscapedChar(char c) {
    char co = 0;

    switch (c) {
        case 'n':
            co = '\n';
            break;

        case '\\':
            co = '\\';
            break;

        case 't':
            co = '\t';
            break;
        
        case '\'':
            co = '\'';
            break;
    }

    return co;
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

struct token * tokenMakeNewline() {
    nextc();
    return tokenCreate(&(struct token) {
        .type = TOKEN_TYPE_NEWLINE
    });
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


static bool opTreatedAsOne(char op) {
    return 
        op == '(' || 
        op == '[' || 
        op == ',' || 
        op == '.' || 
        op == '*' ||
        op == '?';
}


static bool isSingleOperator(char op) {
    return 
        op == '+' ||
        op == '-' ||
        op == '/' ||
        op == '*' ||
        op == '=' ||
        op == '>' ||
        op == '<' ||
        op == '|' ||
        op == '&' ||
        op == '^' ||
        op == '%' ||
        op == '~' ||
        op == '!' ||
        op == '(' ||
        op == '[' ||
        op == ',' ||
        op == '.' ||
        op == '?';

}


bool opValid(const char * op) {
    return 
        S_EQ(op, "+") ||
        S_EQ(op, "-") ||
        S_EQ(op, "*") ||
        S_EQ(op, "/") ||
        S_EQ(op, "!") ||
        S_EQ(op, "^") ||
        S_EQ(op, "+=") ||
        S_EQ(op, "-=") ||
        S_EQ(op, "*=") ||
        S_EQ(op, "/=") ||
        S_EQ(op, ">>") ||
        S_EQ(op, "<<") ||
        S_EQ(op, ">=") ||
        S_EQ(op, "<=") ||
        S_EQ(op, ">") ||
        S_EQ(op, "<") ||
        S_EQ(op, "||") ||
        S_EQ(op, "&&") ||
        S_EQ(op, "|") ||
        S_EQ(op, "&") ||
        S_EQ(op, "++") ||
        S_EQ(op, "--") ||
        S_EQ(op, "=") ||
        S_EQ(op, "!=") ||
        S_EQ(op, "==") ||
        S_EQ(op, "->") ||
        S_EQ(op, "-") ||
        S_EQ(op, "(") ||
        S_EQ(op, "[") ||
        S_EQ(op, ",") ||
        S_EQ(op, ".") ||
        S_EQ(op, "...") ||
        S_EQ(op, "~") ||
        S_EQ(op, "?") ||
        S_EQ(op, "%");

}

void readOpFlushBackButKeepFirst(struct buffer * buffer) {
    const char * data = buffer_ptr(buffer);
    int len = buffer->len;

    for (int i = len - 1; i >= 1; i--) {
        if (data[i] == 0x00)
            continue;

        pushc(data[i]);
    }
}


const char * readOp() {
    bool singleOperator = true;
    char op = nextc();

    struct buffer * buffer = buffer_create();
    buffer_write(buffer, op);

    // if op is like ++ -- += -=...
    if (!opTreatedAsOne(op)) {
        op = peekc();

        if (isSingleOperator(op)) {
            buffer_write(buffer, op);
            nextc();
            singleOperator = false;
        }
    }
    // write null terminator
    buffer_write(buffer, 0x00);
    char * ptr = buffer_ptr(buffer);

    if (!singleOperator) {
        if (!opValid(ptr)) {
            readOpFlushBackButKeepFirst(buffer);
            ptr[1] = 0x00;
        }

    } else if(!opValid(ptr)) {
        compilerError(lexProcess->compiler, "The operator %s is not valid", ptr);
    }

    return ptr;
}

bool lexIsInExpression() {
    return lexProcess->currentExpressionCount > 0;
}



static void lexNewExpression() {
    lexProcess->currentExpressionCount++;

    if (lexProcess->currentExpressionCount == 1)
        lexProcess->parenthesesBuffer = buffer_create();
    
}


static void lexFinishExpression() {
    lexProcess->currentExpressionCount--;

    // had right bracket without a left one = ) 
    // closing expression that doesnt exist
    if(lexProcess->currentExpressionCount < 0) {
        compilerError(lexProcess->compiler, "You closed an expression that you never opened\n");
    }
}


bool isKeyword(const char * str) {
     return 
        S_EQ(str,"unsigned") ||
        S_EQ(str,"signed") ||
        S_EQ(str,"char") ||
        S_EQ(str,"short") ||
        S_EQ(str,"int") ||
        S_EQ(str,"long") ||
        S_EQ(str,"float") ||
        S_EQ(str,"double") ||
        S_EQ(str,"void") ||
        S_EQ(str,"struct") ||
        S_EQ(str,"union") ||
        S_EQ(str,"static") ||
        S_EQ(str,"__ignore_typecheck") ||
        S_EQ(str,"return") ||
        S_EQ(str,"include") ||
        S_EQ(str,"sizeof") ||
        S_EQ(str,"if") ||
        S_EQ(str,"else") ||
        S_EQ(str,"while") ||
        S_EQ(str,"for") ||
        S_EQ(str,"do") ||
        S_EQ(str,"break") ||
        S_EQ(str,"continue") ||
        S_EQ(str,"switch") ||
        S_EQ(str,"case") ||
        S_EQ(str,"default") ||
        S_EQ(str,"goto") ||
        S_EQ(str,"typedef") ||
        S_EQ(str,"const") ||
        S_EQ(str,"extern") ||
        S_EQ(str,"restrict");

}


static struct token * tokenMakeOperatorOrString() {
    char op = peekc();
    if (op == '<') {
        struct token * lastToken = lexerLastToken();

        if (tokenIsKeyword(lastToken, "include")) {
            return tokenMakeString('<', '>');
        }
    }

    struct token * token = tokenCreate(
        &(struct token){
            .type = TOKEN_TYPE_OPERATOR,
            .sval = readOp()
        });

    if (op == '(') {
        lexNewExpression();
    }

    return token;
}


struct token * tokenMakeOnelineComment() {
    struct buffer * buffer = buffer_create();
    char c = 0;

    LEX_GETC_IF(buffer, c, c != '\n' && c != EOF);

    return tokenCreate(&(struct token){
        .type = TOKEN_TYPE_COMMENT,
        .sval = buffer_ptr(buffer)
    });

    while(1) {
        LEX_GETC_IF(buffer, c, c != '*' && c != EOF);

        if (c == EOF) {
            compilerError(lexProcess->compiler, "You did not close this multiline comment\n");
        }

        else if (c == '*') {
            // skip the * 
            nextc();

            if (peekc() == '/') {
                nextc();
                break;
            }
        }
    }

    return tokenCreate(&(struct token) {
        .type = TOKEN_TYPE_COMMENT,
        .sval = buffer_ptr(buffer)
    });
}



struct token * tokenMakeMultilineComment() {
    struct buffer * buffer = buffer_create();
    char c = 0;
}


struct token * handleComment() {
    char c = peekc();

    if (c == '/') {
        nextc();

        if (peekc() == '/') {
            nextc();
            return tokenMakeOnelineComment();
        }

        else if (peekc() == '*') {
            nextc();
            return tokenMakeMultilineComment();
        }

        pushc('/');
        return tokenMakeOperatorOrString();
    }

    return NULL;
}



static struct token * tokenMakeSymbol() {
    char c = nextc();

    if (c == ')') {
        lexFinishExpression();
    }

    struct token * token  = tokenCreate(
        &(struct token) {
            .type = TOKEN_TYPE_SYMBOL,
            .cval = c
        });

    return token;
}


static struct token * tokenMakeIdentifierOrKeyword() {
    struct buffer * buffer = buffer_create();
    char c = 0;

    LEX_GETC_IF(buffer, c, 
        (c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        (c == '_')
    );

    buffer_write(buffer, 0x00);

    // check if its a keyword
    if (isKeyword(buffer_ptr(buffer))) {
        return tokenCreate(&(struct token) {
            .type=TOKEN_TYPE_KEYWORD,
            .sval=buffer_ptr(buffer)
        });
    }

    return tokenCreate(&(struct token) {
        .type = TOKEN_TYPE_IDENTIFIER,
        .sval = buffer_ptr(buffer)
    });
}

struct token * readSpecialToken() {
    char c = peekc();

    if (isalpha(c) || c == '_') {
        return tokenMakeIdentifierOrKeyword();
    }

    return NULL;
}

void lexerPopToken() {
    vector_pop(lexProcess->tokenVector);
}

bool isHexChar(char c) {
    c = tolower(c);
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}


const char * readHexNumberStr() {
    struct buffer * buffer = buffer_create();

    char c = peekc();
    LEX_GETC_IF(buffer, c, isHexChar(c));

    buffer_write(buffer, 0x00);

    return buffer_ptr(buffer);
}


struct token * tokenMakeSpecialNumberHex() {
    // skip the 'x'
    nextc();

    unsigned long number = 0;
    const char *  numberString = readHexNumberStr();

    number = strtol(numberString, 0, 16);

    
    return tokenMakeNumberForValue(number);
}


struct token * tokenMakeSpecialNumber() {
    struct token * token = NULL;
    struct token * lastToken = lexerLastToken();

    lexerPopToken();

    char c = peekc();

    if (c == 'x') {
        token = tokenMakeSpecialNumberHex();
    }


    return token;
}


struct token * tokenMakeQuote() {
    assertNextChar('\'');
    char c = nextc();
    
    if (c == '\\') {
        c = nextc();
        c = lexGetEscapedChar(c);
    }

    if (nextc() != '\'') {
        compilerError(lexProcess->compiler, "You opened a quote but did not close it");
    }

    return tokenCreate(&(struct token) {
        .type = TOKEN_TYPE_NUMBER,
        .cval = c
    });
}


struct token * readNextToken() {
    struct token * token = NULL;

    char c = peekc();

    token = handleComment();

    if (token) 
        return token;

    switch(c) 
    {
        NUMERIC_CASE:
            token = tokenMakeNumber();
            break;

        OPERATOR_CASE_EXCLUDING_DIVISION:
            token = tokenMakeOperatorOrString();
            break;

        SYMBOL_CASE:
            token = tokenMakeSymbol();
            break;

        case 'x':
            token = tokenMakeSpecialNumber();
            break;

        case '"':
            token = tokenMakeString('"', '"');
            break;

        case '\'':
            token = tokenMakeQuote();
            break;

        // we dont care about whitespace, ignore them
        case ' ':
        case '\t':
            token = handleWhitespace();
            break;

        case '\n':
        token = tokenMakeNewline();
            break;

        case EOF:
            break;

        default:
            token = readSpecialToken();
            if (!token) 
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