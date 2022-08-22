#include <stdlib.h>
#include <stdio.h>
#include "dat.h"
#include "memory.h"


static void tail_poolReallocate(Tail *tail, TailIndex newSize);
static void tail_poolConnectNextFree(Tail *tail, TailIndex fromIndex, TailIndex toIndex);
static void trieChars_free(TrieChar *chars);


TrieChar *allocateTrieChars(const TailCharIndex size) {
    return safeCalloc(size, sizeof(TrieChar), "Tail characters");
}

static void trieChars_free(TrieChar *chars) {
    free(chars);
    chars = NULL;
}

static void tail_poolConnectNextFree(Tail *tail, const TailIndex fromIndex, const TailIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        tail->cells[i].chars = NULL;
        tail->cells[i].length = 0;
        tail->cells[i].nextFree = i + 1; // next empty cell
    }
}

Tail *createTail(const TailIndex size) {
    Tail *tail = safeCalloc(1, sizeof(Tail), "Tail");

    if (size < 2) {
        fprintf(stderr, "minimum initial tail size must be at least 2");
        exit(1);
    }

    tail->cellsSize = size;
    tail->cells = safeCalloc(tail->cellsSize, sizeof(TailCell), "Tail cells");

    tail_poolConnectNextFree(tail, 0, tail->cellsSize);

    return tail;
}

void tail_free(Tail *tail) {
    for (TailIndex i = 1; i < tail->cellsSize; i++) {
        if (tail->cells[i].chars != NULL) {
            trieChars_free(tail->cells[i].chars);
        }
    }
    free(tail->cells);
    free(tail);
    tail = NULL;
}

void tail_poolReallocate(Tail *tail, const TailIndex newSize) {
    tail->cells = safeRealloc(tail->cells, newSize, sizeof(TailCell), "Tail cells");

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

TailIndex tail_insertChars(Tail *tail, const TailCharIndex length, TrieChar *string) {
    TailIndex index = tail->cells[0].nextFree;

    if (index == 0) {
        index = tail->cellsSize;
        tail_poolReallocate(tail, (TailIndex)calculateAllocation(tail->cellsSize));
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

void tail_minimize(Tail *tail) {
    TailIndex lastFilled = tail->cellsSize - 1;
    while (tail->cells[lastFilled].chars == NULL && lastFilled > 1) {
        lastFilled--;
    }

    TailIndex newSize = lastFilled + 1;
    tail->cellsSize = newSize;
    tail->cells = safeRealloc(tail->cells, newSize, sizeof(TailCell), "Tail cells");
}