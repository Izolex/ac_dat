#ifndef __DAT_H__
#define __DAT_H__

#include "unicode/utf8.h"

typedef u_int32_t TrieChar;
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

typedef struct {
    TrieChar * characters;
    int length;
    int error;
} TrieNeedle;


TrieBuilder* create_TrieBuilder();
TrieNeedle *createNeedle(const char *needle);
void trie_addNeedle(Trie *trie, TrieNeedle *needle);
void trie_find(Trie *trie, TrieNeedle *needle);
TrieIndex trie_findLastFilled(Trie *trie);

TrieChar *allocateTrieChars(TrieIndex size);

#endif