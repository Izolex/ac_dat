#ifndef DAT_H
#define DAT_H

#include "../include/dat.h"
#include "definitions.h"
#include "list.h"


typedef int32_t TrieIndex, TrieBase;

typedef struct trieOptions {
    bool useTail: 1;
    bool useUserData: 1;
    size_t childListInitSize;
} TrieOptions;

typedef struct {
    TrieBase base;
    TrieIndex check;
    struct list *children;
} TrieCell;

typedef struct trie {
    TrieOptions *options;
    TrieCell *cells;
    TrieIndex size;
    struct tailBuilder *tailBuilder;
    struct userDataList *userDataList;
} Trie;


TrieBase trie_getBase(const Trie *trie, TrieIndex index);
TrieIndex trie_getCheck(const Trie *trie, TrieIndex index);
List *trie_getChildren(const Trie *trie, TrieIndex index);

#endif
