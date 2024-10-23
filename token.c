#include "compiler.h"

bool tokenIsKeyword(struct token * token, const char * value) {
    return token->type == TOKEN_TYPE_KEYWORD && S_EQ(token->sval, value);
}