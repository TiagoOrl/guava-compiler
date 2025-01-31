#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>


struct vector* nodeVector = NULL;
struct vector* nodeVectorRoot = NULL;


void nodeSetVector(struct vector* vec, struct vector* rootVec) {
    nodeVector = vec;
    nodeVectorRoot = rootVec;
}


void nodePush(struct node* node) {
    vector_push(nodeVector, &node);
}

struct node* nodePeekOrNull() {
    return vector_back_ptr_or_null(nodeVector);
}


struct node* nodePeek() {
    return *(struct node**)(vector_back(nodeVector));
}


struct node* nodePop() {
    struct node* lastNode = vector_back_ptr(nodeVector);
    struct node* lastNodeRoot = vector_empty(nodeVector) ? 
        NULL : vector_back_ptr_or_null(nodeVectorRoot);

    vector_pop(nodeVector);

    if (lastNode == lastNodeRoot) 
        vector_pop(nodeVectorRoot);
    

    return lastNode;
}


bool nodeIsExpressionable(struct node* node) {
    return 
        node->type == NODE_TYPE_EXPRESSION || 
        node->type == NODE_TYPE_EXPRESSION_PARENTHESES ||
        node->type == NODE_TYPE_UNARY ||
        node->type == NODE_TYPE_IDENTIFIER ||
        node->type == NODE_TYPE_NUMBER ||
        node->type == NODE_TYPE_STRING;
}


struct node* nodePeekExpressionableOrNull() {
    struct node* lastNode = nodePeekOrNull();

    return nodeIsExpressionable(lastNode) ? lastNode : NULL;
}


void makeExpNode(struct node* leftNode, struct node* rightNode, const char* op) {
    assert(leftNode);
    assert(rightNode);

    nodeCreate(&(struct node){
        .type = NODE_TYPE_EXPRESSION,
        .exp.left = leftNode,
        .exp.right = rightNode,
        .exp.op = op
    });
}


struct node* nodeCreate(struct node* _node) {
    struct node* node = malloc(sizeof(struct node));
    memcpy(node, _node, sizeof(struct node));

    #warning "we should set the binded owner and the binded funciton here"
    nodePush(node);
    return node;
}