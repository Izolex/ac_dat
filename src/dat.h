#ifndef __DAT_H__
#define __DAT_H__

#include "typedefs.h"

TrieOptions *create_TrieOptions(unsigned char useTail);
Trie *create_trie(TrieOptions *options, long int datSize, long int tailSize);
void trie_addNeedle(Trie *trie, Needle *needle);
void trie_find(Trie *trie, Needle *needle);
TrieIndex trie_findLastFilled(Trie *trie);

TrieBase trie_getBase(const Trie *trie, TrieIndex index);
TrieIndex trie_getCheck(const Trie *trie, TrieIndex index);

List *trie_getChildren(const Trie *trie, TrieIndex index);

void trie_print(Trie *trie);

#endif