#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#define MAX_ALPHABET_SIZE 144697 // Unicode 14.0.0

typedef long int TrieIndex;
typedef long int TrieBase;
typedef long int TailIndex;
typedef unsigned long int AlphabetSize;
typedef unsigned long int TrieChar;

typedef struct {
    TrieChar * characters;
    AlphabetSize length;
} TrieNeedle;

typedef struct {
    TrieChar chars[MAX_ALPHABET_SIZE];
    AlphabetSize count;
} CharSet;

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
    TrieIndex fail;
    TrieIndex shortcut;
} TrieCell;

typedef struct {
    TrieCell *cells;
    TrieIndex cellsSize;
    Tail *tail;
} Trie;

#endif