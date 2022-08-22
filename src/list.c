#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "list.h"

static ListIndex list_getFirstFree(const List *list);
static ListIndex list_getLastFree(const List *list);
static void list_setFirstFree(List *list, ListIndex index);
static void list_setLastFree(List *list, ListIndex index);
static void list_connectList(List *list, ListIndex fromIndex, ListIndex toIndex);
static void list_poolReallocate(List *list);
static void list_freeCell(List *list, ListIndex index);
static void list_mergeSort_merge(List *list, ListIndex left, ListIndex middle, ListIndex right);


static ListIndex list_getFirstFree(const List *list) {
    return list->cells[0].next;
}

static void list_setFirstFree(List *list, const ListIndex index) {
    list->cells[0].next = index;
}

static ListIndex list_getLastFree(const List *list) {
    return list->cells[0].prev;
}

static void list_setLastFree(List *list, const ListIndex index) {
    list->cells[0].prev = index;
}

static void list_connectList(List *list, const ListIndex fromIndex, const ListIndex toIndex) {
    for (ListIndex i = fromIndex; i < toIndex; i++) {
        list->cells[i].next = i+1;
        list->cells[i].prev = i-1;
    }
}

void list_free(List *list) {
    free(list);
}

List *create_List(const ListIndex initialSize) {
    List *list = calloc(1, sizeof(List));
    if (list == NULL) {
        fprintf(stderr, "can not allocate %lu memory for list", sizeof(List));
        exit(1);
    }

    list->size = initialSize;
    list->cells = calloc(list->size, sizeof(ListCell));
    if (list->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for list values", sizeof(TrieIndex) * list->size);
        exit(1);
    }

    list_connectList(list, 1, list->size - 1);

    list->cells[0].next = 1;
    list->cells[0].prev = list->size - 1;
    list->cells[list->size - 1].prev = list->size - 2;

    return list;
}

static void list_poolReallocate(List *list) {
    ListIndex newSize = list->size + (long int)ceill(((long double)list->size / 2)) + 1;
    list->cells = realloc(list->cells, sizeof(ListCell) * newSize);
    if (list->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for list values", sizeof(ListCell) * newSize);
        exit(1);
    }

    list_connectList(list, list->size, newSize - 1);

    if (list_getFirstFree(list) == 0) {
        list_setFirstFree(list, list->size);
    } else {
        list->cells[list_getLastFree(list)].next = list->size;
    }

    list->cells[list->size].prev = list_getLastFree(list);
    list->cells[newSize - 1].prev = newSize - 2;
    list->size = newSize;

    list_setLastFree(list, newSize - 1);
}

static void list_freeCell(List *list, const ListIndex index) {
    if (list_getFirstFree(list) == 0 && list_getLastFree(list) == 0) {
        list_setFirstFree(list, index);
        list_setLastFree(list, index);
        list->cells[index].next = 0;
        list->cells[index].prev = 0;
    } else if (list_getFirstFree(list) >= index) {
        list->cells[index].next = list_getFirstFree(list);
        list->cells[index].prev = 0;
        list->cells[list_getFirstFree(list)].prev = index;
        list_setFirstFree(list, index);
    } else if (list_getLastFree(list) <= index) {
        list->cells[index].prev = list_getLastFree(list);
        list->cells[index].next = 0;
        list->cells[list_getLastFree(list)].next = index;
        list_setLastFree(list, index);
    } else {
        ListIndex nextFree = list_getFirstFree(list);
        while (nextFree < index) {
            nextFree = list->cells[nextFree].next;
        }

        list->cells[index].next = nextFree;
        list->cells[index].prev = list->cells[nextFree].prev;

        ListIndex prev = list->cells[nextFree].prev;
        list->cells[prev].next = index;
        list->cells[nextFree].prev = index;
    }
}


bool list_isEmpty(const List *list) {
    return list->front == 0 && list->rear == 0;
}


