#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dat.h"
#include "memory.h"
#include "needle.h"
#include "tail.h"
#include "list.h"
#include "user_data.h"


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
static void trie_insertEndOfText(Trie *trie, TrieIndex check, UserData userData);
static void trie_insertBranch(Trie *trie, TrieIndex state, TrieIndex check, const TrieNeedle *needle, TrieNeedleIndex needleIndex, UserData userData);
static void trie_collisionInTail(Trie *trie, TrieIndex state, const TrieNeedle *needle, TrieNeedleIndex needleIndex, UserData userData);
static inline TrieIndex trie_collisionInTail_needle(Trie *trie, TrieIndex state, const TrieNeedle *needle, TrieNeedleIndex needleIndex, UserData userData);
static inline TrieIndex trie_collisionInTail_tail(Trie *trie, TrieIndex state, TrieIndex tailIndex, TailCharIndex tailIterator, UserData userData);
static inline TrieIndex trie_collisionInTail_common(Trie *trie, TrieIndex state, TailCharIndex commonTail, TailBuilderCell tailBuilderCell);
static TrieIndex trie_collisionInArray(Trie *trie, TrieIndex state, TrieBase base, TrieIndex check, Character character);
static TrieIndex trie_moveBase(Trie *trie, TrieBase oldBase, TrieBase freeBase, TrieIndex check, TrieIndex state);
static TrieIndex trie_findEmptyCell(const Trie *trie, TrieIndex node);
static TrieIndex trie_findFreeBase(const Trie *trie, TrieIndex node);
static TrieIndex trie_storeCharacter(Trie *trie, TrieIndex lastState, TrieBase newNodeBase, Character character);
static TrieIndex trie_storeNeedle(Trie *trie, TrieIndex lastState, const TrieNeedle *needle, TrieNeedleIndex needleIndex, UserData userData);


const UserData emptyUserData = {0};


static TrieIndex createState(const Character character, const TrieBase base) {
    TrieIndex state = 0;
    if (unlikely(add_overflow(character, base, &state))) {
        error("index reached maximum value");
    }
    return state;
}


TrieOptions *createTrieOptions(const bool useTail, const bool useUserData, const size_t childListInitSize) {
    TrieOptions *options = safeAlloc(sizeof(TrieOptions), "TrieOptions");

    options->useTail = useTail;
    options->useUserData = useUserData;
    options->childListInitSize = childListInitSize;

    return options;
}

void trieOptions_free(TrieOptions *options) {
    free(options);
    options = NULL;
}


Trie *createTrie(TrieOptions *options, TailBuilder *tailBuilder, UserDataList *userDataList, const size_t initialSize) {
    if (unlikely(initialSize < 4)) {
        error("minimum initial DAT size must be at least 4");
    }

    Trie *trie = safeAlloc(sizeof(Trie), "Trie");

    trie->options = options;
    trie->tailBuilder = tailBuilder;
    trie->userDataList = userDataList;
    trie->size = (TrieIndex)initialSize;
    trie->cells = safeAlloc(trie->size * sizeof(TrieCell), "Trie cells");
    trie->cells[0] = (TrieCell) {-(trie->size - 1), -2, NULL}; // TRIE_POOL_INFO
    trie->cells[1] = (TrieCell) {1, 0, createList(options->childListInitSize)}; // TRIE_POOL_START
    trie->cells[2] = (TrieCell) {0, -3, NULL};

    trie_poolInit(trie, 3, trie->size);

    trie->cells[initialSize - 1].check = 0;

    return trie;
}

size_t trie_getSize(const Trie *trie) {
    return (size_t)trie->size;
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


static void trie_poolInit(Trie *trie, const TrieIndex fromIndex, const TrieIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        trie->cells[i] = (TrieCell) {-(i - 1), -(i + 1), NULL};
    }
}

