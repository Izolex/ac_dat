#include <stdlib.h>
#include <stdio.h>
#include "dat.h"
#include "memory.h"


static void tailBuilder_poolReallocate(TailBuilder *tailBuilder, TailIndex newSize);
static void tailBuilder_poolInit(TailBuilder *tailBuilder, TailIndex fromIndex, TailIndex toIndex);
static void characters_free(Character *chars);


Character *allocateCharacters(const TailCharIndex size) {
    return safeMalloc(size * sizeof(Character), "Characters");
}

static void characters_free(Character *chars) {
    free(chars);
    chars = NULL;
}

static void tailBuilder_poolInit(TailBuilder *tailBuilder, const TailIndex fromIndex, const TailIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        tailBuilder->cells[i].chars = NULL;
        tailBuilder->cells[i].length = 0;
        tailBuilder->cells[i].nextFree = i + 1; // next empty cell
    }
}

TailBuilder *createTailBuilder(const TailIndex size) {
    TailBuilder *tailBuilder = safeMalloc(sizeof(TailBuilder), "TailBuilder");

    if (size < 2) {
        fprintf(stderr, "minimum initial tail size must be at least 2");
        exit(1);
    }

    tailBuilder->size = size;
    tailBuilder->cells = safeMalloc(tailBuilder->size * sizeof(TailBuilderCell), "TailBuilder cells");

    tailBuilder_poolInit(tailBuilder, 0, tailBuilder->size);

    tailBuilder->cells[tailBuilder->size - 1].nextFree = 0;

    return tailBuilder;
}

void tailBuilder_free(TailBuilder *tailBuilder) {
    for (TailIndex i = 1; i < tailBuilder->size; i++) {
        if (tailBuilder->cells[i].chars != NULL) {
            characters_free(tailBuilder->cells[i].chars);
        }
    }
    free(tailBuilder->cells);
    free(tailBuilder);
    tailBuilder = NULL;
}

static TailIndex tailBuilder_findPrevFree(TailBuilder *tailBuilder, const TailIndex index) {
    TailIndex lastFree = tailBuilder->cells[0].nextFree;
    if (lastFree) {
        TailIndex nextFree = tailBuilder->cells[lastFree].nextFree;
        while (nextFree && nextFree < index) {
            lastFree = tailBuilder->cells[lastFree].nextFree;
            nextFree = tailBuilder->cells[lastFree].nextFree;
        }
    }

    return lastFree;
}

void tailBuilder_poolReallocate(TailBuilder *tailBuilder, const TailIndex newSize) {
    tailBuilder->cells = safeRealloc(tailBuilder->cells, newSize, sizeof(TailBuilderCell), "TailBuilder cells");

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

TailBuilderCell tailBuilder_getCell(const TailBuilder *tailBuilder, const TailIndex index) {
    return tailBuilder->cells[index];
}

void tailBuilder_minimize(TailBuilder *tailBuilder) {
    TailIndex lastFilled = tailBuilder->size - 1;
    while (tailBuilder->cells[lastFilled].chars == NULL && lastFilled > 1) {
        lastFilled--;
    }

    const TailIndex newSize = lastFilled + 1;
    tailBuilder->size = newSize;
    tailBuilder->cells = safeRealloc(tailBuilder->cells, newSize, sizeof(TailBuilderCell), "TailBuilder cells");
    tailBuilder->cells[0].nextFree = 0;
}