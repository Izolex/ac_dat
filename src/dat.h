#ifndef __DAT_H__
#define __DAT_H__

#include "unicode/utf8.h"

typedef long int TrieIndex;
typedef long int TrieBase;

typedef struct {
    TrieBase base;
    TrieIndex check;
} Cell;

typedef struct {
    Cell *cells;
    TrieIndex cellsSize;
} Trie;

typedef struct {
    Trie *trie;
} TrieBuilder;

TrieBuilder* create_TrieBuilder();
void trieBuilder_addNeedle(TrieBuilder *builder, const char *needle);
void trie_find(Trie *trie, const char *needle);
TrieIndex trie_findLastFilled(Trie *trie);

#endif