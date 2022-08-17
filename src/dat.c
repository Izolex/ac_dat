#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "dat.h"
#include "character.h"
#include "tail.h"

void trie_print(Trie *trie) {
    printf("\n");
    for (int i = 0; i < trie->cellsSize; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (int i = 0; i < trie->cellsSize; i++) {
        printf("%4ld | ", trie->cells[i].base);
    }
    printf("\n");
    for (int i = 0; i < trie->cellsSize; i++) {
        printf("%4ld | ", trie->cells[i].check);
    }
    printf("\n");
    for (int i = 0; i < trie->cellsSize; i++) {
        printf("%4ld | ", trie->cells[i].fail);
    }
    printf("\n");
    for (int i = 0; i < trie->cellsSize; i++) {
        printf("%4ld | ", trie->cells[i].shortcut);
    }
    printf("\n\n");
    tail_print(trie->tail);
}

void trie_connectLinkedList(Trie *trie, TrieIndex fromIndex, TrieIndex toIndex) {
    for (TrieIndex i = fromIndex; i < toIndex; i++) {
        trie->cells[i].base = -(i - 1);
        trie->cells[i].check = -(i + 1);
    }
}

Trie *create_trie(long int datSize, long int tailSize) {
    Tail *tail = create_tail(tailSize);
    Trie *trie = malloc(sizeof(Trie));

    if (datSize < 4) {
        fprintf(stderr, "minimum initial DAT size must be at least 4");
        exit(1);
    }

    trie->tail = tail;
    trie->cellsSize = datSize;
    trie->cells = malloc(sizeof(TrieCell) * trie->cellsSize);
    trie->cells[0] = (TrieCell) {-2, -2}; // TRIE_POOL_INFO
    trie->cells[1] = (TrieCell) {1, 0}; // TRIE_POOL_START
    trie->cells[2] = (TrieCell) {0, -3};

    trie_connectLinkedList(trie, 3, trie->cellsSize);

    trie->cells[datSize - 1].check = 0;
    trie->cells[0].base = -(datSize - 1);

    return trie;
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

TrieIndex trie_getFail(Trie *trie, TrieIndex index) {
    return trie->cells[index].fail ?: TRIE_POOL_START;
}

TrieIndex trie_getShortcut(Trie *trie, TrieIndex index) {
    return trie->cells[index].shortcut;
}

void trie_setCheck(Trie *trie, TrieIndex index, TrieIndex value) {
    trie->cells[index].check = value;
}

void trie_setBase(Trie *trie, TrieIndex index, TrieBase value) {
    trie->cells[index].base = value;
}

void trie_setFail(Trie *trie, TrieIndex index, TrieBase value) {
    trie->cells[index].fail = value;
}

void trie_setShortcut(Trie *trie, TrieIndex index, TrieBase value) {
    trie->cells[index].shortcut = value;
}

void trie_poolReallocate(Trie *trie, TrieIndex newSize) {
    trie->cells = realloc(trie->cells, sizeof(TrieCell) * newSize);

    trie_connectLinkedList(trie, trie->cellsSize, newSize);

    trie->cells[-trie->cells[0].base].check = -trie->cellsSize;
    trie->cells[0].base = -(newSize - 1);
    trie->cells[newSize - 1].check = 0;

    trie->cellsSize = newSize;
}

void trie_poolCheckCapacity(Trie *trie, TrieIndex index) {
    if (trie->cellsSize <= index + 1) {
        trie_poolReallocate(trie, index + (long int)ceill((long double)trie->cellsSize / 2));
    }
}

void trie_allocateCell(Trie *trie, TrieIndex cell) {
    TrieIndex base = trie_getBase(trie, cell);
    TrieIndex check = trie_getCheck(trie, cell);

    trie_setBase(trie, -check, base);
    trie_setCheck(trie, -base, check);
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

void trie_fillCharSet(Trie *trie, TrieIndex state, CharSet *charSet) {
    TrieIndex base = trie_getBase(trie, state);
    AlphabetSize index = 0;

    for (TrieIndex c = 0; c < MAX_ALPHABET_SIZE; c++) {
        if (trie_getCheck(trie, base + c) == state) {
            charSet->chars[index] = c;
            charSet->count += 1;
            index++;
        }
    }
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

        trie_fillCharSet(trie, charIndex, newCheckCharSet);
        for (AlphabetSize c2 = 0; c2 < newCheckCharSet->count; c2++) {
            trie_setCheck(trie, charBase + newCheckCharSet->chars[c2], newCharIndex);
        }

        trie_freeCell(trie, charIndex);
        trie_insertNode(trie, newCharIndex, charBase, check);

        charSet_reset(newCheckCharSet);
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

    trie_fillCharSet(trie, lastState, baseCharSet);
    trie_fillCharSet(trie, trie_getCheck(trie, state), checkCharSet);

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

TrieIndex trie_insert(
        Trie *trie,
        TrieIndex lastState,
        TrieBase base,
        TrieChar character
) {
    TrieBase lastBase = trie_getBase(trie, lastState);
    TrieIndex newState = character + lastBase;

    TrieBase newStateBase = trie_getBase(trie, newState);
    TrieIndex check = trie_getCheck(trie, newState);

    if (check <= 0) {
        trie_insertNode(trie, newState, base, lastState);
        return newState;
    } else if (check != lastState || newStateBase < 0) {
        return trie_solveCollision(trie, lastState, newState, base, character);
    } else {
        return newState;
    }
}

void trie_insertEndOfText(
        Trie *trie,
        TrieIndex check
) {
    TrieBase base = trie_getBase(trie, check);
    trie_insert(trie, check, base, TRIE_END_OF_TEXT);
}

void trie_insertBranch(
        Trie *trie,
        TrieIndex state,
        TrieBase base,
        TrieIndex check,
        TrieNeedle *needle,
        long int needleIndex
) {
    TrieIndex newNodeBase = base;
    long int tailCharsLength = needle->length - needleIndex - 1;
    if (tailCharsLength > 0) {
        TrieChar *chars = allocateTrieChars(tailCharsLength);

        long int c = 0;
        long int i = needleIndex + 1;
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

void trie_collisionInTail(
        Trie *trie,
        TrieIndex state,
        TrieIndex check,
        TrieNeedle *needle,
        long int needleIndex
) {
    TrieIndex tailIndex = -trie_getBase(trie, state);
    TailCell tailCell = trie->tail->cells[tailIndex];

    char areSame = 1;
    long int commonTailLength = 0;
    for (long int i = needleIndex + 1; i < needle->length; i++) {
        if (commonTailLength < tailCell.length && tailCell.chars[commonTailLength] == needle->characters[i]) {
            commonTailLength++;
        } else {
            areSame = 0;
            break;
        }
    }

    if (areSame == 1) {
        return;
    }


    TrieBase base = trie_getBase(trie, check);
    trie_setBase(trie, state, base);
    for (long int i = 0; i < commonTailLength; i++) {
        state = trie_insert(trie, state, base, tailCell.chars[i]);
        base = trie_getBase(trie, state);
    }


    TailIndex needleNodeBase = base;
    if ((needle->length - needleIndex - commonTailLength - 1) > 1) {
        long int tailCharsLength = needle->length - needleIndex - commonTailLength - 2;
        TrieChar *chars = allocateTrieChars(tailCharsLength);

        long int c = 0;
        long int i = needleIndex + commonTailLength + 2;
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


    TailIndex tailNodeBase = base;
    if (commonTailLength + 1 < tailCell.length) {
        long int tailCharsLength = tailCell.length - commonTailLength - 1;
        TrieChar *chars = allocateTrieChars(tailCharsLength);

        for (long int i = commonTailLength + 1, c = 0; i < tailCell.length; i++, c++) {
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

void trie_addNeedle(Trie *trie, TrieNeedle *needle) {
    TrieIndex lastState = TRIE_POOL_START;

    for (long int i = 0; i < needle->length; i++) {
        TrieBase lastBase = trie_getBase(trie, lastState);
        TrieChar character = needle->characters[i];
        TrieIndex newState = character + lastBase;

        TrieBase base = trie_getBase(trie, newState);
        TrieIndex check = trie_getCheck(trie, newState);

        if (check <= 0) {
            trie_insertBranch(trie, newState, 1, lastState, needle, i);
            return;
        } else if (check != lastState) {
            lastState = trie_solveCollision(trie, lastState, newState, 1, character);
        } else if (base < 0) {
            trie_collisionInTail(trie, newState, lastState, needle, i);
            return;
        } else {
            lastState = newState;
        }
    }

    trie_insertEndOfText(trie, lastState);
}

void trie_find(Trie *trie, TrieNeedle *needle) {
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
    TrieIndex newState = base + TRIE_END_OF_TEXT;
    TrieIndex check = trie_getCheck(trie, newState);

    if (check != state) {
        printf("can not search word (EOT)\n");
        exit(1);
    }
}

TrieIndex trie_findLastFilled(Trie *trie) {
    TrieIndex state = -trie_getBase(trie, 0);
    while (trie_getBase(trie, state) < 0) {
        state--;
    }

    return state;
}
