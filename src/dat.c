#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dat.h"
#include "memory.h"
#include "tail.h"
#include "list.h"


#define TRIE_CHILDREN_LIST_INIT_SIZE 4


static void trie_setCheck(Trie *trie, TrieIndex index, TrieIndex value);
static void trie_setBase(Trie *trie, TrieIndex index, TrieBase value);
static void trie_setChildren(Trie *trie, TrieIndex index, List *children);
static void trie_poolInit(Trie *trie, TrieIndex fromIndex, TrieIndex toIndex);
static void trie_poolReallocate(Trie *trie, TrieIndex newSize);
static void trie_poolCheckCapacity(Trie *trie, TrieIndex index);
static void trie_allocateCell(Trie *trie, TrieIndex cell);
static void trie_freeCell(Trie *trie, TrieIndex cell);
static void trie_insertNode(Trie *trie, TrieIndex state, TrieBase base, TrieIndex check);
static void trie_switchChildren(Trie *trie, TrieIndex source, TrieIndex target);
static void trie_insertEndOfText(Trie *trie, TrieIndex check);
static void trie_insertBranch(Trie *trie, TrieIndex state, TrieIndex check, const Needle *needle, NeedleIndex needleIndex);
static void trie_collisionInTail(Trie *trie, TrieIndex state, TrieIndex check, const Needle *needle, NeedleIndex needleIndex);
static TrieIndex trie_findEmptyCell(const Trie *trie, TrieIndex node);
static TrieIndex trie_findFreeBase(const Trie *trie, TrieIndex node);
static TrieIndex trie_insert(Trie *trie, TrieIndex lastState, TrieBase base, TrieChar character);
static TrieIndex trie_moveBase(Trie *trie, TrieBase oldBase, TrieBase freeBase, TrieIndex check, TrieIndex state);
static TrieIndex trie_solveCollision(Trie *trie, TrieIndex state, TrieBase base, TrieIndex check, TrieChar character);


static void trie_poolInit(Trie *trie, const TrieIndex fromIndex, const TrieIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        trie->cells[i].base = -(i - 1);
        trie->cells[i].check = -(i + 1);
        trie->cells[i].children = NULL;
    }
}

TrieOptions *createTrieOptions(const bool useTail) {
    TrieOptions *options = safeCalloc(1, sizeof(TrieOptions), "TrieOptions");

    options->useTail = useTail;

    return options;
}

void trieOptions_free(TrieOptions *options) {
    free(options);
    options = NULL;
}

Trie *createTrie(TrieOptions *options, Tail *tail, const TrieIndex initialSize) {
    Trie *trie = safeCalloc(1, sizeof(Trie), "Trie");

    if (initialSize < 4) {
        fprintf(stderr, "minimum initial DAT size must be at least 4");
        exit(1);
    }

    trie->options = options;
    trie->tail = tail;
    trie->cellsSize = initialSize;
    trie->cells = safeCalloc(trie->cellsSize, sizeof(TrieCell), "Trie cells");
    trie->cells[0] = (TrieCell) {-2, -2, NULL}; // TRIE_POOL_INFO
    trie->cells[1] = (TrieCell) {1, 0, createList(TRIE_CHILDREN_LIST_INIT_SIZE)}; // TRIE_POOL_START
    trie->cells[2] = (TrieCell) {0, -3, NULL};

    trie_poolInit(trie, 3, trie->cellsSize);

    trie->cells[initialSize - 1].check = 0;
    trie->cells[0].base = -(initialSize - 1);

    return trie;
}

void trie_free(Trie *trie) {
    for (TrieIndex i = 0; i < trie->cellsSize; i++) {
        List *children = trie_getChildren(trie, i);
        if (children != NULL) {
            list_free(children);
        }
    }
    free(trie->cells);
    free(trie);
    trie = NULL;
}

TrieBase trie_getBase(const Trie *trie, const TrieIndex index) {
    if (index < trie->cellsSize) {
        return trie->cells[index].base;
    }

    return 0;
}

TrieIndex trie_getCheck(const Trie *trie, const TrieIndex index) {
    if (index < trie->cellsSize) {
        return trie->cells[index].check;
    }

    return 0;
}

List *trie_getChildren(const Trie *trie, const TrieIndex index) {
    return trie->cells[index].children;
}

static void trie_setCheck(Trie *trie, const TrieIndex index, const TrieIndex value) {
    trie->cells[index].check = value;
}

