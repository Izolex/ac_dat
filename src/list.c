#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "mem.h"


static ListIndex list_getFirstFree(const List *list);
static ListIndex list_getLastFree(const List *list);
static ListIndex list_getFront(const List*list);
static void list_setSize(List *list, ListIndex value);
static void list_setRear(List *list, ListIndex value);
static void list_setFront(List *list, ListIndex value);
static void list_setFirstFree(List *list, ListIndex index);
static void list_setLastFree(List *list, ListIndex index);
static void list_setLinks(List *list, ListIndex index, ListIndex next, ListIndex prev);
static void list_setCell(List *list, ListIndex index, ListIndex next, ListIndex prev, ListValue value);
static void list_poolInit(List *list, ListIndex fromIndex, ListIndex toIndex);
static void list_poolReallocate(List *list);
static void list_ensureNotEmpty(List *list);
static void list_freeCell(List *list, ListIndex index);
static void list_mergeSort_merge(List *list, ListIndex left, ListIndex middle, ListIndex right);


ListIndex list_getRear(const List *list) {
    return list->rear;
}

ListValue list_getValue(const List *list, const ListIndex index) {
    return list->cells[index].value;
}

static ListIndex list_getFirstFree(const List *list) {
    return list->cells[0].next;
}

static ListIndex list_getLastFree(const List *list) {
    return list->cells[0].prev;
}

static ListIndex list_getFront(const List *list) {
    return list->front;
}


static void list_setSize(List *list, const ListIndex value) {
    list->size = value;
}

static void list_setCells(List *list, ListCell *cells) {
    list->cells = cells;
}

static void list_setFirstFree(List *list, const ListIndex index) {
    list->cells[0].next = index;
}

static void list_setLastFree(List *list, const ListIndex index) {
    list->cells[0].prev = index;
}

static void list_setFirstLastFree(List *list, const ListIndex first, const ListIndex last) {
    list->cells[0] = (ListCell) {0, first, last};
}

static void list_setRear(List *list, const ListIndex value) {
    list->rear = value;
}

static void list_setFront(List *list, const ListIndex value) {
    list->front = value;
}

static void list_setLinks(List *list, const ListIndex index, const ListIndex next, const ListIndex prev) {
    list->cells[index].next = next;
    list->cells[index].prev = prev;
}

static void list_setCell(List *list, const ListIndex index, const ListIndex next, const ListIndex prev, const ListValue value) {
    list->cells[index] = (ListCell) {value, next, prev};
}


static void list_poolInit(List *list, const ListIndex fromIndex, const ListIndex toIndex) {
    for (ListIndex i = fromIndex; i < toIndex; i++) {
        list_setCell(list, i, i+1, i-1, 0);
    }
}

void list_free(List *list) {
    free(list->cells);
    free(list);
    list = NULL;
}

List *createList(const ListIndex initialSize) {
    List *list = safeMalloc(sizeof(List), "List");

    list_setFront(list, 0);
    list_setRear(list, 0);
    list_setSize(list, initialSize);

    list_setCells(list, safeMalloc(list->size * sizeof(ListCell), "List cells"));
    list_poolInit(list, 1, list->size - 1);

    list_setCell(list, 0, 1, list->size - 1, 0);
    list_setCell(list, list->size - 1, 0, list->size - 2, 0);

    return list;
}

static void list_poolReallocate(List *list) {
    const ListIndex lastFree = list_getLastFree(list);
    const ListIndex newSize = (ListIndex)calculateAllocation(list->size);

    list_setCells(list, safeRealloc(list->cells, newSize, sizeof(ListCell), "List cells"));
    list_poolInit(list, list->size, newSize - 1);

    if (list_getFirstFree(list) == 0) {
        list_setFirstFree(list, list->size);
    } else {
        list->cells[lastFree].next = list->size;
    }

    list->cells[list->size].prev = lastFree;
    list_setSize(list, newSize);
    list_setCell(list, newSize - 1, 0, newSize - 2, 0);
    list_setLastFree(list, newSize - 1);
}

static void list_freeCell(List *list, const ListIndex index) {
    if (list_getFirstFree(list) == 0 && list_getLastFree(list) == 0) {
        list_setFirstLastFree(list, index, index);
        list_setLinks(list, index, 0, 0);
    } else if (list_getFirstFree(list) >= index) {
        list->cells[list_getFirstFree(list)].prev = index;
        list_setLinks(list, index, list_getFirstFree(list), 0);
        list_setFirstFree(list, index);
    } else if (list_getLastFree(list) <= index) {
        list_setLinks(list, index, 0, list_getLastFree(list));
        list->cells[list_getLastFree(list)].next = index;
        list_setLastFree(list, index);
    } else {
        ListIndex nextFree = list_getFirstFree(list);
        while (likely(nextFree < index)) {
            nextFree = list->cells[nextFree].next;
        }

        list_setLinks(list, index, nextFree, list->cells[nextFree].prev);

        ListIndex prev = list->cells[nextFree].prev;
        list->cells[prev].next = index;
        list->cells[nextFree].prev = index;
    }
}


