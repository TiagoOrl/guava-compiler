#include <assert.h>
#include "compiler.h"
#include "helpers/vector.h"





struct history {
    int flags;
};

extern struct expressionableOpPrecedenceGroup opPrecedence[TOTAL_OPERATOR_GROUPS];

int parseExpressionalbleSingle(struct history* history);
void parseExpressionable(struct history* history);
void parserDatatypeAdjustForSecondary(struct datatype* datatype, struct token* datatypeSecondaryToken);

static struct compileProcess * currentProcess;
static struct token* parserLastToken;



struct history* historyBegin(int flags) {
    struct history* history = calloc(1, sizeof(struct history));
    history->flags = flags;
    return history;
}


struct history* historyDown(struct history* history, int flags) {
    struct history* newHistory = calloc(1, sizeof(struct history));
    memcpy(newHistory, history, sizeof(struct history));
    newHistory->flags = flags;

    return newHistory;
}


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


static bool tokenNextIsOperator(const char* op) {
    struct token* token = tokenPeekNext();
    return tokenIsOperator(token, op);
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


void parseExpressionableForOp(struct history* history, const char * op) {
    parseExpressionable(history);
}


static int parserGetPrecedenceForOperator(const char* op, struct expressionableOpPrecedenceGroup** groupOut) {
    *groupOut = NULL;
    for (int i = 0; i < TOTAL_OPERATOR_GROUPS; i++) {
        for (int b = 0; opPrecedence[i].operators[b]; b++) {
            const char* _op = opPrecedence[i].operators[b];
            if (S_EQ(op, _op))
            {
                *groupOut = &opPrecedence[i];
                return i;
            }
        }
    }

    return -1;
}


static bool parserLeftOpHasPriority(const char* opLeft, const char* opRight) {
    struct expressionableOpPrecedenceGroup *groupLeft = NULL;
    struct expressionableOpPrecedenceGroup *groupRight = NULL;

    if (S_EQ(opLeft, opRight))
    {
        return false;
    }

    int precedenceLeft = parserGetPrecedenceForOperator(opLeft, &groupLeft);
    int precedenceRight = parserGetPrecedenceForOperator(opRight, &groupRight);

    if (groupLeft->associativity == ASSOCIATIVITY_RIGHT_TO_LEFT)
        return false;

    return precedenceLeft <= precedenceRight;
}


void parserNodeShiftChildrenLeft(struct node* node) {
    assert(node->type == NODE_TYPE_EXPRESSION);
    assert(node->exp.right->type == NODE_TYPE_EXPRESSION);

    const char* rightOp = node->exp.right->exp.op;
    struct node* newExpLeftNode = node->exp.left;
    struct node* newExpRightNode = node->exp.right->exp.left;

    makeExpNode(newExpLeftNode, newExpRightNode, node->exp.op);

    struct node* newLeftOperand = nodePop();
    struct node* newRightOperand = node->exp.right->exp.right;

    node->exp.left = newLeftOperand;
    node->exp.right = newRightOperand;
    node->exp.op = rightOp;
}


void parserReorderExpression(struct node** nodeOut) {
    struct node* node = *nodeOut;

    if (node->type != NODE_TYPE_EXPRESSION)
        return;

    // no expressions, nothing to do
    if (node->exp.left->type != NODE_TYPE_EXPRESSION && node->exp.right && 
        node->exp.right->type != NODE_TYPE_EXPRESSION)
        return;


    if (node->exp.left->type != NODE_TYPE_EXPRESSION && node->exp.right &&
        node->exp.right->type ==  NODE_TYPE_EXPRESSION) {
            const char* rightOp = node->exp.right->exp.op;

            if(parserLeftOpHasPriority(node->exp.op, rightOp)) {
                parserNodeShiftChildrenLeft(node);
                parserReorderExpression(&node->exp.left);
                parserReorderExpression(&node->exp.right);
            }
        }
        
}


void parseExpNormal(struct history* history) {
    struct token* opToken = tokenPeekNext();
    const char * op = opToken->sval;

    struct node* nodeLeft = nodePeekExpressionableOrNull();

    if (!nodeLeft)
        return;

    // pop off the operator token
    tokenNext();

    // pop off the left node
    nodePop();
    nodeLeft->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    parseExpressionableForOp(historyDown(history, history->flags), op);

    // pop off right node
    struct node* nodeRight = nodePop();
    nodeRight->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    

    // pop off expression node
    makeExpNode(nodeLeft, nodeRight, op);
    struct node* nodeExp = nodePop();


    // reorder the expression
    parserReorderExpression(&nodeExp);
    nodePush(nodeExp);
}


int parseExp(struct history* history) {
    parseExpNormal(history);
    return 0;
}


void parseIdentifier(struct history* history) {
    assert(tokenPeekNext()->type == NODE_TYPE_IDENTIFIER);
    parseSingleTokenToNode();
}


static bool isKeywordVariableModifier(const char* val) {
    return 
        S_EQ(val, "unsigned") ||
        S_EQ(val, "signed") ||
        S_EQ(val, "static") ||
        S_EQ(val, "const") ||
        S_EQ(val, "extern") ||
        S_EQ(val, "__ignore_typecheck__");

}


void parseDatatypeModifiers(struct datatype * dtype){
    struct token* token = tokenPeekNext();

    while(token && token->type == TOKEN_TYPE_KEYWORD) {
        if (!isKeywordVariableModifier(token->sval))
            break;

        if (S_EQ(token->sval, "signed"))
            dtype->flags |= DATATYPE_FLAG_IS_SIGNED;

        else if(S_EQ(token->sval, "unsigned"))
            dtype->flags |= ~DATATYPE_FLAG_IS_SIGNED;

        else if(S_EQ(token->sval, "static"))
            dtype->flags |= DATATYPE_FLAG_IS_STATIC;

        else if(S_EQ(token->sval, "const"))
            dtype->flags |= DATATYPE_FLAG_IS_CONST;

        else if(S_EQ(token->sval, "extern"))
            dtype->flags |= DATATYPE_FLAG_IS_EXTERN;

        else if(S_EQ(token->sval, "__ignore_typecheck__"))
            dtype->flags |= DATATYPE_FLAG_IGNORE_TYPE_CHECKING;

        tokenNext();
        token = tokenPeekNext();
    }
}


void parserGetDatatypeTokens(struct token** datatypeToken, struct token** datatypeSecondaryToken) {
    *datatypeToken = tokenNext();
    struct token* nextToken = tokenPeekNext();

    if(tokenIsPrimitiveKeyword(nextToken)) {
        *datatypeSecondaryToken = nextToken;
        tokenNext();
    }
}


int parserDatatypeExpectedForTypeString(const char* str) {
    int type = DATATYPE_EXPECT_PRIMITIVE;

    if (S_EQ(str, "union")) 
        type = DATATYPE_EXPECT_UNION;

    else if(S_EQ(str, "struct"))
        type = DATATYPE_EXPECT_STRUCT;



    return type;
}


int parserGetRandomTypeIndex() {
    static int x = 0;
    x++;
    return x;
}


struct token* parserBuildRandomTypename() {
    char tmpName[25];
    sprintf(tmpName, "customtypename_%i", parserGetRandomTypeIndex());

    char* sval = malloc(sizeof(tmpName));
    strncpy(sval, tmpName, sizeof(tmpName));

    struct token* token = calloc(1, sizeof(struct token));
    token->type = TOKEN_TYPE_IDENTIFIER;
    token->sval = sval;

    return token;
}


int parserGetPointerDepth() {
    int depth = 0;

    while(tokenNextIsOperator("*")) {
        depth++;
        tokenNext();
    }

    return depth;
}


bool parserDatatypeIsSecondaryAllowed(int expectedType) {
    return expectedType ==  DATATYPE_EXPECT_PRIMITIVE;
}


bool parserDatatypeIsSecondaryAllowedForType(const char* type) {
    return 
    S_EQ(type, "long") || 
    S_EQ(type, "short") || 
    S_EQ(type, "double") || 
    S_EQ(type, "float");
}


void parserDatatypeInitTypeAndSizeForPrimitive(
    struct token* datatypeToken,
    struct token* datatypeSecondaryToken,
    struct datatype* datatypeOut
) {
    if (!parserDatatypeIsSecondaryAllowedForType(datatypeToken->sval) && datatypeSecondaryToken) 
        compilerError(currentProcess, "your not allowed a secondary datatype here for the given datatype %s\n", datatypeToken->sval);

    if(S_EQ(datatypeToken->sval, "void")) {
        datatypeOut->type = DATATYPE_VOID;
        datatypeOut->size = DATASIZE_ZERO;
    }

    else if(S_EQ(datatypeToken->sval, "char")) {
        datatypeOut->type = DATATYPE_CHAR;
        datatypeOut->size = DATASIZE_BYTE;
    }

    else if(S_EQ(datatypeToken->sval, "short")) {
        datatypeOut->type = DATATYPE_SHORT;
        datatypeOut->size = DATASIZE_WORD;
    }

    else if(S_EQ(datatypeToken->sval, "int")) {
        datatypeOut->type = DATATYPE_INTEGER;
        datatypeOut->size = DATASIZE_DWORD;
    }

    else if(S_EQ(datatypeToken->sval, "long")) {
        datatypeOut->type = DATATYPE_LONG;
        datatypeOut->size = DATASIZE_DWORD;
    }

    else if(S_EQ(datatypeToken->sval, "float")) {
        datatypeOut->type = DATATYPE_FLOAT;
        datatypeOut->size = DATASIZE_DWORD;
    }

    else if(S_EQ(datatypeToken->sval, "double")) {
        datatypeOut->type = DATATYPE_DOUBLE;
        datatypeOut->size = DATASIZE_DWORD;
    }

    else {
        compilerError(currentProcess, "BUG: Invalid primitive datatype\n");
    }

    parserDatatypeAdjustForSecondary(datatypeOut, datatypeSecondaryToken);
}


void parserDatatypeAdjustForSecondary(struct datatype* datatype, struct token* datatypeSecondaryToken) {
    if(!datatypeSecondaryToken)
        return;

    struct datatype* secondaryDatatype = calloc(1, sizeof(struct datatype));
    parserDatatypeInitTypeAndSizeForPrimitive(datatypeSecondaryToken, NULL, secondaryDatatype);
    datatype->size += secondaryDatatype->size;
    datatype->secondary = secondaryDatatype;
    datatype->flags |= DATATYPE_FLAG_IS_SECONDARY;
}


void parserDatatypeInitTypeAndSize(
    struct token* datatypeToken, 
    struct token* datatypeSecondaryToken,
    struct datatype* datatypeOut,
    int pointerDepth,
    int expectedType
) {
    if (!parserDatatypeIsSecondaryAllowed(expectedType) && datatypeSecondaryToken) {
        compilerError(currentProcess, "You provided an invalid secondary datatype\n");
    }

    switch(expectedType) {
        case DATATYPE_EXPECT_PRIMITIVE:
            parserDatatypeInitTypeAndSizeForPrimitive(
                datatypeToken,
                datatypeSecondaryToken,
                datatypeOut);
        break;

        case DATATYPE_EXPECT_STRUCT:
        case DATATYPE_EXPECT_UNION:
            compilerError(currentProcess, "Struct and union types are currently unsupported\n");
        break;

        default:
            compilerError(currentProcess, "BUG: unsupported datatype expectation\n");
    }
}


void parserDatatypeInit(
    struct token* datatypeToken, 
    struct token* datatypeSecondaryToken,
    struct datatype* datatypeOut,
    int pointerDepth,
    int expectedType
) {
    parserDatatypeInitTypeAndSize(
        datatypeToken, 
        datatypeSecondaryToken, 
        datatypeOut, 
        pointerDepth,
        expectedType);
    
    datatypeOut->typeStr = datatypeToken->sval;

    if(
        S_EQ(datatypeToken->sval, "long") && 
        datatypeSecondaryToken && 
        S_EQ(datatypeToken->sval, "long")
    ){
        compilerWarning(currentProcess, "Our compiler does not support 64 bits longs, therefore, your long long is defaulting to 32 bits\n");
        datatypeOut->size = DATASIZE_DWORD;
    }
}


void parseDatatypeType(struct datatype* dtype) {
    struct token* datatypeToken = NULL;
    struct token* datatypeSecondaryToken = NULL;

    parserGetDatatypeTokens(&datatypeToken, &datatypeSecondaryToken);

    int expectedType = parserDatatypeExpectedForTypeString(datatypeToken->sval);

    if (datatypeIsStructOrUNionForName(datatypeToken->sval)) {
        if (tokenPeekNext()->type == TOKEN_TYPE_IDENTIFIER) {
            datatypeToken = tokenNext();

        } else {
            // this struct has no name
            datatypeToken = parserBuildRandomTypename();
            dtype->flags |= DATATYPE_FLAG_STRUCT_UNION_NO_NAME;
        }
    }

    int pointerDepth = parserGetPointerDepth();
    parserDatatypeInit(
        datatypeToken, 
        datatypeSecondaryToken, 
        dtype, 
        pointerDepth,
        expectedType
    );
}


void parseDatatype(struct datatype *dtype) {
    memset(dtype, 0, sizeof(struct datatype));

    // all varaibles by default are signed by default
    dtype->flags |= DATATYPE_FLAG_IS_SIGNED;

    parseDatatypeModifiers(dtype);
    parseDatatypeType(dtype);
    parseDatatypeModifiers(dtype);
}


void parseVariableFunctionOrStructUnion(struct history* history) {
    struct datatype dtype;
    parseDatatype(&dtype);
}


void parseKeyword(struct history* history) {
    struct token* token = tokenPeekNext();

    if(isKeywordVariableModifier(token->sval) ||keywordIsDatatype(token->sval)) {
        parseVariableFunctionOrStructUnion(history);
        return;
    }


}


int parseExpressionalbleSingle(struct history* history) {
    struct token* token = tokenPeekNext();
    if (!token)
        return -1;

    history->flags |= NODE_FLAG_INSIDE_EXPRESSION;
    int res = -1;

    switch(token->type) {
        case TOKEN_TYPE_NUMBER:
            parseSingleTokenToNode();
            res = 0;
        break;

        case TOKEN_TYPE_IDENTIFIER:
            parseIdentifier(history);
        break;


        case TOKEN_TYPE_KEYWORD:
            parseKeyword(history);
            res = 0;
            break;


        case TOKEN_TYPE_OPERATOR:
            parseExp(history);
            res = 0;
        break;
    }

    return res;
}


void parseExpressionable(struct history* history) {
    while(parseExpressionalbleSingle(history) == 0) {

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
            parseExpressionable(historyBegin(0));
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