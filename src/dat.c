#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "unicode/utf8.h"
#include "dat.h"
#include "debug.h"

typedef unsigned char AlphabetSize;


const int ALLOCATION_STEP = 2;
const TrieIndex TRIE_POOL_INFO = 0;
const TrieIndex TRIE_POOL_START = 1;
const AlphabetSize AlphabetSizeMax = 255;

typedef struct {
    TrieChar chars[AlphabetSizeMax];
    AlphabetSize count;
} CharSet;

Tail* create_tail() {
    Tail *tail = malloc(sizeof(Tail));

    TailCell c0;
    c0.nextFree = 0;
    c0.length = 0;
    c0.chars = NULL;

    tail->cellsSize = 1;
    tail->cells = malloc(sizeof(TailCell) * 1);

    return tail;
}

void tail_poolReallocate(Tail *tail, TailIndex newSize) {
    tail->cells = realloc(tail->cells, sizeof(TailCell) * newSize);

    for (TrieIndex i = tail->cellsSize; i < newSize; i++) {
        tail->cells[i].chars = NULL;
        tail->cells[i].length = 0;
        tail->cells[i].nextFree = i+1; // next empty cell
    }

    tail->cells[tail->cellsSize - 1].nextFree = tail->cellsSize;
    tail->cells[newSize - 1].nextFree = 0;
    tail->cellsSize = newSize;
}