static void trie_poolReallocate(Trie *trie, const TrieIndex newSize) {
    trie->cells = safeRealloc(trie->cells, trie->size, newSize, sizeof(TrieCell), "Trie");

    trie_poolInit(trie, trie->size, newSize);

    trie->cells[-trie->cells[0].base].check = -trie->size;
    trie->cells[0].base = -(newSize - 1);
    trie->cells[newSize - 1].check = 0;

    if (trie->options->useUserData) {
        userDataList_reallocate(trie->userDataList, trie->size, newSize);
    }

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

static void trie_freeCell(Trie *trie, const TrieIndex cell) {
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
        children = createList(trie->options->childListInitSize);
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

    UserData charUserData;
    ListIndex checkListIndex = list_iterate(checkChildren, 0);
    while (likely(checkListIndex > 0)) {
        const Character character = list_getValue(checkChildren, checkListIndex);
        const TrieIndex charIndex = createState(character, oldBase);
        const TrieBase charBase = trie_getBase(trie, charIndex);
        const TrieIndex newCharIndex = createState(character, freeBase);

        if (trie->options->useUserData) {
            charUserData = userDataList_get(trie->userDataList, charIndex);
        }

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

        if (trie->options->useUserData) {
            userDataList_set(trie->userDataList, charIndex, (UserData){NULL, 0});
            userDataList_set(trie->userDataList, newCharIndex, charUserData);
        }

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
            baseChildren = createList(trie->options->childListInitSize);
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
        const TrieNeedle *needle,
        const TrieNeedleIndex needleIndex,
        const UserData userData
) {
    const TrieBase lastBase = trie_getBase(trie, lastState);
    const Character character = needle->characters[needleIndex];
    const TrieIndex newState = createState(character, lastBase);

    const TrieBase base = trie_getBase(trie, newState);
    const TrieIndex check = trie_getCheck(trie, newState);

    if (check <= 0) {
        if (trie->options->useTail) {
            trie_insertBranch(trie, newState, lastState, needle, needleIndex, userData);
            return 0;
        } else {
            trie_insertNode(trie, newState, 1, lastState);
            return newState;
        }
    } else if (check != lastState) {
        return trie_collisionInArray(trie, newState, 1, lastState, character);
    } else if (base < 0 && trie->options->useTail) {
        trie_collisionInTail(trie, newState, needle, needleIndex, userData);
        return 0;
    } else {
        return newState;
    }
}

static void trie_insertEndOfText(Trie *trie, const TrieIndex check, const UserData userData) {
    TrieIndex state = trie_storeCharacter(trie, check, trie_getBase(trie, check), END_OF_TEXT);
    if (trie->options->useUserData) {
        userDataList_set(trie->userDataList, state, userData);
    }
}

static void trie_insertBranch(
        Trie *trie,
        const TrieIndex state,
        const TrieIndex check,
        const TrieNeedle *needle,
        const TrieNeedleIndex needleIndex,
        const UserData userData
) {
    TrieBase newNodeBase = 1;
    const TailCharIndex tailCharsLength = needle->length - needleIndex - 1;

    if (likely(tailCharsLength > 0)) {
        Character *chars = allocateCharacters(tailCharsLength);

        TailCharIndex c = 0;
        TrieNeedleIndex i = needleIndex + 1;
        for (; i < needle->length; i++, c++) {
            chars[c] = needle->characters[i];
        }

        newNodeBase = -tailBuilder_insertChars(trie->tailBuilder, tailCharsLength, chars);
    }

    trie_insertNode(trie, state, newNodeBase, check);
    userDataList_set(trie->userDataList, state, userData);
    if (likely(newNodeBase > 0)) {
        trie_insertEndOfText(trie, state, userData);
    }
}


static inline TrieIndex trie_collisionInTail_common(
        Trie *trie,
        const TrieIndex state,
        const TailCharIndex commonTail,
        const TailBuilderCell tailBuilderCell
) {
    TrieIndex nextState = state;

    for (TailCharIndex i = 0; i < commonTail; i++) {
        nextState = trie_storeCharacter(trie, nextState, 1, tailBuilderCell.chars[i]);
    }

    return nextState;
}

static inline TrieIndex trie_collisionInTail_needle(
        Trie *trie,
        const TrieIndex state,
        const TrieNeedle *needle,
        const TrieNeedleIndex needleIndex,
        const UserData userData
) {
    const TrieNeedleIndex leftCharacters = needle->length - needleIndex;
    TrieIndex endState = state, newState = state;
    TrieBase newBase = 1;

    if (1 < leftCharacters) {
        Character *chars = allocateCharacters(leftCharacters - 1);

        TailCharIndex c = 0;
        TrieNeedleIndex i = needleIndex + 1;
        for (; i < needle->length; i++, c++) {
            chars[c] = needle->characters[i];
        }

        newBase = -tailBuilder_insertChars(trie->tailBuilder, leftCharacters - 1, chars);
    }

    if (1 <= leftCharacters) {
        endState = trie_storeCharacter(trie, state, newBase,needle->characters[needleIndex]);
        newState = trie_getCheck(trie, endState);
        userDataList_set(trie->userDataList, endState, userData);
    }

    if (1 >= leftCharacters) {
        trie_insertEndOfText(trie, endState, userData);
    }

    return newState;
}

static inline TrieIndex trie_collisionInTail_tail(
        Trie *trie,
        const TrieIndex state,
        const TrieIndex tailIndex,
        const TailCharIndex tailIterator,
        const UserData userData
) {
    const TailBuilderCell tailBuilderCell = trie->tailBuilder->cells[tailIndex];
    const TrieNeedleIndex leftCharacters = tailBuilderCell.length - tailIterator;
    TrieIndex endState = state, newState = state;
    TrieBase newBase = 1;

    if (1 < leftCharacters) {
        Character *chars = allocateCharacters(leftCharacters - 1);

        TailCharIndex c = 0;
        TrieNeedleIndex i = tailIterator + 1;
        for (; i < tailBuilderCell.length; i++, c++) {
            chars[c] = tailBuilderCell.chars[i];
        }

        newBase = -tailBuilder_insertChars(trie->tailBuilder, leftCharacters - 1, chars);
    }

    if (1 <= leftCharacters) {
        endState = trie_storeCharacter(trie, state, newBase, tailBuilderCell.chars[tailIterator]);
        newState = trie_getCheck(trie, endState);
        userDataList_set(trie->userDataList, endState, userData);
    }

    if (1 >= leftCharacters) {
        trie_insertEndOfText(trie, endState, userData);
    }

    return newState;
}

static void trie_collisionInTail(
        Trie *trie,
        const TrieIndex state,
        const TrieNeedle *needle,
        const TrieNeedleIndex needleIndex,
        const UserData userData
) {
    UserData stateUserData;
    if (trie->options->useUserData) {
        stateUserData = userDataList_get(trie->userDataList, state);
    }
    const TrieIndex tailIndex = -trie_getBase(trie, state);
    const TailBuilderCell tailBuilderCell = trie->tailBuilder->cells[tailIndex];


    TailCharIndex tailIterator = 0;
    TrieNeedleIndex needleIterator = needleIndex + 1;
    while (tailIterator < tailBuilderCell.length
        && needleIterator < needle->length
        && tailBuilderCell.chars[tailIterator] == needle->characters[needleIterator]
    ) {
        tailIterator++, needleIterator++;
    }

    if (tailIterator == tailBuilderCell.length && needleIterator == needle->length) {
        return;
    }


    trie_setBase(trie, state, 1);
    TrieIndex nextState = trie_collisionInTail_common(trie, state, tailIterator, tailBuilderCell);
    nextState = trie_collisionInTail_needle(trie, nextState, needle, needleIterator, userData);
    trie_collisionInTail_tail(trie, nextState, tailIndex, tailIterator, stateUserData);


    tailBuilder_freeCell(trie->tailBuilder, tailIndex);
}


void trie_addNeedle(Trie *trie, const TrieNeedle *needle) {
    TrieIndex lastState = TRIE_POOL_START;

    for (TrieNeedleIndex i = 0; i < needle->length; i++) {
        lastState = trie_storeNeedle(trie, lastState, needle, i, emptyUserData);
        if (0 == lastState) {
            return;
        }
    }

    trie_insertEndOfText(trie, lastState, emptyUserData);
}

void trie_addNeedleWithData(Trie *trie, const TrieNeedle *needle, UserData data) {
    TrieIndex lastState = TRIE_POOL_START;

    for (TrieNeedleIndex i = 0; i < needle->length; i++) {
        lastState = trie_storeNeedle(trie, lastState, needle, i, data);
        if (0 == lastState) {
            return;
        }
    }

    trie_insertEndOfText(trie, lastState, data);
}
