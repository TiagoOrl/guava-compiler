#include "compiler.h"


/**
 * Format: {operator1, operator2, operator3,..., NULL}
 */
struct expressionableOpPrecedenceGroup opPrecedence[TOTAL_OPERATOR_GROUPS] = {
    {.operators = {"++", "--", "()", "[]", "(", "[", ".", "->",  NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"*", "/", "%",  NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"+", "-", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"<<", ">>", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"<", "<=", ">", ">=", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"==", "!=", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"&", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"^", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"|", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"&&", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"||", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT},
    {.operators = {"?", ":", NULL}, .associativity = ASSOCIATIVITY_RIGHT_TO_LEFT},
    {.operators = {"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|=",  NULL}, .associativity = ASSOCIATIVITY_RIGHT_TO_LEFT},
    {.operators = {",", NULL}, .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT}
};