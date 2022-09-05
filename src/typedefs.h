#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#include <stdbool.h>


#define END_OF_TEXT 3
#define TRIE_POOL_INFO 0
#define TRIE_POOL_START 1


#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define prefetch(addr, rw, locality) __builtin_prefetch((addr), (rw), (locality))
#define add_overflow(a, b, result) __builtin_add_overflow((a), (b), (result))
#else
#define likely(x) (x)
#define unlikely(x) (x)
#define prefetch(addr, rw, locality) (void)
#define add_overflow(a, b, result) ({(*result) = (a) + (b); false;})
#endif


typedef long TrieIndex, TrieBase, Character, TailIndex, ListValue, AutomatonTransition, AutomatonIndex;
typedef unsigned long ListIndex, TailCharIndex, NeedleIndex;


typedef struct {
    Character *chars;
    TailCharIndex length;
} TailCell;

typedef struct {
    TailIndex size;
    TailCell *cells;
} Tail;

typedef struct {
    AutomatonIndex base;
    AutomatonIndex check;
    AutomatonIndex fail;
    AutomatonIndex output;
} AutomatonCell;

typedef struct {
    AutomatonIndex size;
    AutomatonCell *cells;
} Automaton;


typedef struct {
    ListValue value;
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
    Character *characters;
    NeedleIndex length;
} Needle;

typedef struct {
    Character *chars;
    TailCharIndex length;
    TailIndex nextFree;
} TailBuilderCell;

typedef struct {
    TailBuilderCell *cells;
    TailIndex size;
} TailBuilder;

typedef struct {
    TrieBase base;
    TrieIndex check;
    List *children;
} TrieCell;

typedef struct {
    bool useTail: 1;
} TrieOptions;

typedef struct {
    TrieOptions *options;
    TrieCell *cells;
    TrieIndex size;
    TailBuilder *tailBuilder;
} Trie;

typedef struct {
    Automaton *automaton;
    Tail *tail;
} FileData;

#endif