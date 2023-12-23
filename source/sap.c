#include "sap.h"

#include "filesystem.h"
#include "parser.h"

void sakuraL_loadfile(SakuraState *S, const char *file) {
    struct s_str source = readfile(file);
    if (source.str == NULL) {
        printf("Error: could not read file %s\n", file);
        return;
    }

    sakuraL_loadstring(S, &source);

    s_str_free(&source);
}

void sakuraL_loadstring(SakuraState *S, struct s_str *source) {
    struct TokenStack *tokens = sakuraY_analyze(S, source);
    struct NodeStack *nodes = sakuraY_parse(S, tokens);

    // vm operations

    sakuraX_freeNodeStack(nodes);
    sakuraX_freeTokStack(tokens);
}

void sakuraL_loadstring_c(SakuraState *S, const char *str) {
    struct s_str source = s_str(str);
    sakuraL_loadstring(S, &source);
    s_str_free(&source);
}