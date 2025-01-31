#include "compiler.h"

bool tokenIsKeyword(struct token * token, const char * value) {
    return token->type == TOKEN_TYPE_KEYWORD && S_EQ(token->sval, value);
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