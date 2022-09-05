#ifndef AC_LIST_H
#define AC_LIST_H

#include "defs.h"

List *createList(ListIndex initialSize);
void list_free(List *list);
void list_reset(List *list);

TrieIndex list_pop(List *list);
TrieIndex list_shift(List *list);
TrieIndex list_getValue(const List *list, ListIndex index);

ListIndex list_push(List *list, TrieIndex value);
ListIndex list_insert(List *list, TrieIndex value);
ListIndex list_linearSearch(const List *list, TrieIndex value);
ListIndex list_binarySearch(const List *list, TrieIndex value);
ListIndex list_iterate(const List *list, ListIndex currentIndex);
ListIndex list_getRear(const List *list);

void list_mergeSort(List *list);
void list_remove(List *list, ListIndex index);
void list_delete(List *list, ListIndex index);

bool list_isEmpty(const List *list);

#endif