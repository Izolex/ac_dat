#ifndef AC_H
#define AC_H

#include "typedefs.h"

typedef long AutomatonTransition;
typedef long AutomatonIndex;

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

Automaton *createAutomaton_DFS(const Trie *trie, List *list);
Automaton *createAutomaton_BFS(const Trie *trie, List *list);

void automaton_free(Automaton *automaton);
bool automaton_search(const Automaton *automaton, const Tail *tail, const Needle *needle);

#endif