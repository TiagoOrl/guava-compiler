
#include "compiler.h"
#include <stdarg.h>
#include <stdlib.h>

struct lexProcessFunctions compilerLexFunctions = {
    .nextChar = compileProcessNextChar,
    .peekChar = compileProcessPeekChar,
    .pushChar = compileProcessPushChar
};

void compilerError(struct compileProcess * compiler, const char * msg, ...) {
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, " on line %i, col %i in file %s\n", 
        compiler->pos.line,
        compiler->pos.col,
        compiler->pos.filename
        );

    exit(-1);
}


void compilerWarning(struct compileProcess * compiler, const char * msg, ...) {
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, " on line %i, col %i in file %s\n", 
        compiler->pos.line,
        compiler->pos.col,
        compiler->pos.filename
        );
}


int compileFile(const char * filename, const char * outFilename, int flags) {
    struct compileProcess * process = compileProcessCreate(filename, outFilename, flags);

    if (!process) 
        return COMPILER_FAILED_WITH_ERRORS;

    // preform lexical analysis
    struct lexProcess * lexProcess = lexProcessCreate(process, &compilerLexFunctions, NULL);

    if (!lexProcess) {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    if (lex(lexProcess) != LEXICAL_ANALYSIS_ALL_OK) {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    //

    // preform parsing
    //

    // preform code generation
    //

    return COMPILER_FILE_COMPILED_OK;
}