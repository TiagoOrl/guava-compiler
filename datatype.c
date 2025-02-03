#include "compiler.h"


bool datatypeIsStructOrUNionForName(const char* name) {
    return S_EQ(name, "union") || S_EQ(name, "struct");
}