#pragma once

#include <stdint.h>
#include <string.h>

#include "parser.h"
#include "sakura.h"

// constant pool structure
struct SakuraAssembly {
    int *instructions;
    SakuraConstantPool pool;
    ull size;
    ull capacity;
    ull registers;

    struct SakuraAssembly **closures;
    ull closureCapacity;
    ull closureIdx;

    ull highestRegister;
    ull functionsLoaded;
};

// assembly instructions

// Stack Manipulation
#define SAKURA_MOVE 0    // mov a, b -> moves the value of b to a
#define SAKURA_LOADK 1   // loadk a -> loads the constant at index a into the stack
#define SAKURA_LOADNIL 2 // loadnil a, b -> loads nil into a, a + 1, ..., a + b
#define SAKURA_POP 17    // pop a -> pops a values from the stack

// Global Variable Operations
#define SAKURA_GETGLOBAL 3 // getglobal a -> loads the global at index a into the stack
#define SAKURA_SETGLOBAL 4 // setglobal a, b -> sets the global at index a to b

// Table Operations
#define SAKURA_NEWTABLE 33 // newtable a, b -> creates a new table with b size and stores it in a
#define SAKURA_GETTABLE 5  // gettable a, b, c -> loads the value at index b in the table at index a into c
#define SAKURA_SETTABLE 6  // settable a, b, c -> sets the value at index b in the table at index a to c

// Function/Closure Operations
#define SAKURA_CLOSURE 7 // closure a, b -> creates a closure from the function at index b and stores it in a
#define SAKURA_CALL 8    // call a, b, c -> calls function at index c, with a return values, and b arguments
#define SAKURA_RETURN 9  // return a, b -> returns from a function with a and b range of values

// Arithmetic Operations
#define SAKURA_ADD 10 // add a, b, c -> adds the values at index b and c and stores it in a
#define SAKURA_SUB 11 // sub a, b, c -> subtracts the values at index b and c and stores it in a
#define SAKURA_MUL 12 // mul a, b, c -> multiplies the values at index b and c and stores it in a
#define SAKURA_DIV 13 // div a, b, c -> divides the values at index b and c and stores it in a
#define SAKURA_MOD 14 // mod a, b, c -> mods the values at index b and c and stores it in a
#define SAKURA_POW 15 // pow a, b, c -> powers the values at index b and c and stores it in a
#define SAKURA_UNM 16 // unm a, b -> negates the value at index b and stores it in a

// Comparisonn & Logical Operations
#define SAKURA_EQ 18  // eq a, b, c -> checks if the values at index b and c are equal and stores it in a
#define SAKURA_LT 19  // lt a, b, c -> checks if index b is less than the value at index c and stores it in a
#define SAKURA_LE 20  // le a, b, c -> checks if index b is less than or equal to the value at index c and stores it in a
#define SAKURA_NOT 21 // not a, b -> inverts the bool at b and stores it in a

// Control Flow
#define SAKURA_JMP 22   // jmp a -> jumps to the instruction at index a
#define SAKURA_JMPIF 23 // jmpif a, b -> jumps to the instruction at index a if the value at index b is false

// String Manipulation
#define SAKURA_CONCAT 24 // concat a, b, c -> concatenates the values at b through c and stores in a

// Length Operations
#define SAKURA_LENSTR 25 // lenstr a, b -> gets the length of the value at index b and stores it in a
#define SAKURA_LENTBL 26 // lentbl a, b -> gets the length of the table at index b and stores it in a

// Bitwise Operations
#define SAKURA_BAND 27 // band a, b, c -> bitwise ands the values at index b and c and stores it in a
#define SAKURA_BOR 28  // bor a, b, c -> bitwise ors the values at index b and c and stores it in a
#define SAKURA_BXOR 29 // bxor a, b, c -> bitwise xors the values at index b and c and stores it in a
#define SAKURA_BNOT 30 // bnot a, b -> bitwise nots the value at index b and stores it in a
#define SAKURA_SHL 31  // shl a, b, c -> bitwise shifts index b left by the value at index c and stores it in a
#define SAKURA_SHR 32  // shr a, b, c -> bitwise shifts index b right by the value at index c and stores it in a

struct SakuraAssembly *sakuraY_assemble(SakuraState *S, struct NodeStack *nodes);

// visitor functions
void sakuraV_visitNode(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitBlock(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitUnary(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitBinary(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitCall(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitIndex(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitFunction(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitIf(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitString(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitNumber(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitIdentifier(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitWhile(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitVar(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);
void sakuraV_visitTable(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);

#define SakuraAssembly() SakuraAssembly_new(1)

struct SakuraAssembly *SakuraAssembly_new(int fullSetup);
void sakuraX_freeAssembly(struct SakuraAssembly *assembly);
void SakuraAssembly_push(struct SakuraAssembly *assembly, int instruction);
void SakuraAssembly_push2(struct SakuraAssembly *assembly, int instruction, int a);
void SakuraAssembly_push3(struct SakuraAssembly *assembly, int instruction, int a, int b);
void SakuraAssembly_push4(struct SakuraAssembly *assembly, int instruction, int a, int b, int c);

int sakuraX_pushKNumber(struct SakuraAssembly *assembly, double value);
int sakuraX_pushKString(struct SakuraAssembly *assembly, const struct s_str *value);

void SakuraAssembly_pushChildAssembly(struct SakuraAssembly *assembly, struct SakuraAssembly *child);