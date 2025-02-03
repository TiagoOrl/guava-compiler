#include "compiler.h"

#define PRIMITIVE_TYPES_TOTAL 7
const char* primitiveTypes[PRIMITIVE_TYPES_TOTAL] = {
    "void", "char", "short", "int", "long", "float", "double"
};

bool tokenIsKeyword(struct token * token, const char * value) {
    return token->type == TOKEN_TYPE_KEYWORD && S_EQ(token->sval, value);
}


bool tokenIsOperator(struct token* token, const char* val) {
    return token->type == TOKEN_TYPE_OPERATOR && S_EQ(token->sval, val);
}


bool tokenIsSymbol(struct token* token, char c) {
    return token->type == TOKEN_TYPE_SYMBOL && token->cval == c;
}


bool tokenIsNlOrCommentOrNewlineSeparator(struct token* token) {
    return 
        token->type == TOKEN_TYPE_NEWLINE || 
        token->type == TOKEN_TYPE_COMMENT ||
        tokenIsSymbol(token, '\\');

}


bool tokenIsPrimitiveKeyword(struct token* token) {
    if (token->type != TOKEN_TYPE_KEYWORD) 
        return false;

    for (int i = 0; i < PRIMITIVE_TYPES_TOTAL; i++)
    {
        if (S_EQ(primitiveTypes[i], token->sval))
            return true;
    }

    return false;
    
}