#ifndef __AC_LIST_H__
#define __AC_LIST_H__

#include "typedefs.h"

typedef long int ListSize;
typedef long int ListIndex;

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
TrieIndex list_pop(List *list);
TrieIndex list_shift(List *list);
ListIndex list_push(List *list, TrieIndex value);
unsigned char list_isEmpty(List *list);

void list_print(List *list);

#endif