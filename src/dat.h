#ifndef __DAT_H__
#define __DAT_H__

#include "unicode/utf8.h"
#include "character.h"

typedef long int TrieIndex;
typedef long int TrieBase;
typedef long int TailIndex;

typedef struct {
    TrieChar *chars;
    TrieIndex length;
    TailIndex nextFree;
} TailCell;

typedef struct {
    TailCell *cells;
    TailIndex cellsSize;
    TailIndex firstFree;
} Tail;

typedef struct {
    TrieBase base;
    TrieIndex check;
} Cell;

typedef struct {
    Cell *cells;
    TrieIndex cellsSize;
    Tail *tail;
} Trie;

typedef struct {
    Trie *trie;
} TrieBuilder;


TrieBuilder *create_TrieBuilder();
void trie_addNeedle(Trie *trie, TrieNeedle *needle);
void trie_find(Trie *trie, TrieNeedle *needle);
TrieIndex trie_findLastFilled(Trie *trie);

void trie_print(Trie *trie);

#endif