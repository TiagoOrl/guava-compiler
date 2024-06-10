
#include "compiler.h"

int compileFile(const char * filename, const char * outFilename, int flags) {
    compileProcess * process = compileProcessCreate(filename, outFilename, flags);

    if (!process) 
        return COMPILER_FAILED_WITH_ERRORS;

    // preform lexical analysis
    //

    // preform parsing
    //

    // preform code generation
    //

    return COMPILER_FILE_COMPILED_OK;
}