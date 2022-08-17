#ifndef __AC_H__
#define __AC_H__

#include "typedefs.h"

Trie *createAutomaton(Trie *trie);
unsigned char search(Trie *trie, TrieNeedle *needle);

#endif