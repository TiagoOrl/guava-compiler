#include "compiler.h"
#include "helpers/vector.h"


static struct compileProcess * currentProcess;


int parseNext() {
    return 0;
}


int parse(struct compileProcess * process) {
    currentProcess = process;
    vector_set_peek_pointer(process->tokenVec, 0);
    struct node* node = NULL;

    while(parseNext() == 0) 
    {
        // node = node_peek();
        vector_push(process->nodeTreeVec, &node);
    }

    return PARSE_ALL_OK;
}