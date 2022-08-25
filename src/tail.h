#ifndef TAIL_H
#define TAIL_H

#include "typedefs.h"

Tail *createTail(TailIndex size);
void tail_free(Tail *tail);
void tail_freeCell(Tail *tail, TailIndex index);
void tail_minimize(Tail *tail);
TailIndex tail_insertChars(Tail *tail, TailCharIndex length, TrieChar *string);
TailCell tail_getCell(const Tail *tail, TailIndex index);
TrieChar *allocateTrieChars(TailCharIndex size);

#endif
