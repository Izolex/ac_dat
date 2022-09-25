#ifndef AC_LIST_H
#define AC_LIST_H

#include "../include/list.h"


typedef int32_t ListValue;
typedef u_int32_t ListIndex;

typedef struct {
    ListValue value;
    ListIndex next, prev;
} ListCell;

typedef struct list {
    ListCell *cells;
    ListIndex size, rear, front;
} List;


void list_reset(List *list);

ListValue list_pop(List *list);
ListValue list_shift(List *list);
ListValue list_getValue(const List *list, ListIndex index);

ListIndex list_push(List *list, ListValue value);
ListIndex list_insert(List *list, ListValue value);
ListIndex list_linearSearch(const List *list, ListValue value);
ListIndex list_binarySearch(const List *list, ListValue value);
ListIndex list_iterate(const List *list, ListIndex currentIndex);
ListIndex list_getRear(const List *list);

void list_mergeSort(List *list);
void list_remove(List *list, ListIndex index);
void list_delete(List *list, ListIndex index);

_Bool list_isEmpty(const List *list);

#endif
