#include "compiler.h"
#include "helpers/vector.h"


static struct compileProcess * currentProcess;
static struct token* parserLastToken;



static void parserIgnoreNlOrComment(struct token* token) {
    while(token && tokenIsNlOrCommentOrNewlineSeparator(token)) {
        // skip the tokan
        vector_peek(currentProcess->tokenVec);
        token = vector_peek_no_increment(currentProcess->tokenVec);
    }
}

static struct token* tokenNext() {
    struct token* nextToken = vector_peek_no_increment(currentProcess->tokenVec);
    parserIgnoreNlOrComment(nextToken);
    currentProcess->pos = nextToken->pos;
    parserLastToken = nextToken;
    return vector_peek(currentProcess->tokenVec);
}


static struct token* tokenPeekNext() {
    struct token* nextToken = vector_peek_no_increment(currentProcess->tokenVec);
    parserIgnoreNlOrComment(nextToken);

    return vector_peek_no_increment(currentProcess->tokenVec);
}


void parseSingleTokenToNode() {
    struct token* token = tokenNext();
    struct node* node = NULL;

    switch(token->type) {
        case TOKEN_TYPE_NUMBER:
            node = nodeCreate(&(struct node){
                .type = NODE_TYPE_NUMBER, 
                .llnum = token->llnum
            });
            break;

        case TOKEN_TYPE_IDENTIFIER:
            node = nodeCreate(&(struct node) {
                .type = NODE_TYPE_IDENTIFIER,
                .sval = token->sval
            });
            break;
        
        case TOKEN_TYPE_STRING:
            node = nodeCreate(&(struct node) {
                .type=NODE_TYPE_STRING,
                .sval = token->sval
            });
            break;

        default:
            compilerError(currentProcess, "this is not a single token that can be converted to a node");
    }
}


int parseNext() {
    struct token* token = tokenPeekNext();

    if (!token) 
        return -1;

    int res = 0;
    switch(token->type) {

        case TOKEN_TYPE_NUMBER:
        case TOKEN_TYPE_IDENTIFIER:
        case TOKEN_TYPE_STRING:
            parseSingleTokenToNode();
            break;
    }

    return 0;
}


int parse(struct compileProcess * process) {
    currentProcess = process;
    parserLastToken = NULL;

    nodeSetVector(process->nodeVec, process->nodeTreeVec);

    vector_set_peek_pointer(process->tokenVec, 0);
    struct node* node = NULL;

    while(parseNext() == 0) 
    {
        node = nodePeek();
        vector_push(process->nodeTreeVec, &node);
    }

    return PARSE_ALL_OK;
}