ListIndex list_push(List *list, const TrieIndex value) {
    ListIndex index = list_getFirstFree(list);
    if (index == 0) {
        list_poolReallocate(list);
        index = list_getFirstFree(list);
    }

    if (list_getLastFree(list) == index) {
        if (list->cells[index].prev == 0) {
            list_setLastFree(list, list->cells[index].next);
        } else {
            list_setLastFree(list, list->cells[index].prev);
        }
    }
    list_setFirstFree(list, list->cells[index].next);

    if (list->cells[index].next) {
        list->cells[list->cells[index].next].prev = list->cells[index].prev;
    }

    if (list->rear) {
        list->cells[list->rear].next = index;
        list->cells[index].prev = list->rear;
    } else {
        list->cells[index].prev = 0;
    }
    if (!list->front) {
        list->front = index;
    }

    list->rear = index;
    list->cells[index].trieIndex = value;
    list->cells[index].next = 0;

    return index;
}

ListIndex list_insert(List *list, const TrieIndex value) {
    if (list->front == 0 && list->rear == 0 || list->cells[list->rear].trieIndex < value) {
        return list_push(list, value);
    }

    ListIndex from = 0, to = list->rear;

    while (from <= to) {
        ListIndex middle = (from + to) / 2;
        if (value > list->cells[middle].trieIndex) {
            from = middle + 1;
        } else {
            to = middle - 1;
        }
    }

    if (list->rear + 1 > list->size - 1) {
        list_poolReallocate(list);
    } else if (list->rear + 1 == list->size - 1) {
        list_setFirstFree(list, 0);
        list_setLastFree(list, 0);
    } else {
        list_setFirstFree(list, list->cells[list_getFirstFree(list)].next);
    }

    for (ListIndex i = list->rear; i >= from; i--) {
        list->cells[i + 1].trieIndex = list->cells[i].trieIndex;
        list->cells[i + 1].prev = i;
        list->cells[i + 1].next = i + 2;
    }

    list->cells[list->rear + 1].next = 0;
    list->cells[from + 1].prev = from;

    list->cells[from].trieIndex = value;
    list->cells[from].next = from + 1;

    if (from == list->front) {
        list->cells[from].prev = 0;
    } else {
        list->cells[from].prev = from - 1;
    }

    list->rear++;

    return from;
}

TrieIndex list_shift(List *list) {
    if (list_isEmpty(list)) {
        fprintf(stderr, "list is empty");
        exit(1);
    }

    ListIndex index = list->front;
    TrieIndex trieIndex = list->cells[index].trieIndex;

    list->cells[index].trieIndex = 0;
    list->front = list->cells[index].next;
    if (list->front) {
        list->cells[list->front].prev = 0;
    }
    if (index == list->rear) {
        list->rear = 0;
    }

    list_freeCell(list, index);

    return trieIndex;
}

TrieIndex list_pop(List *list) {
    if (list_isEmpty(list)) {
        fprintf(stderr, "list is empty");
        exit(1);
    }

    ListIndex index = list->rear;
    TrieIndex trieIndex = list->cells[index].trieIndex;

    list->cells[index].trieIndex = 0;
    list->rear = list->cells[index].prev;
    if (list->rear) {
        list->cells[list->rear].next = 0;
    }
    if (index == list->front) {
        list->front = 0;
    }

    list_freeCell(list, index);

    return trieIndex;
}

ListIndex list_linearSearch(const List *list, const TrieIndex value) {
    if (list->front == 0) {
        return 0;
    }

    ListIndex index = list->front;
    while (index != 0) {
        if (list->cells[index].trieIndex == value) {
            return index;
        }
        index = list->cells[index].next;
    }

    return 0;
}

void list_delete(List *list, const ListIndex index) {
    if (list->front == index) {
        list->front = list->cells[index].next;
        if (list->front) {
            list->cells[list->front].prev = 0;
        }
    } else {
        if (list->cells[index].prev) {
            list->cells[list->cells[index].prev].next = list->cells[index].next;
        }
    }
    if (list->rear == index) {
        list->rear = list->cells[index].prev;
        if (list->rear) {
            list->cells[list->rear].next = 0;
        }
    } else {
        if (list->cells[index].next) {
            list->cells[list->cells[index].next].prev = list->cells[index].prev;
        }
    }

    list->cells[index].trieIndex = 0;
    list_freeCell(list, index);
}

