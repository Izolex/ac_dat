#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "unicode/utf8.h"

uint32_t u8decode(uint32_t c);
void tail_print(Tail *tail);
void trieBuilder_print(Trie *trie);

#endif