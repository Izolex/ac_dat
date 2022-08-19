#ifndef __AC_LIST_H__
#define __AC_LIST_H__

#include "typedefs.h"

typedef unsigned long int ListSize;
typedef unsigned long int ListIndex;

typedef struct {
    TrieIndex trieIndex;
    ListIndex next;
    ListIndex prev;
} ListCell;

typedef struct {
    ListCell *cells;
    ListSize size;
    ListIndex rear;
    ListIndex front;
} List;

List *create_List();
void list_free(List *list);
ListIndex list_enqueue(List *list, TrieIndex value);
TrieIndex list_dequeue(List *list);
unsigned char list_queueIsEmpty(List *list);

#endif