bool list_isEmpty(const List *list) {
    return list_getFront(list) == 0 && list_getRear(list) == 0;
}


ListIndex list_push(List *list, const ListValue value) {
    ListIndex index = list_getFirstFree(list);
    if (index == 0) {
        list_poolReallocate(list);
        index = list_getFirstFree(list);
    }

    if (list_getLastFree(list) == index) {
        list_setFirstLastFree(list, 0, 0);
    } else {
        list_setFirstFree(list, list->cells[index].next);
    }

    if (list->cells[index].next) {
        list->cells[list->cells[index].next].prev = list->cells[index].prev;
    }
    if (list_getRear(list)) {
        list->cells[list_getRear(list)].next = index;
        list->cells[index].prev = list_getRear(list);
    } else {
        list->cells[index].prev = 0;
    }
    if (!list->front) {
        list_setFront(list, index);
    }

    list_setRear(list, index);
    list->cells[index].value = value;
    list->cells[index].next = 0;

    return index;
}

ListIndex list_insert(List *list, const ListValue value) {
    const ListIndex rear = list_getRear(list);

    if ((list_getFront(list) == 0 && rear == 0) || list->cells[rear].value < value) {
        return list_push(list, value);
    }
    ListIndex from = 0, to = rear;

    while (from <= to) {
        ListIndex middle = from + ((to - from) / 2);
        if (value == list->cells[middle].value) {
            return middle;
        } else if (value > list->cells[middle].value) {
            from = middle + 1;
        } else {
            to = middle - 1;
        }
    }

    if (rear + 1 == list->size - 1) {
        list_setFirstLastFree(list, 0, 0);
    } else {
        if (rear + 1 > list->size - 1) {
            list_poolReallocate(list);
        }
        if (rear + 2 < list->size) {
            list->cells[rear + 2].prev = 0;
        }
        list_setFirstFree(list, list->cells[list_getFirstFree(list)].next);
    }

    for (ListIndex i = rear; i >= from; i--) {
        list_setCell(list, i + 1, i + 2, i, list->cells[i].value);
    }

    list->cells[rear + 1].next = 0;
    list->cells[from + 1].prev = from;

    list_setCell(list, from, from + 1, from == list_getFront(list) ? 0 : from - 1, value);
    list_setRear(list, rear + 1);

    return from;
}

static void list_ensureNotEmpty(List *list) {
    if (unlikely(list_isEmpty(list))) {
        fprintf(stderr, "list is empty");
        exit(EXIT_FAILURE);
    }
}

ListValue list_shift(List *list) {
    list_ensureNotEmpty(list);

    const ListIndex index = list->front;
    const ListValue value = list->cells[index].value;

    list->cells[index].value = 0;
    list_setFront(list, list->cells[index].next);
    if (list->front) {
        list->cells[list->front].prev = 0;
    }
    if (index == list_getRear(list)) {
        list_setRear(list, 0);
    }

    list_freeCell(list, index);

    return value;
}

ListValue list_pop(List *list) {
    list_ensureNotEmpty(list);

    const ListIndex index = list_getRear(list);
    const ListValue value = list->cells[index].value;

    list->cells[index].value = 0;
    list_setRear(list, list->cells[index].prev);
    if (list_getRear(list)) {
        list->cells[list_getRear(list)].next = 0;
    }
    if (index == list->front) {
        list_setFront(list, 0);
    }

    list_freeCell(list, index);

    return value;
}

ListIndex list_linearSearch(const List *list, const ListValue value) {
    if (list_getFront(list) == 0) {
        return 0;
    }

    ListIndex index = list->front;
    while (index != 0) {
        if (list->cells[index].value == value) {
            return index;
        }
        index = list->cells[index].next;
    }

    return 0;
}

// for non sorted list
void list_remove(List *list, const ListIndex index) {
    list_ensureNotEmpty(list);

    if (list_getFront(list) == index) {
        list_setFront(list, list->cells[index].next);
        if (list->front) {
            list->cells[list->front].prev = 0;
        }
    } else {
        if (list->cells[index].prev) {
            list->cells[list->cells[index].prev].next = list->cells[index].next;
        }
    }
    if (list_getRear(list) == index) {
        list_setRear(list, list->cells[index].prev);
        if (list_getRear(list)) {
            list->cells[list_getRear(list)].next = 0;
        }
    } else {
        if (list->cells[index].next) {
            list->cells[list->cells[index].next].prev = list->cells[index].prev;
        }
    }

    list->cells[index].value = 0;
    list_freeCell(list, index);
}