static void trie_setBase(Trie *trie, const TrieIndex index, const TrieBase value) {
    trie->cells[index].base = value;
}

static void trie_setChildren(Trie *trie, const TrieIndex index, List *children) {
    trie->cells[index].children = children;
}

static void trie_poolReallocate(Trie *trie, const TrieIndex newSize) {
    trie->cells = safeRealloc(trie->cells, newSize, sizeof(TrieCell), "Trie");

    trie_poolInit(trie, trie->cellsSize, newSize);

    trie->cells[-trie->cells[0].base].check = -trie->cellsSize;
    trie->cells[0].base = -(newSize - 1);
    trie->cells[newSize - 1].check = 0;

    trie->cellsSize = newSize;
}

static void trie_poolCheckCapacity(Trie *trie, const TrieIndex index) {
    if (trie->cellsSize <= index + 1) {
        trie_poolReallocate(trie, (TrieIndex)calculateAllocation(index));
    }
}

static void trie_allocateCell(Trie *trie, const TrieIndex cell) {
    TrieBase base = trie_getBase(trie, cell);
    TrieIndex check = trie_getCheck(trie, cell);
    List *children = trie_getChildren(trie, cell);

    trie_setBase(trie, -check, base);
    trie_setCheck(trie, -base, check);

    if (children == NULL) {
        trie_setChildren(trie, cell, createList(TRIE_CHILDREN_LIST_INIT_SIZE));
    }
}

void trie_freeCell(Trie *trie, const TrieIndex cell) {
    TrieIndex next = -trie_getCheck(trie, TRIE_POOL_START);
    while (next != TRIE_POOL_START && next < cell) {
        next = -trie_getCheck(trie, next);
    }

    TrieIndex prev = -trie_getBase(trie, next);

    trie_setCheck(trie, cell, -next);
    trie_setBase(trie, cell, -prev);
    trie_setBase(trie, next, -cell);
    trie_setCheck(trie, prev, -cell);
}

static void trie_insertNode(
        Trie *trie,
        const TrieIndex state,
        const TrieBase base,
        const TrieIndex check
) {
    trie_poolCheckCapacity(trie, state);
    trie_allocateCell(trie, state);

    trie_setCheck(trie, state, check);
    trie_setBase(trie, state, base);

    List *checkChildren = trie_getChildren(trie, check);
    TrieBase checkBase = trie_getBase(trie, check);
    TrieChar character = state - checkBase;

    if (list_binarySearch(checkChildren, character) == 0) {
        list_insert(checkChildren, character);
    }
}

static TrieIndex trie_findEmptyCell(
        const Trie *trie,
        const TrieIndex node
) {
    TrieIndex state = -trie_getCheck(trie, 0);
    while (state != 0 && state <= node) {
        state = -trie_getCheck(trie, state);
    }

    if (state == 0) {
        state = trie->cellsSize;
    }

    return state;
}

static TrieIndex trie_findFreeBase(const Trie *trie, const TrieIndex node) {
    TrieIndex emptyCell = trie_findEmptyCell(trie, node);
    TrieIndex lastCell = -trie_getBase(trie, TRIE_POOL_INFO);
    TrieBase base = emptyCell - node;
    List *list = trie_getChildren(trie, node);
    ListIndex listIndex;

    SEARCH:
    listIndex = list_iterate(list, 0);
    while (listIndex > 0) {
        TrieIndex index = base + list_getValue(list, listIndex);
        if (trie_getCheck(trie, index) > 0) {
            if (lastCell == emptyCell) {
                base++;
            } else {
                emptyCell = trie_findEmptyCell(trie, emptyCell);
                base = emptyCell - node;
            }
            goto SEARCH;
        }
        listIndex = list_iterate(list, listIndex);
    }

    return base;
}

static void trie_switchChildren(Trie *trie, const TrieIndex source, const TrieIndex target) {
    trie_setChildren(trie, target, trie_getChildren(trie, source));
    trie_setChildren(trie, source, NULL);
}

