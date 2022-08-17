#ifndef __DAT_H__
#define __DAT_H__

#include "typedefs.h"

Trie *create_trie(long int datSize, long int tailSize);
void trie_addNeedle(Trie *trie, TrieNeedle *needle);
void trie_find(Trie *trie, TrieNeedle *needle);
TrieIndex trie_findLastFilled(Trie *trie);

void trie_print(Trie *trie);

#endif