void list_reset(List *list) {
    for (ListIndex i = 0; i < list->size; i++) {
        list->cells[i].next = i+1;
        list->cells[i].prev = i-1;
        list->cells[i].trieIndex = 0;
    }

    list_setFirstFree(list, 1);
    list_setLastFree(list, list->size - 1);

    list->front = 0;
    list->rear = 0;

    list->cells[list->size - 1].next = 0;
}

ListIndex list_iterate(const List *list, const ListIndex currentIndex) {
    if (currentIndex == 0) {
        return list->front;
    }

    return list->cells[currentIndex].next;
}

TrieIndex list_getValue(const List *list, const ListIndex index) {
    return list->cells[index].trieIndex;
}

static void list_mergeSort_merge(List *list, const ListIndex left, const ListIndex middle, const ListIndex right) {
    ListIndex leftSize = middle - left + 1, rightSize = right - middle;
    ListCell leftArray[leftSize], rightArray[rightSize];

    for (ListIndex i = 0; i < leftSize; i++) {
        leftArray[i] = list->cells[left + i];
    }
    for (ListIndex i = 0; i < rightSize; i++) {
        rightArray[i] = list->cells[middle + 1 + i];
    }

    ListIndex leftIndex = 0, rightIndex = 0, listIndex = left;
    while (leftIndex < leftSize && rightIndex < rightSize) {
        if (leftArray[leftIndex].trieIndex == 0 && rightArray[rightIndex].trieIndex == 0) {
            leftIndex++;
            rightIndex++;
            continue;
        } else if (leftArray[leftIndex].trieIndex != 0 && leftArray[leftIndex].trieIndex <= rightArray[rightIndex].trieIndex) {
            list->cells[listIndex].trieIndex = leftArray[leftIndex].trieIndex;
            leftIndex++;
        } else {
            list->cells[listIndex].trieIndex = rightArray[rightIndex].trieIndex;
            rightIndex++;
        }
        listIndex++;
    }

    for (; leftIndex < leftSize; listIndex++, leftIndex++) {
        list->cells[listIndex].trieIndex = leftArray[leftIndex].trieIndex;
    }
    for (; rightIndex < rightSize; listIndex++, rightIndex++) {
        list->cells[listIndex].trieIndex = rightArray[rightIndex].trieIndex;
    }
}

void list_mergeSort(List *list) {
    ListIndex toIndex = list->rear;
    if (toIndex == 0) {
        return;
    }

    ListIndex current, left;
    for (current = 1; current <= list->size - 1; current = 2 * current) {
        for (left = 0; left < list->size - 1; left += 2 * current) {
            ListIndex middle = left + current - 1;
            if (middle > list->size - 1) {
                middle = list->size - 1;
            }
            ListIndex right = left + 2 * current - 1;
            if (right > list->size - 1) {
                right = list->size - 1;
            }

            list_mergeSort_merge(list, left ?: 1, middle, right);
        }
    }

    list_connectList(list, 0, toIndex);
    if (toIndex == list->size - 1) {
        list->cells[toIndex].next = 0;
    } else {
        list->cells[toIndex + 1].prev = toIndex;
    }

    ListIndex rear = list->rear;
    if (list_getValue(list, rear) == 0) {
        while (list_getValue(list, rear) == 0) {
            rear--;
        }
    }

    if (rear == list->size - 1) {
        list_setFirstFree(list, 0);
        list_setLastFree(list, 0);
    } else {
        list_setFirstFree(list, rear + 1);
        list_setLastFree(list, list->size - 1);
    }

    list->front = 1;
    list->rear = rear;
    list->cells[rear].next = 0;
    if (rear < list->size - 1) {
        list->cells[rear + 1].prev = 0;
    }
}

ListIndex list_binarySearch(const List *list, const TrieIndex value) {
    ListIndex from = 0, to = list->rear;

    while (from <= to) {
        ListIndex middle = (from + to) / 2;
        if (value == list->cells[middle].trieIndex) {
            return middle;
        } else if (value > list->cells[middle].trieIndex) {
            from = middle + 1;
        } else {
            to = middle - 1;
        }
    }

    return 0;
}
