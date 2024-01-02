#include "sakura.h"

#include <stdlib.h>

#include "assembler.h"

unsigned int hash(const char *key, size_t len, size_t capacity) {
    unsigned int hashValue = 5381;
    for (size_t i = 0; i < len; i++)
        hashValue = (hashValue << 5) + hashValue + key[i];
    return hashValue % capacity;
}

SakuraState *sakura_createState() {
    SakuraState *state = malloc(sizeof(SakuraState));
    if (state != NULL) {
        state->stackIndex = 0;
        state->currentState = SAKURA_FLAG_ENDED;
        state->error = SAKURA_EFLAG_NONE;

        state->pool.size = 0;
        state->pool.capacity = 8;
        state->pool.constants = malloc(state->pool.capacity * sizeof(TValue));

        state->callStack = malloc(128 * sizeof(int));
        state->callStackSize = 128;
        state->callStackIndex = 0;

        state->locals = malloc(128 * sizeof(struct s_str));
        state->localsSize = 128;

        for (size_t i = 0; i < state->localsSize; i++) {
            state->locals[i].str = NULL;
            state->locals[i].len = 0;
        }

        sakuraX_initializeTVMap(&state->globals, 16);

        // initialize registry
        state->registry.rax.tt = SAKURA_TNUMFLT;
        state->registry.rax.value.n = 0;
        state->registry.rbx.tt = SAKURA_TNUMFLT;
        state->registry.rbx.value.n = 0;
        state->registry.rcx.tt = SAKURA_TNUMFLT;
        state->registry.rcx.value.n = 0;
        state->registry.rdx.tt = SAKURA_TNUMFLT;
        state->registry.rdx.value.n = 0;

        for (size_t i = 0; i < 64; i++) {
            state->registry.args[i].tt = SAKURA_TNUMFLT;
            state->registry.args[i].value.n = 0;
        }

        state->internalOffset = 0;
    }

    return state;
}

void sakura_destroyState(SakuraState *state) {
    if (state != NULL) {
        sakuraX_destroyTVMap(&state->globals);
        for (size_t i = 0; i < state->pool.size; i++) {
            if (state->pool.constants[i].tt == SAKURA_TSTR)
                s_str_free(&state->pool.constants[i].value.s);
        }
        free(state->pool.constants);
        state->pool.constants = NULL;
        state->pool.size = 0;
        free(state->callStack);
        state->callStack = NULL;
        state->callStackSize = 0;
        state->callStackIndex = 0;
        for (size_t i = 0; i < state->localsSize; i++)
            s_str_free(&state->locals[i]);
        free(state->locals);
        state->locals = NULL;
        free(state);
    }
}

void sakuraDEBUG_dumpStack(SakuraState *S) {
    printf("Stack dump (%p-%d):\n", S, S->stackIndex);
    for (int i = 0; i < S->stackIndex; i++) {
        printf("  [%d] ", i);
        if (S->stack[i].tt == SAKURA_TNUMFLT) {
            printf("%f\n", S->stack[i].value.n);
        } else if (S->stack[i].tt == SAKURA_TSTR) {
            printf("%.*s\n", S->stack[i].value.s.len, S->stack[i].value.s.str);
        } else if (S->stack[i].tt == SAKURA_TCFUNC) {
            printf("[CFunc %p]\n", S->stack[i].value.cfn);
        } else if (S->stack[i].tt == SAKURA_TFUNC) {
            printf("[SakuraFunc '<NIL>']\n");
        } else {
            printf("[Unknown]\n");
        }
    }
}

void sakuraX_initializeTVMap(struct TVMap *map, size_t initCapacity) {
    map->pairs = malloc(initCapacity * sizeof(struct TVMapPair));
    map->size = 0;
    map->capacity = initCapacity;

    for (size_t i = 0; i < initCapacity; i++) {
        map->pairs[i].init = 0;
        map->pairs[i].key.str = NULL;
        map->pairs[i].key.len = 0;
    }
}

void sakuraX_resizeTVMap(struct TVMap *map, size_t newCapacity) {
    struct TVMapPair *oldPairs = map->pairs;
    size_t oldCapacity = map->capacity;

    map->pairs = malloc(newCapacity * sizeof(struct TVMapPair));
    map->size = 0;
    map->capacity = newCapacity;

    for (size_t i = 0; i < newCapacity; i++) {
        map->pairs[i].init = 0;
        map->pairs[i].key.str = NULL;
        map->pairs[i].key.len = 0;
    }

    for (size_t i = 0; i < oldCapacity; i++) {
        if (oldPairs[i].init == 1) {
            sakuraX_TVMapInsert(map, &oldPairs[i].key, oldPairs[i].value);
            s_str_free(&oldPairs[i].key); // Free the old key (assuming ownership transfer)
        }
    }

    free(oldPairs);
}

void sakuraX_TVMapInsert(struct TVMap *map, const struct s_str *key, TValue value) {
    if ((double)map->size / map->capacity >= 0.5) {
        sakuraX_resizeTVMap(map, map->capacity * 2);
    }

    size_t idx = hash(key->str, key->len, map->capacity);
    map->pairs[idx].key = s_str_copy(key);
    map->pairs[idx].value = value;
    map->pairs[idx].init = 1;
    map->size++;
}

int sakuraX_TVMapGetIndex(struct TVMap *map, const struct s_str *key) {
    size_t idx = hash(key->str, key->len, map->capacity);
    return map->pairs[idx].init == 1 ? idx : -1;
}

TValue *sakuraX_TVMapGet(struct TVMap *map, const struct s_str *key) {
    size_t idx = hash(key->str, key->len, map->capacity);
    return map->pairs[idx].init == 1 ? &map->pairs[idx].value : NULL;
}

