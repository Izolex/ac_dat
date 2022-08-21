#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "dat.h"


static void tail_poolReallocate(Tail *tail, TailIndex newSize);
static void tail_poolConnectNextFree(Tail *tail, TailIndex fromIndex, TailIndex toIndex);


static void tail_poolConnectNextFree(Tail *tail, const TailIndex fromIndex, const TailIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        tail->cells[i].chars = NULL;
        tail->cells[i].length = 0;
        tail->cells[i].nextFree = i + 1; // next empty cell
    }
}

Tail *create_tail(const TailIndex size) {
    Tail *tail = calloc(1, sizeof(Tail));
    if (tail == NULL) {
        fprintf(stderr, "can not allocate %lu memory for tail", sizeof(Tail));
        exit(1);
    }

    if (size < 2) {
        fprintf(stderr, "minimum initial tail size must be at least 2");
        exit(1);
    }

    tail->cellsSize = size;
    tail->cells = calloc(tail->cellsSize, sizeof(TailCell));
    if (tail == NULL) {
        fprintf(stderr, "can not allocate %lu memory for tail cells", sizeof(TailCell) * tail->cellsSize);
        exit(1);
    }

    tail_poolConnectNextFree(tail, 0, tail->cellsSize);

    return tail;
}

void tail_poolReallocate(Tail *tail, const TailIndex newSize) {
    tail->cells = realloc(tail->cells, sizeof(TailCell) * newSize);
    if (tail->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for tail cells", sizeof(TailCell) * newSize);
        exit(1);
    }

    tail_poolConnectNextFree(tail, tail->cellsSize, newSize);

    tail->cells[tail->cellsSize - 1].nextFree = tail->cellsSize;
    tail->cells[newSize - 1].nextFree = 0;
    tail->cellsSize = newSize;
}

void tail_freeCell(Tail *tail, const TailIndex index) {
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

TailIndex tail_insertChars(Tail *tail, const size_t length, TrieChar *string) {
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

TailCell tail_getCell(const Tail *tail, const TailIndex index) {
    return tail->cells[index];
}
