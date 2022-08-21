#ifndef __AC_H__
#define __AC_H__

#include "typedefs.h"

Trie *createAutomaton_DFS(Trie *trie);
Trie *createAutomaton_BFS(Trie *trie);
Bool search(Trie *trie, const TrieNeedle *needle);

#endif