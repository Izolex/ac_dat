#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ac_list.h"

ListIndex list_getFirstFree(List *list) {
    return list->cells[0].next;
}

void list_setFirstFree(List *list, ListIndex index) {
    list->cells[0].next = index;
}

ListIndex list_getLastFree(List *list) {
    return list->cells[0].prev;
}

void list_setLastFree(List *list, ListIndex index) {
    list->cells[0].prev = index;
}

void list_connectList(List *list, ListIndex fromIndex, ListIndex toIndex) {
    for (ListIndex i = fromIndex; i < toIndex; i++) {
        list->cells[i].next = i+1;
        list->cells[i].prev = i-1;
    }
}

void list_free(List *list) {
    free(list);
}

List *create_List() {
    List *list = calloc(1, sizeof(List));
    if (list == NULL) {
        fprintf(stderr, "can not allocate %lu memory for list", sizeof(List));
        exit(1);
    }

    list->size = MAX_ALPHABET_SIZE;
    list->cells = malloc(sizeof(ListCell) * list->size);
    if (list->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for list values", sizeof(TrieIndex) * list->size);
        exit(1);
    }

    list_connectList(list, 0, list->size);

    list->cells[0].next = list->size;
    list->cells[list->size].next = 0;

    return list;
}

void list_poolReallocate(List *list) {
    ListSize newSize = list->size + (long int)ceill(((long double)list->size / 2));
    list->cells = realloc(list->cells, sizeof(ListCell) * newSize);
    if (list->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for list values", sizeof(TrieIndex) * newSize);
        exit(1);
    }

    list_connectList(list, list->size, newSize);

    if (list_getFirstFree(list) == 0) {
        list_setFirstFree(list, list->size);
    } else {
        list->cells[list_getLastFree(list)].next = list->size;
    }

    list_setLastFree(list, newSize);

    list->size = newSize;
}

void list_freeCell(List *list, ListIndex index) {
    if (list_getFirstFree(list) == 0 && list_getLastFree(list) == 0) {
        list_setFirstFree(list, index);
        list_setLastFree(list, index);
        list->cells[index].next = 0;
        list->cells[index].prev = 0;
    } else if (list_getFirstFree(list) > index) {
        list->cells[index].next = list_getFirstFree(list);
        list->cells[index].prev = 0;
        if (list_getFirstFree(list) == list_getLastFree(list)) {
            list_setLastFree(list, index);
        }
        list_setFirstFree(list, index);
    } else if (list_getLastFree(list) < index) {
        list->cells[index].prev = list_getLastFree(list);
        list->cells[index].next = 0;
        if (list_getFirstFree(list) == list_getLastFree(list)) {
            list_setFirstFree(list, index);
        }
        list_setLastFree(list, index);
    } else {
        ListIndex nextFree = list_getFirstFree(list);
        while (nextFree < index) {
            nextFree = list->cells[nextFree].next;
        }

        list->cells[index].next = nextFree;
        list->cells[index].prev = list->cells[nextFree].prev;

        list->cells[list->cells[nextFree].prev].next = nextFree;
        list->cells[nextFree].prev = index;
    }
}


unsigned char list_isEmpty(List *list) {
    return list->front == 0 && list->rear == 0;
}


ListIndex list_push(List *list, TrieIndex value) {
    ListIndex index = list_getFirstFree(list);
    if (index == 0) {
        list_poolReallocate(list);
        index = list_getFirstFree(list);
    }

    if (list->rear) {
        list->cells[list->rear].next = index;
    }
    if (!list->front) {
        list->front = index;
    }

    list_setFirstFree(list, list->cells[index].next);
    if (list_getLastFree(list) == index) {
        list_setLastFree(list, 0);
    }

    list->rear = index;
    list->cells[index].trieIndex = value;
    list->cells[index].next = 0;

    return index;
}

TrieIndex list_shift(List *list) {
    ListIndex index = list->front;
    TrieIndex trieIndex = list->cells[index].trieIndex;

    list->cells[index].trieIndex = 0;
    list->front = list->cells[index].next;
    list->cells[list->front].prev = 0;
    if (index == list->rear) {
        list->rear = 0;
    }

    list_freeCell(list, index);

    return trieIndex;
}

TrieIndex list_pop(List *list) {
    ListIndex index = list->rear;
    TrieIndex trieIndex = list->cells[index].trieIndex;

    list->cells[index].trieIndex = 0;
    list->rear = list->cells[index].prev;
    list->cells[list->rear].next = 0;
    if (index == list->front) {
        list->front = 0;
    }

    list_freeCell(list, index);

    return trieIndex;
}


void list_print(List *list) {
    printf("\n\n");
    printf("FirstFree: %ld, lastFree: %ld, front: %ld, rear: %ld", list_getFirstFree(list), list_getLastFree(list), list->front, list->rear);
    printf("\n\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4ld | ", list->cells[i].trieIndex);
    }
    printf("\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4ld | ", list->cells[i].next);
    }
    printf("\n\n");
}