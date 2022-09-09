#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#include <stdbool.h>


#define END_OF_TEXT 3
#define TRIE_POOL_INFO 0
#define TRIE_POOL_START 1


#define error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)


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


typedef int32_t TrieIndex, TrieBase, Character, TailIndex, ListValue, AutomatonTransition, AutomatonIndex;
typedef u_int32_t ListIndex, TailCharIndex, NeedleIndex;


typedef struct {
    Character *characters;
    NeedleIndex length;
} Needle;


typedef struct {
    AutomatonIndex base, check, fail, output;
} AutomatonCell;

typedef struct {
    AutomatonIndex size;
    AutomatonCell *cells;
} Automaton;


typedef struct {
    Character *chars;
    TailCharIndex length;
} TailCell;

typedef struct {
    TailIndex size;
    TailCell *cells;
} Tail;

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
    ListValue value;
    ListIndex next, prev;
} ListCell;

typedef struct {
    ListCell *cells;
    ListIndex size, rear, front;
} List;


typedef struct {
    bool useTail: 1;
    size_t childListInitSize;
} TrieOptions;

typedef struct {
    TrieBase base;
    TrieIndex check;
    List *children;
} TrieCell;

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