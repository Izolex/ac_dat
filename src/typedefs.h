#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__


#include <stdbool.h>


#define MAX_ALPHABET_SIZE 144697 // Unicode 14.0.0
#define END_OF_TEXT '\3'
#define TRIE_POOL_INFO 0
#define TRIE_POOL_START 1

typedef long int TrieIndex;
typedef long int TrieBase;
typedef long int TailIndex;
typedef long int AlphabetSize;
typedef long int TrieChar;

typedef long int ListIndex;

typedef unsigned long TailCharIndex;
typedef unsigned long NeedleIndex;


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
    AlphabetSize length;
} Needle;

typedef struct {
    TrieChar chars[MAX_ALPHABET_SIZE];
    AlphabetSize count;
} CharSet;

typedef struct {
    TrieChar *chars;
    size_t length;
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