#ifndef AC_H
#define AC_H

#include "typedefs.h"

Automaton *createAutomaton_DFS(const Trie *trie, List *list);
Automaton *createAutomaton_BFS(const Trie *trie, List *list);

void automaton_free(Automaton *automaton);
bool automaton_search(const Automaton *automaton, const TailBuilder *tailBuilder, const Needle *needle);

#endif