#ifndef __AC_LIST_H__
#define __AC_LIST_H__

#include "typedefs.h"

List *create_List(AlphabetSize initialSize);
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

void list_mergeSort(List *list);
void list_delete(List *list, ListIndex index);

unsigned char list_isEmpty(const List *list);

void list_print(List *list);

#endif