TValue *sakuraX_TVMapGet_c(struct TVMap *map, const char *key) {
    size_t idx = hash(key, strlen(key), map->capacity);
    return map->pairs[idx].init == 1 ? &map->pairs[idx].value : NULL;
}

void sakuraX_destroyTVMap(struct TVMap *map) {
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->pairs[i].init == 0)
            continue;
        s_str_free(&map->pairs[i].key);
        if (map->pairs[i].value.tt == SAKURA_TSTR) {
            s_str_free(&map->pairs[i].value.value.s);
        }
    }
    free(map->pairs);
    map->pairs = NULL;
    map->size = 0;
    map->capacity = 0;
}

void sakuraY_attemptFreeTValue(TValue *val) {
    if (val->tt == SAKURA_TFUNC) {
        sakuraX_freeAssembly(val->value.assembly);
    }
}

TValue sakuraY_makeTNumber(double value) {
    TValue val;
    val.tt = SAKURA_TNUMFLT;
    val.value.n = value;
    return val;
}

TValue sakuraY_makeTString(struct s_str *value) {
    TValue val;
    val.tt = SAKURA_TSTR;
    val.value.s = s_str_copy(value);
    return val;
}

TValue sakuraY_makeTCFunc(int (*fnPtr)(SakuraState *)) {
    TValue val;
    val.tt = SAKURA_TCFUNC;
    val.value.cfn = fnPtr;
    return val;
}

TValue sakuraY_makeTFunc(struct SakuraAssembly *assembly) {
    TValue val;
    val.tt = SAKURA_TFUNC;
    val.value.assembly = assembly;
    return val;
}

void sakura_setGlobal(SakuraState *S, const struct s_str *name) {
    TValue *val = sakuraX_TVMapGet(&S->globals, name);
    if (val == NULL) {
        sakuraX_TVMapInsert(&S->globals, name, sakuraY_pop(S));
    } else {
        *val = sakuraY_pop(S);
    }
}

void sakuraY_push(SakuraState *S, TValue val) {
    if (S->stackIndex >= SAKURA_STACK_SIZE) {
        printf("Error: stack overflow\n");
        exit(1);
    }
    S->stack[S->stackIndex++] = val;
}

TValue sakuraY_pop(SakuraState *S) {
    if (S->stackIndex <= 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }
    return S->stack[--S->stackIndex];
}

// make a function to pop a specific index and shift everything down
TValue sakuraY_popN(SakuraState *S, int n) {
    if (S->stackIndex <= n) {
        printf("Error: stack underflow\n");
        exit(1);
    }

    TValue val = S->stack[n];

    for (int i = n; i < S->stackIndex - 1; i++)
        S->stack[i] = S->stack[i + 1];
    S->stackIndex--; // this will allow the top element to be overwritten (it is duplicated above)

    return val;
}

TValue *sakuraY_peek(SakuraState *S) {
    if (S->stackIndex <= 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }

    return &(S->stack[S->stackIndex - 1]);
}

int sakura_peek(SakuraState *S) {
    if (S->stackIndex <= 0) {
        printf("Error: stack underflow\n");
        exit(1);
    }

    return (int)(S->stack[S->stackIndex - 1].value.n);
}

int sakura_isNumber(SakuraState *S) { return sakuraY_peek(S)->tt == SAKURA_TNUMFLT; }
int sakura_isString(SakuraState *S) { return sakuraY_peek(S)->tt == SAKURA_TSTR; }

double sakura_popNumber(SakuraState *S) {
    TValue val = sakuraY_pop(S);
    if (val.tt != SAKURA_TNUMFLT) {
        printf("Error: expected number, got %d\n", val.tt);
        exit(1);
    }
    return val.value.n;
}

struct s_str sakura_popString(SakuraState *S) {
    TValue val = sakuraY_pop(S);
    if (val.tt != SAKURA_TSTR) {
        printf("Error: expected string, got %d\n", val.tt);
        exit(1);
    }
    return val.value.s;
}

void sakuraY_storeLocal(SakuraState *S, const struct s_str *name, int idx) {
    if (idx >= S->localsSize) {
        S->locals = realloc(S->locals, (idx + 1) * sizeof(struct s_str));

        for (size_t i = S->localsSize; i < idx + 1; i++) {
            S->locals[i].str = NULL;
            S->locals[i].len = 0;
        }

        S->localsSize = idx + 1;
    }

    S->locals[idx] = s_str_copy(name);
}

void copyTValue(TValue *dest, TValue *src) {
    dest->tt = src->tt;
    if (src->tt == SAKURA_TNUMFLT) {
        dest->value.n = src->value.n;
    } else if (src->tt == SAKURA_TSTR) {
        dest->value.s = s_str_copy(&src->value.s);
    } else {
        // Handle other types as needed
    }
}

void sakuraY_mergePools(SakuraState *S, SakuraConstantPool *pool) {
    S->pool.capacity += pool->capacity;
    S->pool.constants = realloc(S->pool.constants, S->pool.capacity * sizeof(TValue));
    for (size_t i = 0; i < pool->size; i++)
        copyTValue(&S->pool.constants[S->pool.size + i], &pool->constants[i]);
    S->pool.size += pool->size;
}

void sakuraY_mergePoolsA(SakuraConstantPool *into, SakuraConstantPool *from) {
    into->capacity += from->capacity;
    into->constants = realloc(into->constants, into->capacity * sizeof(TValue));
    for (size_t i = 0; i < from->size; i++)
        copyTValue(&into->constants[into->size + i], &from->constants[i]);
    into->size += from->size;
}