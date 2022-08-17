#ifndef __DAT_H__
#define __DAT_H__

#include "typedefs.h"

Trie *create_trie(long int datSize, long int tailSize);
void trie_addNeedle(Trie *trie, TrieNeedle *needle);
void trie_find(Trie *trie, TrieNeedle *needle);
TrieIndex trie_findLastFilled(Trie *trie);

TrieBase trie_getBase(Trie *trie, TrieIndex index);
TrieIndex trie_getCheck(Trie *trie, TrieIndex index);

TrieIndex trie_getFail(Trie *trie, TrieIndex index);
void trie_setFail(Trie *trie, TrieIndex index, TrieBase value);

TrieIndex trie_getShortcut(Trie *trie, TrieIndex index);
void trie_setShortcut(Trie *trie, TrieIndex index, TrieBase value);

void trie_print(Trie *trie);

#endif