static TrieIndex trie_moveBase(
        Trie *trie,
        const TrieBase oldBase,
        const TrieBase freeBase,
        const TrieIndex check,
        const TrieIndex state
) {
    TrieIndex nextState = state;
    List *checkChildren = trie_getChildren(trie, check);

    ListIndex checkListIndex = list_iterate(checkChildren, 0);
    while (checkListIndex > 0) {
        TrieChar character = list_getValue(checkChildren, checkListIndex);
        TrieIndex charIndex = oldBase + character;
        TrieIndex newCharIndex = freeBase + character;
        TrieBase charBase = trie_getBase(trie, charIndex);

        if (charIndex == nextState) {
            nextState = newCharIndex;
        }

        List *charChildren = trie_getChildren(trie, charIndex);
        ListIndex charListIndex = list_iterate(charChildren, 0);
        while (charListIndex > 0) {
            trie_setCheck(trie, charBase + list_getValue(charChildren, charListIndex), newCharIndex);
            charListIndex = list_iterate(charChildren, charListIndex);
        }

        trie_switchChildren(trie, charIndex, newCharIndex);
        trie_freeCell(trie, charIndex);
        trie_insertNode(trie, newCharIndex, charBase, check);

        checkListIndex = list_iterate(checkChildren, checkListIndex);
    }

    return nextState;
}

static TrieIndex trie_solveCollision(
        Trie *trie,
        const TrieIndex state,
        const TrieBase base,
        const TrieIndex check,
        const TrieChar character
) {
    List *baseChildren = trie_getChildren(trie, check);
    List *checkChildren = trie_getChildren(trie, trie_getCheck(trie, state));

    TrieBase freeBase;
    TrieIndex newState, parentIndex, newCheck = check;

    if (list_getRear(baseChildren) + 1 < list_getRear(checkChildren)) {
        parentIndex = check;

        list_insert(baseChildren, character);
        freeBase = trie_findFreeBase(trie, parentIndex);
        list_delete(baseChildren, list_binarySearch(baseChildren, character));

        newState = freeBase + character;
    } else {
        parentIndex = trie_getCheck(trie, state);

        freeBase = trie_findFreeBase(trie, parentIndex);

        newState = state;
    }

    TrieBase tempBase = trie_getBase(trie, parentIndex);
    trie_setBase(trie, parentIndex, freeBase);
    TrieIndex newNodeCheck = trie_moveBase(trie, tempBase, freeBase, parentIndex, newCheck);

    trie_insertNode(trie, newState, base, newNodeCheck);

    return newState;
}

static TrieIndex trie_insert(
        Trie *trie,
        const TrieIndex lastState,
        const TrieBase base,
        const TrieChar character
) {
    TrieBase lastBase = trie_getBase(trie, lastState);
    TrieIndex newState = character + lastBase;

    TrieBase newStateBase = trie_getBase(trie, newState);
    TrieIndex check = trie_getCheck(trie, newState);

    if (check <= 0) {
        trie_insertNode(trie, newState, base, lastState);
        return newState;
    } else if (check != lastState || newStateBase < 0) {
        return trie_solveCollision(trie, newState, base, lastState, character);
    } else {
        return newState;
    }
}

static void trie_insertEndOfText(
        Trie *trie,
        const TrieIndex check
) {
    TrieBase base = trie_getBase(trie, check);
    trie_insert(trie, check, base, END_OF_TEXT);
}

static void trie_insertBranch(
        Trie *trie,
        const TrieIndex state,
        const TrieIndex check,
        const Needle *needle,
        const NeedleIndex needleIndex
) {
    TrieBase newNodeBase = 1;
    TailCharIndex tailCharsLength = needle->length - needleIndex - 1;
    if (tailCharsLength > 0) {
        TrieChar *chars = allocateTrieChars(tailCharsLength);

        TailCharIndex c = 0;
        NeedleIndex i = needleIndex + 1;
        for (; i < needle->length; i++, c++) {
            chars[c] = needle->characters[i];
        }

        newNodeBase = -tail_insertChars(trie->tail, tailCharsLength, chars);
    }

    trie_insertNode(trie, state, newNodeBase, check);
    if (newNodeBase > 0) {
        trie_insertEndOfText(trie, state);
    }
}