// keeps list sorted
void list_delete(List *list, const ListIndex index) {
    list_ensureNotEmpty(list);

    if (list_getRear(list) == index) {
        list_remove(list, index);
        return;
    }

    const ListIndex rear = list_getRear(list);

    for (ListIndex i = index + 1; i <= rear; i++) {
        list->cells[i - 1].value = list->cells[i].value;
    }

    if (rear + 1 <= list->size - 1) {
        list->cells[rear].next = list_getRear(list) + 1;
        list->cells[rear + 1].prev = rear;
        list_setFirstFree(list, rear);
    } else {
        list->cells[rear].next = 0;
        list_setFirstFree(list, 0);
    }

    list->cells[rear].value = 0;
    list->cells[rear].prev = 0;
    list->cells[rear - 1].next = 0;

    list_setRear(list, rear - 1);
}

void list_reset(List *list) {
    for (ListIndex i = 0; i < list->size; i++) {
        list_setCell(list, i, i+1, i-1, 0);
    }

    list_setFirstLastFree(list, 1, list->size - 1);

    list_setFront(list, 0);
    list_setRear(list, 0);

    list->cells[list->size - 1].next = 0;
}

ListIndex list_iterate(const List *list, const ListIndex currentIndex) {
    if (currentIndex == 0) {
        return list_getFront(list);
    }

    const ListIndex next = list->cells[currentIndex].next;
    prefetch(&list->cells[next], 0, 2);
    return next;
}

static void list_mergeSort_merge(List *list, const ListIndex left, const ListIndex middle, const ListIndex right) {
    const ListIndex leftSize = middle - left + 1, rightSize = right - middle;
    ListCell leftArray[leftSize], rightArray[rightSize];

    for (ListIndex i = 0; i < leftSize; i++) {
        leftArray[i] = list->cells[left + i];
    }
    for (ListIndex i = 0; i < rightSize; i++) {
        rightArray[i] = list->cells[middle + 1 + i];
    }

    ListIndex leftIndex = 0, rightIndex = 0, listIndex = left;
    while (leftIndex < leftSize && rightIndex < rightSize) {
        if (leftArray[leftIndex].value == 0 && rightArray[rightIndex].value == 0) {
            leftIndex++;
            rightIndex++;
            continue;
        } else if (leftArray[leftIndex].value != 0 && leftArray[leftIndex].value <= rightArray[rightIndex].value) {
            list->cells[listIndex].value = leftArray[leftIndex].value;
            leftIndex++;
        } else {
            list->cells[listIndex].value = rightArray[rightIndex].value;
            rightIndex++;
        }
        listIndex++;
    }

    for (; leftIndex < leftSize; listIndex++, leftIndex++) {
        list->cells[listIndex].value = leftArray[leftIndex].value;
    }
    for (; rightIndex < rightSize; listIndex++, rightIndex++) {
        list->cells[listIndex].value = rightArray[rightIndex].value;
    }
}

void list_mergeSort(List *list) {
    const ListIndex toIndex = list_getRear(list);
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

    list_poolInit(list, 0, toIndex);
    if (toIndex == list->size - 1) {
        list->cells[toIndex].next = 0;
    } else {
        list->cells[toIndex + 1].prev = toIndex;
    }

    ListIndex rear = list_getRear(list);
    if (list_getValue(list, rear) == 0) {
        while (list_getValue(list, rear) == 0) {
            rear--;
        }
    }

    if (rear == list->size - 1) {
        list_setFirstLastFree(list, 0, 0);
    } else {
        list_setFirstLastFree(list, rear + 1, list->size - 1);
    }

    list_setFront(list, 1);
    list_setRear(list, rear);
    list->cells[rear].next = 0;
    if (rear < list->size - 1) {
        list->cells[rear + 1].prev = 0;
    }
}

ListIndex list_binarySearch(const List *list, const ListValue value) {
    ListIndex from = 0, to = list_getRear(list);

    while (from <= to) {
        const ListIndex middle = from + ((to - from) / 2);

        if (value == list->cells[middle].value) {
            return middle;
        } else if (value > list->cells[middle].value) {
            from = middle + 1;
        } else if (middle == 0) {
            return 0;
        } else {
            to = middle - 1;
        }
    }

    return 0;
}
