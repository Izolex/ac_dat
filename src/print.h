#ifndef PRINT_H
#define PRINT_H

#include "typedefs.h"

void list_print(const List *list);
void tailBuilder_print(const TailBuilder *tailBuilder);
void tail_print(const Tail *tail);
void trie_print(const Trie *trie);
void automaton_print(const Automaton *automaton);

#endif