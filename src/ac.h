#ifndef __AC_H__
#define __AC_H__

#include "typedefs.h"

typedef long AutomatonTransition;
typedef long AutomatonIndex;

typedef struct {
    AutomatonIndex base;
    AutomatonIndex check;
    AutomatonIndex fail;
    AutomatonIndex shortcut;
} AutomatonCell;

typedef struct {
    AutomatonIndex size;
    AutomatonCell *cells;
} Automaton;

Automaton *createAutomaton_DFS(const Trie *trie, List *list);
Automaton *createAutomaton_BFS(const Trie *trie, List *list);
unsigned char automaton_search(const Automaton *automaton, const Tail *tail, const Needle *needle);

#endif