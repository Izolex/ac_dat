#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dat.h"
#include "memory.h"
#include "tail.h"
#include "list.h"


#define TRIE_CHILDREN_LIST_INIT_SIZE 4


static TrieIndex createState(Character character, TrieBase base);
static size_t getListSize(const List *list);
static void trie_setCheck(Trie *trie, TrieIndex index, TrieIndex value);
static void trie_setBase(Trie *trie, TrieIndex index, TrieBase value);
static void trie_setChildren(Trie *trie, TrieIndex index, List *children);
static void trie_poolInit(Trie *trie, TrieIndex fromIndex, TrieIndex toIndex);
static void trie_poolReallocate(Trie *trie, TrieIndex newSize);
static void trie_poolCheckCapacity(Trie *trie, TrieIndex index);
static void trie_allocateCell(Trie *trie, TrieIndex cell);
static void trie_freeCell(Trie *trie, TrieIndex cell);
static void trie_insertNode(Trie *trie, TrieIndex state, TrieBase base, TrieIndex check);
static void trie_insertEndOfText(Trie *trie, TrieIndex check);
static void trie_insertBranch(Trie *trie, TrieIndex state, TrieIndex check, const Needle *needle, NeedleIndex needleIndex);
static void trie_collisionInTail(Trie *trie, TrieIndex state, TrieIndex check, const Needle *needle, NeedleIndex needleIndex);
static TrieIndex trie_collisionInArray(Trie *trie, TrieIndex state, TrieBase base, TrieIndex check, Character character);
static TrieIndex trie_moveBase(Trie *trie, TrieBase oldBase, TrieBase freeBase, TrieIndex check, TrieIndex state);
static TrieIndex trie_findEmptyCell(const Trie *trie, TrieIndex node);
static TrieIndex trie_findFreeBase(const Trie *trie, TrieIndex node);
static TrieIndex trie_storeCharacter(Trie *trie, TrieIndex lastState, TrieBase newNodeBase, Character character);
static TrieIndex trie_storeNeedle(Trie *trie, TrieIndex lastState, const Needle *needle, NeedleIndex needleIndex);


static TrieIndex createState(const Character character, const TrieBase base) {
    TrieIndex state = 0;
    if (unlikely(add_overflow(character, base, &state))) {
        fprintf(stderr, "index reached maximum value");
        exit(EXIT_FAILURE);
    }
    return state;
}


TrieOptions *createTrieOptions(const bool useTail) {
    TrieOptions *options = safeMalloc(sizeof(TrieOptions), "TrieOptions");

    options->useTail = useTail;

    return options;
}

void trieOptions_free(TrieOptions *options) {
    free(options);
    options = NULL;
}


static void trie_poolInit(Trie *trie, const TrieIndex fromIndex, const TrieIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        trie->cells[i] = (TrieCell) {-(i - 1), -(i + 1), NULL};
    }
}

Trie *createTrie(TrieOptions *options, TailBuilder *tailBuilder, const TrieIndex initialSize) {
    if (unlikely(initialSize < 4)) {
        fprintf(stderr, "minimum initial DAT size must be at least 4");
        exit(1);
    }

    Trie *trie = safeMalloc(sizeof(Trie), "Trie");

    trie->options = options;
    trie->tailBuilder = tailBuilder;
    trie->size = initialSize;
    trie->cells = safeMalloc(trie->size * sizeof(TrieCell), "Trie cells");
    trie->cells[0] = (TrieCell) {-(initialSize - 1), -2, NULL}; // TRIE_POOL_INFO
    trie->cells[1] = (TrieCell) {1, 0, createList(TRIE_CHILDREN_LIST_INIT_SIZE)}; // TRIE_POOL_START
    trie->cells[2] = (TrieCell) {0, -3, NULL};

    trie_poolInit(trie, 3, trie->size);

    trie->cells[initialSize - 1].check = 0;

    return trie;
}