static void trie_collisionInTail(
        Trie *trie,
        TrieIndex state,
        const TrieIndex check,
        const Needle *needle,
        const NeedleIndex needleIndex
) {
    TrieIndex tailIndex = -trie_getBase(trie, state);
    TailCell tailCell = trie->tail->cells[tailIndex];

    bool areSame = true;
    TailCharIndex commonTailLength = 0;
    for (NeedleIndex i = needleIndex + 1; i < needle->length; i++) {
        if (commonTailLength < tailCell.length && tailCell.chars[commonTailLength] == needle->characters[i]) {
            commonTailLength++;
        } else {
            areSame = false;
            break;
        }
    }

    if (areSame == true) {
        return;
    }


    TrieBase base = trie_getBase(trie, check);
    trie_setBase(trie, state, base);
    for (TailCharIndex i = 0; i < commonTailLength; i++) {
        state = trie_insert(trie, state, base, tailCell.chars[i]);
        base = trie_getBase(trie, state);
    }


    TrieBase needleNodeBase = base;
    if ((needle->length - needleIndex - commonTailLength - 1) > 1) {
        TailCharIndex tailCharsLength = needle->length - needleIndex - commonTailLength - 2;
        TrieChar *chars = allocateTrieChars(tailCharsLength);

        TailCharIndex c = 0;
        NeedleIndex i = needleIndex + commonTailLength + 2;
        for (; i < needle->length; i++, c++) {
            chars[c] = needle->characters[i];
        }

        needleNodeBase = -tail_insertChars(trie->tail, tailCharsLength, chars);
    }
    state = trie_insert(trie, state, needleNodeBase, needle->characters[needleIndex + commonTailLength + 1]);
    if (needleNodeBase > 0) {
        trie_insertEndOfText(trie, state);
    }
    state = trie_getCheck(trie, state);


    TrieBase tailNodeBase = base;
    if (commonTailLength + 1 < tailCell.length) {
        TailCharIndex tailCharsLength = tailCell.length - commonTailLength - 1;
        TrieChar *chars = allocateTrieChars(tailCharsLength);

        TailCharIndex i = commonTailLength + 1;
        NeedleIndex c = 0;
        for (; i < tailCell.length; i++, c++) {
            chars[c] = tailCell.chars[i];
        }

        tailNodeBase = -tail_insertChars(trie->tail, tailCharsLength, chars);
    }
    if (commonTailLength < tailCell.length) {
        state = trie_insert(trie, state, tailNodeBase, tailCell.chars[commonTailLength]);
    }
    if (tailNodeBase > 0) {
        trie_insertEndOfText(trie, state);
    }


    tail_freeCell(trie->tail, tailIndex);
}

void trie_addNeedle(Trie *trie, const Needle *needle) {
    TrieIndex lastState = TRIE_POOL_START;

    for (NeedleIndex i = 0; i < needle->length; i++) {
        TrieBase lastBase = trie_getBase(trie, lastState);
        TrieChar character = needle->characters[i];
        TrieIndex newState = character + lastBase;

        TrieBase base = trie_getBase(trie, newState);
        TrieIndex check = trie_getCheck(trie, newState);

        if (check <= 0) {
            if (trie->options->useTail) {
                trie_insertBranch(trie, newState, lastState, needle, i);
                return;
            } else {
                trie_insertNode(trie, newState, 1, lastState);
                lastState = newState;
            }
        } else if (check != lastState) {
            lastState = trie_solveCollision(trie, newState, 1, lastState, character);
        } else if (base < 0 && trie->options->useTail) {
            trie_collisionInTail(trie, newState, lastState, needle, i);
            return;
        } else {
            lastState = newState;
        }
    }

    trie_insertEndOfText(trie, lastState);
}

void trie_find(Trie *trie, Needle *needle) {
    TrieIndex state = TRIE_POOL_START;

    long int length = 0;
    for (long int i = 0; i < needle->length; i++) {
        TrieBase base = trie_getBase(trie, state);
        TrieChar character = needle->characters[i];
        TrieIndex newState = character + base;
        TrieIndex check = trie_getCheck(trie, newState);

        if (check == state) {
            state = newState;
        } else if (base < 0) {
            TailCell tailCell = trie->tail->cells[-base];
            long int tailIterator = 0;
            for (; tailIterator < tailCell.length; tailIterator++) {
                if (tailCell.chars[tailIterator] != needle->characters[length]) {
                    printf("can not search word (tail)\n");
                    exit(1);
                }
                length++;
            }
            if (length != needle->length || tailIterator != tailCell.length) {
                printf("can not search word (tail length)\n");
                exit(1);
            }
            return;
        } else {
            printf("can not search word\n");
            exit(1);
        }

        length++;
    }

    TrieBase base = trie_getBase(trie, state);
    TrieIndex newState = base + END_OF_TEXT;
    TrieIndex check = trie_getCheck(trie, newState);

    if (check != state) {
        printf("can not search word (EOT)\n");
        exit(1);
    }
}
