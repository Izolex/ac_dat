#ifndef __DAT_H__
#define __DAT_H__

#include "typedefs.h"

TrieOptions *create_TrieOptions(bool useTail);
Trie *create_trie(TrieOptions *options, TrieIndex datSize, TailIndex tailSize);

void trie_addNeedle(Trie *trie, const Needle *needle);
void trie_find(Trie *trie, Needle *needle);

TrieBase trie_getBase(const Trie *trie, TrieIndex index);
TrieIndex trie_getCheck(const Trie *trie, TrieIndex index);
List *trie_getChildren(const Trie *trie, TrieIndex index);

#endif