void trie_free(Trie *trie) {
    for (TrieIndex i = 0; i < trie->size; i++) {
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
    return likely(index < trie->size) ? trie->cells[index].base : 0;
}

TrieIndex trie_getCheck(const Trie *trie, const TrieIndex index) {
    return likely(index < trie->size) ? trie->cells[index].check : 0;
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

    trie_poolInit(trie, trie->size, newSize);

    trie->cells[-trie->cells[0].base].check = -trie->size;
    trie->cells[0].base = -(newSize - 1);
    trie->cells[newSize - 1].check = 0;

    trie->size = newSize;
}

static void trie_poolCheckCapacity(Trie *trie, const TrieIndex index) {
    if (unlikely(trie->size <= index + 1)) {
        trie_poolReallocate(trie, (TrieIndex)calculateAllocation(index));
    }
}

static void trie_allocateCell(Trie *trie, const TrieIndex cell) {
    const TrieBase base = trie_getBase(trie, cell);
    const TrieIndex check = trie_getCheck(trie, cell);

    trie_setBase(trie, -check, base);
    trie_setCheck(trie, -base, check);
}

void trie_freeCell(Trie *trie, const TrieIndex cell) {
    TrieIndex next = -trie_getCheck(trie, TRIE_POOL_START);
    while (likely(next != TRIE_POOL_START && next < cell)) {
        next = -trie_getCheck(trie, next);
    }

    const TrieIndex prev = -trie_getBase(trie, next);

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

    const TrieBase checkBase = trie_getBase(trie, check);
    const Character character = state - checkBase;

    List *children = trie_getChildren(trie, check);

    if (children == NULL) {
        children = createList(TRIE_CHILDREN_LIST_INIT_SIZE);
        trie_setChildren(trie, check, children);
    }

    list_insert(children, character);
}

static TrieIndex trie_findEmptyCell(const Trie *trie, const TrieIndex node) {
    TrieIndex state = -trie_getCheck(trie, 0);
    while (likely(state != 0 && state <= node)) {
        state = -trie_getCheck(trie, state);
    }

    if (unlikely(state == 0)) {
        state = trie->size;
    }

    return state;
}

static TrieIndex trie_findFreeBase(const Trie *trie, const TrieIndex node) {
    const TrieIndex lastCell = -trie_getBase(trie, TRIE_POOL_INFO);
    const List *list = trie_getChildren(trie, node);

    TrieIndex emptyCell = trie_findEmptyCell(trie, node);
    TrieBase base = emptyCell - node;
    ListIndex listIndex;

    SEARCH:
    listIndex = list_iterate(list, 0);
    while (likely(listIndex > 0)) {
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

static TrieIndex trie_moveBase(
        Trie *trie,
        const TrieBase oldBase,
        const TrieBase freeBase,
        const TrieIndex check,
        const TrieIndex state
) {
    TrieIndex nextState = state;
    const List *checkChildren = trie_getChildren(trie, check);

    ListIndex checkListIndex = list_iterate(checkChildren, 0);
    while (likely(checkListIndex > 0)) {
        const Character character = list_getValue(checkChildren, checkListIndex);
        const TrieIndex charIndex = createState(character, oldBase);
        const TrieIndex newCharIndex = createState(character, freeBase);
        const TrieBase charBase = trie_getBase(trie, charIndex);

        if (charIndex == nextState) {
            nextState = newCharIndex;
        }

        const List *charChildren = trie_getChildren(trie, charIndex);
        if (charChildren != NULL) {
            ListIndex charListIndex = list_iterate(charChildren, 0);
            while (charListIndex > 0) {
                const TrieIndex s = createState(list_getValue(charChildren, charListIndex), charBase);
                trie_setCheck(trie, s, newCharIndex);
                charListIndex = list_iterate(charChildren, charListIndex);
            }
        }

        List *nodeChildren = trie_getChildren(trie, charIndex);
        trie_setChildren(trie, charIndex, NULL);

        trie_freeCell(trie, charIndex);
        trie_insertNode(trie, newCharIndex, charBase, check);

        trie_setChildren(trie, newCharIndex, nodeChildren);

        checkListIndex = list_iterate(checkChildren, checkListIndex);
    }

    return nextState;
}

static size_t getListSize(const List *list) {
    return list == NULL ? 0 : list_getRear(list);
}

static TrieIndex trie_collisionInArray(
        Trie *trie,
        const TrieIndex state,
        const TrieBase newNodeBase,
        const TrieIndex check,
        const Character character
) {
    TrieIndex parentIndex;
    List *baseChildren = trie_getChildren(trie, check);

    const List *checkChildren = trie_getChildren(trie, trie_getCheck(trie, state));
    const bool isBaseCollision = getListSize(baseChildren) + 1 < getListSize(checkChildren);

    if (isBaseCollision) {
        parentIndex = check;
        if (baseChildren == NULL) {
            baseChildren = createList(TRIE_CHILDREN_LIST_INIT_SIZE);
            trie_setChildren(trie, check, baseChildren);
        }
        list_push(baseChildren, character);
    } else {
        parentIndex = trie_getCheck(trie, state);
    }

    const TrieBase tempBase = trie_getBase(trie, parentIndex);
    const TrieBase freeBase = trie_findFreeBase(trie, parentIndex);
    const TrieIndex newState = isBaseCollision ? createState(character, freeBase) : state;

    if (isBaseCollision) {
        list_pop(baseChildren);
    }

    trie_setBase(trie, parentIndex, freeBase);
    const TrieIndex newNodeCheck = trie_moveBase(trie, tempBase, freeBase, parentIndex, check);
    trie_insertNode(trie, newState, newNodeBase, newNodeCheck);

    return newState;
}

static TrieIndex trie_storeCharacter(
        Trie *trie,
        const TrieIndex lastState,
        const TrieBase newNodeBase,
        const Character character
) {
    const TrieBase lastBase = trie_getBase(trie, lastState);
    const TrieIndex newState = createState(character, lastBase);
    const TrieBase newStateBase = trie_getBase(trie, newState);
    const TrieIndex check = trie_getCheck(trie, newState);

    if (check <= 0) {
        trie_insertNode(trie, newState, newNodeBase, lastState);
        return newState;
    } else if (check != lastState || newStateBase < 0) {
        return trie_collisionInArray(trie, newState, newNodeBase, lastState, character);
    } else {
        return newState;
    }
}

static TrieIndex trie_storeNeedle(
        Trie *trie,
        const TrieIndex lastState,
        const Needle *needle,
        const NeedleIndex needleIndex
) {
    const TrieBase lastBase = trie_getBase(trie, lastState);
    const Character character = needle->characters[needleIndex];
    const TrieIndex newState = createState(character, lastBase);

    const TrieBase base = trie_getBase(trie, newState);
    const TrieIndex check = trie_getCheck(trie, newState);

    if (check <= 0) {
        if (trie->options->useTail) {
            trie_insertBranch(trie, newState, lastState, needle, needleIndex);
            return 0;
        } else {
            trie_insertNode(trie, newState, 1, lastState);
            return newState;
        }
    } else if (check != lastState) {
        return trie_collisionInArray(trie, newState, 1, lastState, character);
    } else if (base < 0 && trie->options->useTail) {
        trie_collisionInTail(trie, newState, lastState, needle, needleIndex);
        return 0;
    } else {
        return newState;
    }
}

static void trie_insertEndOfText(Trie *trie, const TrieIndex check) {
    trie_storeCharacter(trie, check, trie_getBase(trie, check), END_OF_TEXT);
}

static void trie_insertBranch(
        Trie *trie,
        const TrieIndex state,
        const TrieIndex check,
        const Needle *needle,
        const NeedleIndex needleIndex
) {
    TrieBase newNodeBase = 1;
    const TailCharIndex tailCharsLength = needle->length - needleIndex - 1;

    if (likely(tailCharsLength > 0)) {
        Character *chars = allocateCharacters(tailCharsLength);

        TailCharIndex c = 0;
        NeedleIndex i = needleIndex + 1;
        for (; i < needle->length; i++, c++) {
            chars[c] = needle->characters[i];
        }

        newNodeBase = -tailBuilder_insertChars(trie->tailBuilder, tailCharsLength, chars);
    }

    trie_insertNode(trie, state, newNodeBase, check);
    if (likely(newNodeBase > 0)) {
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
    const TrieIndex tailIndex = -trie_getBase(trie, state);
    const TailBuilderCell tailBuilderCell = trie->tailBuilder->cells[tailIndex];


    TailCharIndex commonTailLength = 0;
    NeedleIndex needleIterator = needleIndex + 1;
    while (commonTailLength < tailBuilderCell.length
        && needleIterator < needle->length
        && tailBuilderCell.chars[commonTailLength] == needle->characters[needleIterator]
    ) {
        commonTailLength++, needleIterator++;
    }

    if (commonTailLength == tailBuilderCell.length && needleIterator == needle->length) {
        return;
    }


    TrieBase base = trie_getBase(trie, check);
    trie_setBase(trie, state, base);
    for (TailCharIndex i = 0; i < commonTailLength; i++) {
        state = trie_storeCharacter(trie, state, base, tailBuilderCell.chars[i]);
        base = trie_getBase(trie, state);
    }


    TrieBase needleNodeBase = base;
    if ((needle->length - needleIndex - commonTailLength - 1) > 1) {
        const TailCharIndex tailCharsLength = needle->length - needleIndex - commonTailLength - 2;
        Character *chars = allocateCharacters(tailCharsLength);

        TailCharIndex c = 0;
        NeedleIndex i = needleIndex + commonTailLength + 2;
        for (; i < needle->length; i++, c++) {
            chars[c] = needle->characters[i];
        }

        needleNodeBase = -tailBuilder_insertChars(trie->tailBuilder, tailCharsLength, chars);
    }
    state = trie_storeCharacter(trie, state, needleNodeBase, needle->characters[needleIndex + commonTailLength + 1]);
    if (needleNodeBase > 0) {
        trie_insertEndOfText(trie, state);
    }
    state = trie_getCheck(trie, state);


    TrieBase tailNodeBase = base;
    if (commonTailLength + 1 < tailBuilderCell.length) {
        const TailCharIndex tailCharsLength = tailBuilderCell.length - commonTailLength - 1;
        Character *chars = allocateCharacters(tailCharsLength);

        TailCharIndex i = commonTailLength + 1;
        NeedleIndex c = 0;
        for (; i < tailBuilderCell.length; i++, c++) {
            chars[c] = tailBuilderCell.chars[i];
        }

        tailNodeBase = -tailBuilder_insertChars(trie->tailBuilder, tailCharsLength, chars);
    }
    if (commonTailLength < tailBuilderCell.length) {
        state = trie_storeCharacter(trie, state, tailNodeBase, tailBuilderCell.chars[commonTailLength]);
    }
    if (tailNodeBase > 0) {
        trie_insertEndOfText(trie, state);
    }


    tailBuilder_freeCell(trie->tailBuilder, tailIndex);
}

void trie_addNeedle(Trie *trie, const Needle *needle) {
    TrieIndex lastState = TRIE_POOL_START;

    for (NeedleIndex i = 0; i < needle->length; i++) {
        lastState = trie_storeNeedle(trie, lastState, needle, i);
        if (0 == lastState) {
            return;
        }
    }

    trie_insertEndOfText(trie, lastState);
}
