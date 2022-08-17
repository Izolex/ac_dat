#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "dat.h"

void tail_poolConnectNextFree(Tail *tail, TailIndex fromIndex, TailIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        tail->cells[i].chars = NULL;
        tail->cells[i].length = 0;
        tail->cells[i].nextFree = i + 1; // next empty cell
    }
}

Tail *create_tail(long int size) {
    Tail *tail = malloc(sizeof(Tail));

    if (size < 2) {
        fprintf(stderr, "minimum initial tail size must be at least 2");
        exit(1);
    }

    tail->cellsSize = size;
    tail->cells = malloc(sizeof(TailCell) * tail->cellsSize);

    tail_poolConnectNextFree(tail, 0, tail->cellsSize);

    return tail;
}

void tail_poolReallocate(Tail *tail, TailIndex newSize) {
    tail->cells = realloc(tail->cells, sizeof(TailCell) * newSize);

    tail_poolConnectNextFree(tail, tail->cellsSize, newSize);

    tail->cells[tail->cellsSize - 1].nextFree = tail->cellsSize;
    tail->cells[newSize - 1].nextFree = 0;
    tail->cellsSize = newSize;
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

TailIndex tail_insertChars(Tail *tail, const long int length, TrieChar *string) {
    TailIndex index = tail->cells[0].nextFree;

    if (index == 0) {
        index = tail->cellsSize;
        tail_poolReallocate(tail, index + (long int)ceill(((long double)tail->cellsSize / 2)));
    }

    tail->cells[0].nextFree = tail->cells[index].nextFree;

    tail->cells[index].length = length;
    tail->cells[index].chars = string;
    tail->cells[index].nextFree = 0;

    return index;
}

TailCell tail_getCell(Tail *tail, TailIndex index) {
    return tail->cells[index];
}

void tail_print(Tail *tail) {
    for (int i = 0; i < tail->cellsSize; i++) {
        TailCell cell = tail->cells[i];
        printf("%d (%ld): ", i, cell.nextFree);
        if (cell.length > 0) {
            for (int c = 0; c < cell.length; c++) {
                printf("%ld ", cell.chars[c]);
            }
        } else {
            printf("empty");
        }
        printf("\n");
    }
    printf("\n\n");
}