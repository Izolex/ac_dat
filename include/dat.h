#ifndef __AC_DAT__DAT_H__
#define __AC_DAT__DAT_H__


#include "needle.h"
#include "tail.h"
#include "user_data.h"


struct trieOptions;
struct trie;


struct trieOptions *createTrieOptions(_Bool useTail, _Bool useUserData, size_t childListInitSize);
void trieOptions_free(struct trieOptions *options);

struct trie *createTrie(struct trieOptions *options, struct tailBuilder *tailBuilder, struct userDataList *userDataList, size_t initialSize);
size_t trie_getSize(const struct trie *trie);
void trie_free(struct trie *trie);

void trie_addNeedle(struct trie *trie, const struct trieNeedle *needle);
void trie_addNeedleWithData(struct trie *trie, const struct trieNeedle *needle, struct userData data);

#endif
