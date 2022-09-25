#ifndef __AC_DAT__PRINT__H__
#define __AC_DAT__PRINT__H__


#include "dat.h"
#include "ac.h"
#include "list.h"
#include "user_data.h"


void list_print(const struct list *list);
void tailBuilder_print(const struct tailBuilder *tailBuilder);
void tail_print(const struct tail *tail);
void trie_print(const struct trie *trie);
void automaton_print(const struct automaton *automaton);
void userDataList_print(size_t size, const struct userDataList *userDataList);

#endif
