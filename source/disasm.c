#include "disasm.h"

#include <stdlib.h>

char *sakuraX_readTVal(TValue *val) {
    char *allocVal = malloc(1024);
    switch (val->tt) {
    case SAKURA_TNUMFLT:
        sprintf(allocVal, "%f", val->value.n);
        break;
    case SAKURA_TSTR:
        sprintf(allocVal, "%.*s", val->value.s.len, val->value.s.str);
        break;
    }

    return allocVal;
}

void sakuraX_writeDisasm(SakuraState *S, struct SakuraAssembly *assembler, const char *filename) {
    printf("main <%s:0,0> (%d instructions, %d bytes)\n", filename, assembler->size, assembler->size * sizeof(int));
    printf("%d registers, %d variables, %d constants, %d functions\n", assembler->registers, 0, assembler->pool.size,
           0);

    struct s_str **cachedGlobals = malloc(sizeof(struct s_str *) * S->globals.size);

    size_t idx = 1;
    for (size_t i = 0; i < assembler->size; i++) {
        switch (assembler->instructions[i]) {
        case SAKURA_LOADK: {
            char *allocVal = sakuraX_readTVal(&assembler->pool.constants[-assembler->instructions[i + 2] - 1]);
            printf("    %d\t(%d)\t\tLOADK\t\t%d\t\t;; %s into stack pos %d\n", idx, i, assembler->instructions[i + 2],
                   allocVal, assembler->instructions[i + 1]);
            free(allocVal);
            i += 2;
            break;
        }
        case SAKURA_ADD: {
            printf("    %d\t(%d)\t\tADD\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_MUL: {
            printf("    %d\t(%d)\t\tMUL\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_CALL: {
            struct s_str *key = cachedGlobals[assembler->instructions[i + 1]];
            printf("    %d\t(%d)\t\tCALL\t\t%d, %d\t\t;; %.*s(%d args...)\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], key->len, key->str, assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_GETGLOBAL: {
            struct s_str *key = &S->globals.pairs[assembler->instructions[i + 2]].key;
            cachedGlobals[assembler->instructions[i + 1]] = key;
            printf("    %d\t(%d)\t\tGETGLOBAL\t%d\t\t;; store '%.*s' into stack pos %d\n", idx, i,
                   assembler->instructions[i + 2], key->len, key->str, assembler->instructions[i + 1]);
            i += 2;
            break;
        }
        }

        idx++;
    }

    free(cachedGlobals);

    printf("Raw Instructions:\n");
    for (size_t i = 0; i < assembler->size; i++) {
        printf("%d ", assembler->instructions[i]);
    }
    printf("\n");
}