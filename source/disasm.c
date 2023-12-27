#include "disasm.h"

#include <stdlib.h>

char *sakuraX_readTVal(TValue *val) {
    char *allocVal = malloc(1024 * sizeof(char));
    switch (val->tt) {
    case SAKURA_TNUMFLT:
        sprintf(allocVal, "%f", val->value.n);
        break;
    case SAKURA_TSTR:
        sprintf(allocVal, "%.*s", val->value.s.len, val->value.s.str);
        break;
    default:
        sprintf(allocVal, "[Unknown T%d D%f @ %p (%p)]", val->tt, val->value.n, val, &val->tt);
        break;
    }

    return allocVal;
}

void sakuraX_writeDisasm(SakuraState *S, struct SakuraAssembly *assembler, const char *filename) {
    printf("Raw Instructions:\n");
    for (size_t i = 0; i < assembler->size; i++) {
        printf("%d ", assembler->instructions[i]);
    }
    printf("\n");
    // printf("Constants Dump (%p through %p):\n", assembler->pool.constants,
    //        assembler->pool.constants + assembler->pool.size);
    // for (size_t i = 0; i < assembler->pool.size; i++) {
    //     char *allocVal = sakuraX_readTVal(&assembler->pool.constants[i]);
    //     printf("  [%d] %s\n", i, allocVal);
    //     free(allocVal);
    // }

    printf("main <%s:0,0> (%d instructions, %d bytes) UNOPTIMIZED\n", filename, assembler->size,
           assembler->size * sizeof(int));
    printf("%d registers, %d variables, %d constants, %d functions\n", assembler->highestRegister, 0,
           assembler->pool.size, assembler->functionsLoaded);

    struct s_str **cachedGlobals = malloc(sizeof(struct s_str *) * (assembler->functionsLoaded * 2));

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
        case SAKURA_SUB: {
            printf("    %d\t(%d)\t\tSUB\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
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
        case SAKURA_DIV: {
            printf("    %d\t(%d)\t\tDIV\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_POW: {
            printf("    %d\t(%d)\t\tPOW\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_MOD: {
            printf("    %d\t(%d)\t\tMOD\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_EQ: {
            printf("    %d\t(%d)\t\tEQ\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_LT: {
            printf("    %d\t(%d)\t\tLT\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2], assembler->instructions[i + 3]);
            i += 3;
            break;
        }
        case SAKURA_LE: {
            printf("    %d\t(%d)\t\tLE\t\t%d, %d, %d\n", idx, i, assembler->instructions[i + 1],
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
        case SAKURA_JMP: {
            printf("    %d\t(%d)\t\tJMP\t\t%d\n", idx, i, assembler->instructions[i + 1]);
            i += 1;
            break;
        }
        case SAKURA_JMPIF: {
            printf("    %d\t(%d)\t\tJMPIF\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_RETURN: {
            printf("    %d\t(%d)\t\tRETURN\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        case SAKURA_NOT: {
            printf("    %d\t(%d)\t\tNOT\t\t%d, %d\n", idx, i, assembler->instructions[i + 1],
                   assembler->instructions[i + 2]);
            i += 2;
            break;
        }
        }

        idx++;
    }

    free(cachedGlobals);

    printf("=================\n");
}