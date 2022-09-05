#ifndef DAT_H
#define DAT_H

#include "defs.h"

TrieOptions *createTrieOptions(bool useTail);
void trieOptions_free(TrieOptions *options);

Trie *createTrie(TrieOptions *options, TailBuilder *tailBuilder, TrieIndex);
void trie_free(Trie *trie);

void trie_addNeedle(Trie *trie, const Needle *needle);
void trie_find(Trie *trie, Needle *needle);

TrieBase trie_getBase(const Trie *trie, TrieIndex index);
TrieIndex trie_getCheck(const Trie *trie, TrieIndex index);
List *trie_getChildren(const Trie *trie, TrieIndex index);

#endif