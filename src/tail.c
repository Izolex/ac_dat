#include <stdlib.h>
#include <stdio.h>
#include "dat.h"

const int TAIL_ALLOCATION_STEP = 2;

Tail *create_tail() {
    Tail *tail = malloc(sizeof(Tail));

    tail->cellsSize = 1;
    tail->cells = malloc(sizeof(TailCell) * 1);
    tail->cells[0] = (TailCell) {NULL, 0, 0};

    return tail;
}

void tail_poolConnectNextFree(Tail *tail, TailIndex fromIndex, TailIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        tail->cells[i].chars = NULL;
        tail->cells[i].length = 0;
        tail->cells[i].nextFree = i + 1; // next empty cell
    }
}

void tail_poolReallocate(Tail *tail, TailIndex newSize) {
    tail->cells = realloc(tail->cells, sizeof(TailCell) * newSize);

    tail_poolConnectNextFree(tail, tail->cellsSize, newSize);

    tail->cells[tail->cellsSize - 1].nextFree = tail->cellsSize;
    tail->cells[newSize - 1].nextFree = 0;
    tail->cellsSize = newSize;
}

void tail_poolCheckCapacity(Tail *tail, TailIndex index) {
    if (tail->cellsSize <= index + 1) {
        tail_poolReallocate(tail, index + 1 + TAIL_ALLOCATION_STEP);
    }
}

void tail_freeCell(Tail *tail, TailIndex index) {
    free(tail->cells[index].chars);

    tail->cells[index].chars = NULL;
    tail->cells[index].length = 0;

    if (tail->cells[0].nextFree > index) {
        tail->cells[index].nextFree = tail->cells[0].nextFree;
        tail->cells[0].nextFree = index;
    } else if (tail->cells[0].nextFree == 0) {
        tail->cells[index].nextFree = 0;
        tail->cells[0].nextFree = index;
    } else {
        TailIndex prevFree = index;
        while (tail->cells[prevFree].nextFree != 0) {
            prevFree--;
        }

        tail->cells[index].nextFree = tail->cells[prevFree].nextFree;
        tail->cells[prevFree].nextFree = index;
    }
}

TailIndex tail_insertChars(Tail *tail, const int length, TrieChar *string) {
    TailIndex index = tail->cells[0].nextFree;

    if (index == 0) {
        index = tail->cellsSize;
        tail_poolCheckCapacity(tail, index + 1);
    }

    tail->cells[0].nextFree = tail->cells[index].nextFree;

    tail->cells[index].length = length;
    tail->cells[index].chars = string;
    tail->cells[index].nextFree = 0;

    return index;
}

TrieChar *tail_allocateChars(TrieIndex size) {
    TrieChar *chars = malloc(sizeof(TrieChar) * size);
    if (chars == NULL) {
        fprintf(stderr, "can not allocate %lu memory for tail chars", sizeof(TrieChar) * size);
        exit(1);
    }

    return chars;
}

void tail_print(Tail *tail) {
    for (int i = 0; i < tail->cellsSize; i++) {
        TailCell cell = tail->cells[i];
        printf("%d (%u): ", i, cell.nextFree);
        if (cell.length > 0) {
            for (int c = 0; c < cell.length; c++) {
                printf("%d ", cell.chars[c]);
            }
        } else {
            printf("empty");
        }
        printf("\n");
    }
    printf("\n\n");
}