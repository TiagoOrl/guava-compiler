
#include "compiler.h"

struct lexProcessFunctions compilerLexFunctions = {
    .nextChar = compileProcessNextChar,
    .peekChar = compileProcessPeekChar,
    .pushChar = compileProcessPushChar
};


int compileFile(const char * filename, const char * outFilename, int flags) {
    struct compileProcess * process = compileProcessCreate(filename, outFilename, flags);

    if (!process) 
        return COMPILER_FAILED_WITH_ERRORS;

    // preform lexical analysis
    struct lexProcess * lexProcess = lexProcessCreate(process, &compilerLexFunctions, NULL);

    if (!lexProcess) {
        return NULL;
    }

    if (lex(lexProcess) != LEXICAL_ANALYSIS_ALL_OK) {
        return NULL;
    }

    //

    // preform parsing
    //

    // preform code generation
    //

    return COMPILER_FILE_COMPILED_OK;
}