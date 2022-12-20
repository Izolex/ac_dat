#include <stdlib.h>
#include <stdio.h>
#include "dat.h"
#include "tail.h"
#include "memory.h"


static void tailBuilder_poolReallocate(TailBuilder *tailBuilder, TailIndex newSize);
static void tailBuilder_poolInit(TailBuilder *tailBuilder, TailIndex fromIndex, TailIndex toIndex);
static void characters_free(Character *chars);
static TailIndex tailBuilder_findLastFilled(const TailBuilder *tailBuilder);


Character *allocateCharacters(const TailCharIndex size) {
    return safeAlloc(size * sizeof(Character), "Characters");
}

static void characters_free(Character *chars) {
    free(chars);
    chars = NULL;
}


Tail *createTail(const size_t size) {
    Tail *tail = safeAlloc(sizeof(Tail), "Tail");
    tail->size = (TailIndex)size;
    tail->cells = safeAlloc(sizeof(TailCell) * tail->size, "Tail cells");

    return tail;
}

Tail *createTailFromBuilder(const TailBuilder *tailBuilder) {
    const TailIndex lastFilled = tailBuilder_findLastFilled(tailBuilder);

    Tail *tail = createTail(lastFilled + 1);

    for (TailIndex i = 0; i < tail->size; i++) {
        tail->cells[i].length = tailBuilder->cells[i].length;
        tail->cells[i].chars = tailBuilder->cells[i].chars;
    }

    return tail;
}

TailCell tail_getCell(const Tail *tail, const TailIndex index) {
    return tail->cells[index];
}

void tail_free(Tail *tail) {
    for (TailIndex i = 1; i < tail->size; i++) {
        if (likely(tail->cells[i].chars != NULL)) {
            characters_free(tail->cells[i].chars);
        }
    }
    free(tail->cells);
    free(tail);
    tail = NULL;
}


static void tailBuilder_poolInit(TailBuilder *tailBuilder, const TailIndex fromIndex, const TailIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        tailBuilder->cells[i] = (TailBuilderCell) {NULL, 0, i + 1};
    }
}

TailBuilder *createTailBuilder(const size_t size) {
    TailBuilder *tailBuilder = safeAlloc(sizeof(TailBuilder), "TailBuilder");

    if (size < 2) {
        error("minimum initial tail size must be at least 2");
    }

    tailBuilder->size = (TailIndex)size;
    tailBuilder->cells = safeAlloc(tailBuilder->size * sizeof(TailBuilderCell), "TailBuilder cells");

    tailBuilder_poolInit(tailBuilder, 0, tailBuilder->size);

    tailBuilder->cells[tailBuilder->size - 1].nextFree = 0;

    return tailBuilder;
}

void tailBuilder_freeCharacters(TailBuilder *tailBuilder) {
    for (TailIndex i = 1; i < tailBuilder->size; i++) {
        if (tailBuilder->cells[i].chars != NULL) {
            characters_free(tailBuilder->cells[i].chars);
        }
    }
}

void tailBuilder_free(TailBuilder *tailBuilder) {
    free(tailBuilder->cells);
    free(tailBuilder);
    tailBuilder = NULL;
}

static TailIndex tailBuilder_findPrevFree(TailBuilder *tailBuilder, const TailIndex index) {
    TailIndex lastFree = tailBuilder->cells[0].nextFree;
    if (lastFree) {
        TailIndex nextFree = tailBuilder->cells[lastFree].nextFree;
        while (likely(nextFree && nextFree < index)) {
            lastFree = tailBuilder->cells[lastFree].nextFree;
            nextFree = tailBuilder->cells[lastFree].nextFree;
        }
    }

    return lastFree;
}

void tailBuilder_poolReallocate(TailBuilder *tailBuilder, const TailIndex newSize) {
    tailBuilder->cells = safeRealloc(tailBuilder->cells, tailBuilder->size, newSize, sizeof(TailBuilderCell), "TailBuilder cells");

    tailBuilder_poolInit(tailBuilder, tailBuilder->size, newSize);

    const TailIndex prevFree = tailBuilder_findPrevFree(tailBuilder, tailBuilder->size - 1);

    tailBuilder->cells[prevFree].nextFree = tailBuilder->size;
    tailBuilder->cells[newSize - 1].nextFree = 0;
    tailBuilder->size = newSize;
}

void tailBuilder_freeCell(TailBuilder *tail, const TailIndex index) {
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
        const TailIndex prevFree = tailBuilder_findPrevFree(tail, index);

        tail->cells[index].nextFree = tail->cells[prevFree].nextFree;
        tail->cells[prevFree].nextFree = index;
    }
}

TailIndex tailBuilder_insertChars(TailBuilder *tail, const TailCharIndex length, Character *string) {
    TailIndex index = tail->cells[0].nextFree;

    if (index == 0) {
        index = tail->size;
        tailBuilder_poolReallocate(tail, (TailIndex)calculateAllocation(tail->size));
    }

    tail->cells[0].nextFree = tail->cells[index].nextFree;

    tail->cells[index].length = length;
    tail->cells[index].chars = string;
    tail->cells[index].nextFree = 0;

    return index;
}

void tailBuilder_minimize(TailBuilder *tailBuilder) {
    const TailIndex newSize = tailBuilder_findLastFilled(tailBuilder) + 1;

    tailBuilder->cells = safeRealloc(tailBuilder->cells, tailBuilder->size, newSize, sizeof(TailBuilderCell), "TailBuilder cells");
    tailBuilder->size = newSize;
    tailBuilder->cells[0].nextFree = 0;
}

static TailIndex tailBuilder_findLastFilled(const TailBuilder *tailBuilder) {
    TailIndex lastFilled = tailBuilder->size - 1;
    while (likely(tailBuilder->cells[lastFilled].chars == NULL && lastFilled > 1)) {
        lastFilled--;
    }

    return lastFilled;
}
