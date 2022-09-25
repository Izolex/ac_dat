#ifndef __AC_DAT__AH_H__
#define __AC_DAT__AH_H__


#include "dat.h"
#include "list.h"


enum searchMode {
    SEARCH_MODE_FIRST     = 0b00000001,
    SEARCH_MODE_NEEDLE    = 0b00000010,
    SEARCH_MODE_USER_DATA = 0b00000100,
};

struct automaton;
struct occurrence;


size_t automaton_getSize(const struct automaton *automaton);
struct occurrence *automaton_search(
    const struct automaton *automaton,
    const struct tail *tail,
    const struct userDataList *userDataList,
    const struct needle *needle,
    enum searchMode mode
);

struct automaton *createAutomaton_DFS(const struct trie *trie, struct list *list);
struct automaton *createAutomaton_BFS(const struct trie *trie, struct list *list);

void occurrence_free(struct occurrence *occurrence);
void automaton_free(struct automaton *automaton);

#endif
