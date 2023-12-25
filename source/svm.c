#include "svm.h"

#include "sakura.h"

void sakuraX_interpret(SakuraState *S, struct SakuraAssembly *assembly) {
    S->currentState = SAKURA_FLAG_RUNTIME;

    sakuraY_mergePools(S, &assembly->pool);
    S->stackIndex = assembly->registers;

    int *instructions = assembly->instructions;
    for (size_t i = 0; i < assembly->size; i++) {
        int reg = -1;
        switch (instructions[i]) {
        case SAKURA_LOADK:
            S->stack[instructions[i + 1]] = assembly->pool.constants[-instructions[i + 2] - 1];
            i += 2;
            break;
        case SAKURA_ADD: {
            reg = instructions[i + 1];
            TValue val = sakuraY_getStack(S, instructions[i + 2]);
            TValue val2 = sakuraY_getStack(S, instructions[i + 3]);
            if (val.tt == SAKURA_TNUMFLT) {
                if (val2.tt == SAKURA_TNUMFLT) {
                    S->stack[reg] = sakuraY_makeTNumber(val2.value.n + val.value.n);
                } else if (val2.tt == SAKURA_TSTR) {
                    struct s_str v = s_str_concat_d(&val2.value.s, val.value.n);
                    S->stack[reg] = sakuraY_makeTString(&v);
                    s_str_free(&v);
                } else {
                    printf("Error: unknown addition operands\n");
                }
            } else if (val.tt == SAKURA_TSTR) {
                if (val2.tt == SAKURA_TNUMFLT) {
                    struct s_str v = s_str_concat_dd(val2.value.n, &val.value.s);
                    S->stack[reg] = sakuraY_makeTString(&v);
                    s_str_free(&v);
                } else if (val2.tt == SAKURA_TSTR) {
                    struct s_str v = s_str_concat(&val2.value.s, &val.value.s);
                    S->stack[reg] = sakuraY_makeTString(&v);
                    s_str_free(&v);
                } else {
                    printf("Error: unknown addition operands\n");
                }
            } else {
                printf("Error: what the frick is this\ntry again.\n");
            }
            i += 3;
            break;
        }
        case SAKURA_MUL: {
            reg = instructions[i + 1];
            TValue val = sakuraY_getStack(S, instructions[i + 2]);
            TValue val2 = sakuraY_getStack(S, instructions[i + 3]);
            if (val.tt == SAKURA_TNUMFLT) {
                if (val2.tt == SAKURA_TNUMFLT) {
                    S->stack[reg] = sakuraY_makeTNumber(val2.value.n * val.value.n);
                } else {
                    printf("Error: unknown multiplication operands: %d %d\n", val.tt, val2.tt);
                }
            } else {
                printf("Error: unknown multiplication operands: %d\n", val.tt);
            }
            i += 3;
            break;
        }
        default:
            printf("Error: unknown/unimplemented runtime instruction '%d'\n", instructions[i]);
            break;
        }
    }
}