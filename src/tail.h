#ifndef TAIL_H
#define TAIL_H

#include "../include/tail.h"
#include "needle.h"

typedef int32_t TailIndex;
typedef u_int32_t TailCharIndex;

typedef struct {
    Character *chars;
    TailCharIndex length;
} TailCell;

typedef struct tail {
    TailIndex size;
    TailCell *cells;
} Tail;

typedef struct {
    Character *chars;
    TailCharIndex length;
    TailIndex nextFree;
} TailBuilderCell;

typedef struct tailBuilder {
    TailBuilderCell *cells;
    TailIndex size;
} TailBuilder;


void tailBuilder_freeCharacters(TailBuilder *tailBuilder);
void tailBuilder_freeCell(TailBuilder *tailBuilder, TailIndex index);
void tailBuilder_minimize(TailBuilder *tailBuilder);
TailIndex tailBuilder_insertChars(TailBuilder *tailBuilder, TailCharIndex length, Character *string);

TailCell tail_getCell(const Tail *tail, TailIndex index);

Character *allocateCharacters(TailCharIndex size);

#endif
