#ifndef __TAIL_H__
#define __TAIL_H__

#include "dat.h"

Tail *create_tail(long int size);
void tail_freeCell(Tail *tail, TailIndex index);
TailIndex tail_insertChars(Tail *tail, int length, TrieChar *string);
void tail_print(Tail *tail);

#endif