void tail_poolCheckCapacity(Tail *tail, TailIndex index) {
    if (tail->cellsSize <= index + 1) {
        tail_poolReallocate(tail, index + 1 + ALLOCATION_STEP);
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

TailIndex tail_insertChars(Tail *tail, const int length, TrieChar * string) {
    TailIndex index = tail->cells[0].nextFree;

    if (index == 0) {
        index = tail->cellsSize;
        tail_poolCheckCapacity(tail, index + 1);
    }

    tail->cells[0].nextFree = tail->cells[index].nextFree;

    tail->cells[index].length = length;
    tail->cells[index].chars = string;
    tail->cells[index].nextFree = 0;

    tail_print(tail);

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

Trie* create_trie() {
    Tail *tail = create_tail();
    Trie *trie = malloc(sizeof(Trie));

    Cell c0, c1, c2;

    c0.base = -2;
    c0.check = -2;

    c1.base = 1; // TRIE_POOL_START
    c1.check = 0;

    c2.base = 0;
    c2.check = 0;

    trie->tail = tail;
    trie->cellsSize = 3;
    trie->cells = malloc(sizeof(Cell) * trie->cellsSize);
    trie->cells[0] = c0;
    trie->cells[1] = c1;
    trie->cells[2] = c2;

    return trie;
}

TrieBuilder* create_TrieBuilder() {
    TrieBuilder *builder = malloc(sizeof(TrieBuilder));
    builder->trie = create_trie();
    return builder;
}

TrieBase trie_getBase(Trie *trie, TrieIndex index) {
    if (index < trie->cellsSize) {
        return trie->cells[index].base;
    }

    return 0;
}

TrieIndex trie_getCheck(Trie *trie, TrieIndex index) {
    if (index < trie->cellsSize) {
        return trie->cells[index].check;
    }

    return 0;
}

void trie_setCheck(Trie *trie, TrieIndex index, TrieIndex value) {
    trie->cells[index].check = value;
}

void trie_setBase(Trie *trie, TrieIndex index, TrieBase value) {
    trie->cells[index].base = value;
}

void trie_poolReallocate(Trie *trie, TrieIndex newSize) {
    trie->cells = realloc(trie->cells, sizeof(Cell) * newSize);

    for (TrieIndex i = trie->cellsSize; i < newSize; i++) {
        trie->cells[i].base = -(i-1); // prev empty cell
        trie->cells[i].check = -(i+1); // next empty cell
    }

    trie->cells[-trie->cells[0].base].check = -trie->cellsSize; // old last (empty) cell now points to first newly added empty cell
    trie->cells[newSize - 1].check = 0; // new last (empty) cell does not have next empty cell
    trie->cells[0].base = -(newSize - 1); // first base should point to last (empty) cell

    trie->cellsSize = newSize;
}

void trie_poolCheckCapacity(Trie *trie, TrieIndex index) {
    // +1 is for ensuring at least one empty cell on the end
    if (trie->cellsSize <= index + 1) {
        trie_poolReallocate(trie, index + 1 + ALLOCATION_STEP);
    }
}

// take care of doubly linked list
void trie_allocateCell(Trie *trie, TrieIndex cell) {
    TrieIndex base = trie_getBase(trie, cell);
    TrieIndex check = trie_getCheck(trie, cell);

    trie_setBase(trie, -check, base);
    trie_setCheck(trie, -base, check);

    trie_setBase(trie, cell, 0); // todo remove, useless work
    trie_setCheck(trie, cell, 0); // todo remove, useless work
}

void trie_freeCell(Trie *trie, TrieIndex cell) {
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

// insert new node without collision
void trie_insertNode(
        Trie *trie,
        TrieIndex state,
        TrieBase base,
        TrieIndex check
) {
    trie_poolCheckCapacity(trie, state);
    trie_allocateCell(trie, state);

    trie_setCheck(trie, state, check);
    trie_setBase(trie, state, base);
}

CharSet *charSet_create() {
    CharSet *ch = calloc(1, sizeof(CharSet));

    if (ch == NULL) {
        fprintf(stderr, "can not allocate %lu memory for charset", sizeof(CharSet));
        exit(1);
    }

    return ch;
}

void charSet_free(CharSet *charSet) {
    free(charSet);
}

void charSet_fill(CharSet *charSet, Trie *trie, TrieIndex state) {
    TrieIndex base = trie_getBase(trie, state);
    AlphabetSize index = 0;

    for (TrieIndex c = 0; c < trie->cellsSize; c++) {
        if (trie_getCheck(trie, base + c) == state) {
            charSet->chars[index] = c;
            charSet->count += 1;
            index++;
        }
    }
}

void charSet_append(CharSet *charSet, TrieChar character) {
    charSet->chars[charSet->count] = character;
    charSet->count += 1;
}

void charSet_removeLast(CharSet *charSet) {
    charSet->chars[charSet->count - 1] = 0;
    charSet->count -= 1;
}

TrieIndex trie_findEmptyCell(
        Trie *trie,
        TrieIndex node
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

TrieIndex trie_findFreeBase(Trie *trie, CharSet *charSet, TrieIndex node) {
    TrieIndex emptyCell = trie_findEmptyCell(trie, node);
    TrieIndex lastCell = -trie_getBase(trie, TRIE_POOL_INFO);
    TrieBase base = emptyCell - node;

    SEARCH:
    for (AlphabetSize c = 0; c < charSet->count; c++) {
        TrieIndex index = base + charSet->chars[c];
        if (trie_getCheck(trie, index) > 0) {
            if (lastCell == emptyCell) {
                base++;
            } else {
                emptyCell = trie_findEmptyCell(trie, emptyCell);
                base = emptyCell - node;
            }
            goto SEARCH;
        }
    }

    return base;
}

TrieIndex trie_moveBase(
        Trie *trie,
        CharSet *charSet,
        TrieBase oldBase,
        TrieBase freeBase,
        TrieIndex check,
        TrieIndex state
) {
    CharSet *newCheckCharSet = charSet_create();

    for (AlphabetSize c = 0; c < charSet->count; c++) {
        TrieIndex charIndex = oldBase + charSet->chars[c];
        TrieIndex newCharIndex = freeBase + charSet->chars[c];
        TrieBase charBase = trie_getBase(trie, charIndex);

        if (charIndex == state) {
            state = newCharIndex;
        }

        charSet_fill(newCheckCharSet, trie, charIndex);
        for (AlphabetSize c2 = 0; c2 < newCheckCharSet->count; c2++) {
            trie_setCheck(trie, charBase + newCheckCharSet->chars[c2], newCharIndex);
        }

        trie_freeCell(trie, charIndex);
        trie_insertNode(trie, newCharIndex, charBase, check);

        memset(newCheckCharSet, 0, sizeof(CharSet));
    }

    charSet_free(newCheckCharSet);

    return state;
}


TrieIndex trie_solveCollision(
        Trie *trie,
        TrieIndex lastState,
        TrieIndex state,
        TrieBase base,
        TrieChar character
) {
    CharSet *baseCharSet = charSet_create();
    CharSet *checkCharSet = charSet_create();

    charSet_fill(baseCharSet, trie, lastState);
    charSet_fill(checkCharSet, trie, trie_getCheck(trie, state));

    TrieBase freeBase;
    TrieIndex newState, parentIndex;
    TrieIndex newCheck = lastState;
    CharSet *parentCharset;

    if (baseCharSet->count + 1 < checkCharSet->count) {
        parentIndex = lastState;
        parentCharset = baseCharSet;

        charSet_append(baseCharSet, character);
        freeBase = trie_findFreeBase(trie, parentCharset, parentIndex);
        charSet_removeLast(baseCharSet);

        newState = freeBase + character;
    } else {
        parentIndex = trie_getCheck(trie, state);
        parentCharset = checkCharSet;

        freeBase = trie_findFreeBase(trie, parentCharset, parentIndex);

        newState = state;
    }

    TrieBase tempBase = trie_getBase(trie, parentIndex);
    TrieIndex newNodeCheck = trie_moveBase(trie, parentCharset, tempBase, freeBase, parentIndex, newCheck);

    trie_setBase(trie, parentIndex, freeBase);
    trie_insertNode(trie, newState, base, newNodeCheck);

    charSet_free(baseCharSet);
    charSet_free(checkCharSet);

    return newState;
}

void trie_insertBranch(
        Trie *trie,
        TrieIndex state,
        TrieBase base,
        TrieIndex check,
        int needleIndex,
        const TrieChar * needle
) {
    int needleLength = needleIndex;
    while (needle[needleLength] != '\0') {
        needleLength++;
    }

    TrieIndex newNodeBase = base;
    int tailCharsLength = needleLength - needleIndex - 1;
    if (tailCharsLength > 0) {
        TrieChar *chars = tail_allocateChars(tailCharsLength);

        int c = 0;
        int i = needleIndex + 1;
        for (; i < needleLength; i++, c++) {
            chars[c] = needle[i];
        }

        newNodeBase = -tail_insertChars(trie->tail, tailCharsLength, chars);
    }

    trie_insertNode(trie, state, newNodeBase, check);
}

TrieIndex trie_insert(
    Trie *trie,
    TrieIndex lastState,
    TrieBase base,
    TrieChar n
) {
    TrieBase lastBase = trie_getBase(trie, lastState);
    TrieChar code = u8decode(n);
    TrieIndex newState = code + lastBase;

    TrieBase newStateBase = trie_getBase(trie, newState);
    TrieIndex check = trie_getCheck(trie, newState);

    if (check <= 0) {
        trie_insertNode(trie, newState, base, lastState);
        return newState;
    } else if (check != lastState || newStateBase < 0) {
        return trie_solveCollision(trie, lastState, newState, base, code);
    } else {
        return newState;
    }
}
void trie_collisionInTail(
        Trie *trie,
        TrieIndex state,
        TrieIndex check,
        int needleIndex,
        const TrieChar * needle
) {
    TrieIndex tailIndex = -trie_getBase(trie, state);
    TailCell tailCell = trie->tail->cells[tailIndex];

    char areSame = 1;
    int needleLength = needleIndex + 1;
    int commonTailLength = 0;
    while (needle[needleLength] != '\0') {
        if (areSame == 1 && commonTailLength < tailCell.length && tailCell.chars[commonTailLength] == needle[needleLength]) {
            commonTailLength++;
        } else {
            areSame = 0;
        }
        needleLength++;
    }

    if (areSame == 1) {
        return;
    }


    TrieBase base = trie_getBase(trie, check);
    trie_setBase(trie, state, base);
    for (int i = 0; i < commonTailLength; i++) {
        state = trie_insert(trie, state, base, tailCell.chars[i]);
        base = trie_getBase(trie, state);
    }


    TailIndex needleNodeBase = base;
    if ((needleLength - needleIndex - commonTailLength - 1) > 1) {
        int tailCharsLength = needleLength - needleIndex - commonTailLength - 2;
        TrieChar *chars = tail_allocateChars(tailCharsLength);

        int c = 0;
        int i = needleIndex + commonTailLength + 2;
        for (; i < needleLength; i++, c++) {
            chars[c] = needle[i];
        }

        needleNodeBase = -tail_insertChars(trie->tail, tailCharsLength, chars);
    }
    state = trie_insert(trie, state, needleNodeBase, needle[needleIndex + commonTailLength + 1]);
    state = trie_getCheck(trie, state);


    TailIndex tailNodeBase = base;
    if (commonTailLength + 1 < tailCell.length) {
        int tailCharsLength = tailCell.length - commonTailLength - 1;
        TrieChar *chars = tail_allocateChars(tailCharsLength);

        for (int i = commonTailLength + 1, c = 0; i < tailCell.length; i++, c++) {
            chars[c] = tailCell.chars[i];
        }

        tailNodeBase = -tail_insertChars(trie->tail, tailCharsLength, chars);
    }
    trie_insert(trie, state, tailNodeBase, tailCell.chars[commonTailLength]);


    tail_freeCell(trie->tail, tailIndex);
}

void trieBuilder_addNeedle(TrieBuilder *builder, const char *needle) {
    int length = 0;
    TrieIndex lastState = TRIE_POOL_START;

    while (needle[length] != '\0') {
        TrieBase lastBase = trie_getBase(builder->trie, lastState);
        uint32_t n = needle[length];
        TrieChar code = u8decode(n);
        TrieIndex newState = code + lastBase;

        TrieBase base = trie_getBase(builder->trie, newState);
        TrieIndex check = trie_getCheck(builder->trie, newState);

        if (check <= 0) {
            trie_insertBranch(builder->trie, newState, 1, lastState, length, needle);
            break;
        } else if (check != lastState) {
            lastState = trie_solveCollision(builder->trie, lastState, newState, 1, code);
        } else if (base < 0) {
            trie_collisionInTail(builder->trie, newState, lastState, length, needle);
            break;
        } else {
            lastState = newState;
        }

        length++;
    }
}

void trie_find(Trie *trie, const char *needle) {
    int length = 0;
    TrieIndex state = TRIE_POOL_START;

    while (needle[length] != '\0') {
        TrieBase base = trie_getBase(trie, state);
        uint32_t n = needle[length];
        TrieChar code = u8decode(n);
        TrieIndex newState = code + base;
        TrieIndex check = trie_getCheck(trie, newState);

        printf("WORD %c\n", n);

        if (check == state) {
            state = newState;
        } else if (base < 0) {
            TailCell tailCell = trie->tail->cells[-base];
            for (int i = 0; i < tailCell.length; i++) {
                if (tailCell.chars[i] != needle[length]) {
                    printf("can not search word (tail)\n");
                    exit(1);
                }
                length++;
            }
            // todo check over length
            return;
        } else {
            printf("can not search word\n");
            exit(1);
        }

        length++;
    }
}

TrieIndex trie_findLastFilled(Trie *trie) {
    TrieIndex state = -trie_getBase(trie, 0);
    while (trie_getBase(trie, state) < 0) {
        state--;
    }

    return state;
}
