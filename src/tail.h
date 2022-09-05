#ifndef TAIL_H
#define TAIL_H

#include "defs.h"

TailBuilder *createTailBuilder(TailIndex size);
void tailBuilder_free(TailBuilder *tailBuilder);
void tailBuilder_freeCharacters(TailBuilder *tailBuilder);
void tailBuilder_freeCell(TailBuilder *tailBuilder, TailIndex index);
void tailBuilder_minimize(TailBuilder *tailBuilder);
TailIndex tailBuilder_insertChars(TailBuilder *tailBuilder, TailCharIndex length, Character *string);

Tail *createTail(TailIndex size);
Tail *createTailFromBuilder(TailBuilder *tailBuilder);
TailCell tail_getCell(const Tail *tail, TailIndex index);
void tail_free(Tail *tail);

Character *allocateCharacters(TailCharIndex size);

#endif
