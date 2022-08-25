#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#include <stdbool.h>



#define END_OF_TEXT '\3'
#define TRIE_POOL_INFO 0
#define TRIE_POOL_START 1


typedef long TrieIndex, TrieBase, TrieChar, TailIndex;
typedef unsigned long ListIndex, TailCharIndex, NeedleIndex;


typedef struct {
    TrieIndex trieIndex;
    ListIndex next;
    ListIndex prev;
} ListCell;

typedef struct {
    ListCell *cells;
    ListIndex size;
    ListIndex rear;
    ListIndex front;
} List;

typedef struct {
    TrieChar * characters;
    NeedleIndex length;
} Needle;

typedef struct {
    TrieChar *chars;
    TailCharIndex length;
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
    List *children;
} TrieCell;

typedef struct {
    bool useTail:1;
} TrieOptions;

typedef struct {
    TrieOptions *options;
    TrieCell *cells;
    TrieIndex cellsSize;
    Tail *tail;
} Trie;

#endif