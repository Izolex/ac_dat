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

void list_print(List *list) {
    printf("\n\n");
    printf("FirstFree: %ld, front: %ld, rear: %ld", list_getFirstFree(list), list->front, list->rear);
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

void list_connectList(List *list, ListIndex fromIndex, ListIndex toIndex) {
    for (ListIndex i = fromIndex; i < toIndex; i++) {
        list->cells[i].next = i+1;
    }
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

    list_connectList(list, 0, list->size - 1);

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
        ListIndex prevEmpty = list->size - 1;
        while (prevEmpty != 0 && list->cells[prevEmpty].trieIndex != 0) {
            prevEmpty--;
        }

        list->cells[prevEmpty].next = list->size;
    }

    list->size = newSize;
}

ListIndex list_enqueue(List *list, TrieIndex value) {
    ListIndex firstFree = list_getFirstFree(list);
    if (firstFree == 0) {
        list_poolReallocate(list);
        firstFree = list_getFirstFree(list);
    }
    ListIndex newFirstFree = list->cells[firstFree].next;

    list->cells[firstFree].trieIndex = value;
    list->cells[firstFree].next = 0;

    if (list->rear) {
        list->cells[list->rear].next = firstFree;
    }
    if (!list->front) {
        list->front = firstFree;
    }

    list->rear = firstFree;
    list->cells[0].next = newFirstFree;
    list_setFirstFree(list, newFirstFree);

    return firstFree;
}

TrieIndex list_dequeue(List *list) {
    ListIndex index = list->front;
    TrieIndex trieIndex = list->cells[index].trieIndex;

    list->cells[index].trieIndex = 0;
    list->front = list->cells[index].next;

    if (index == list->rear) {
        list->rear = 0;
        list->cells[index].next = 0;
        list_setFirstFree(list, index);
        return trieIndex;
    }

    ListIndex firstFree = list_getFirstFree(list);
    if (firstFree == 0) {
        list_setFirstFree(list, index);
    } else if (firstFree > index) {
        list->cells[index].next = firstFree;
        list_setFirstFree(list, index);
    } else {
        ListIndex prevEmpty = index;
        while (prevEmpty != 0) {
            if (list->cells[prevEmpty].trieIndex == 0) {
                break;
            }
            prevEmpty--;
        }


        list->cells[index].next = list->cells[prevEmpty].next;
        list->cells[prevEmpty].next = index;
    }

    return trieIndex;
}

unsigned char list_queueIsEmpty(List *list) {
    return list->front == 0 && list->rear == 0;
}

void list_free(List *list) {
    free(list);
}
