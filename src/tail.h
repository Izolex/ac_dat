#ifndef __TAIL_H__
#define __TAIL_H__

#include "typedefs.h"

Tail *create_tail(TailIndex size);
void tail_freeCell(Tail *tail, TailIndex index);
TailIndex tail_insertChars(Tail *tail, size_t length, TrieChar *string);
void tail_print(Tail *tail);

TailCell tail_getCell(const Tail *tail, TailIndex index);

#endif
