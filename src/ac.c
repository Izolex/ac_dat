#include "typedefs.h"
#include "ac.h"
#include "ac_list.h"
#include "dat.h"
#include "tail.h"

TrieIndex step(Trie *trie, TrieIndex state, const TrieChar character) {
    TrieIndex trieIndex;

    trieIndex = trie_getBase(trie, state) + character;
    while (trie_getCheck(trie, trieIndex) <= 0  && state > TRIE_POOL_START) {
        state = trie_getFail(trie, trieIndex);
        trieIndex = trie_getBase(trie, state) + character;
    }


    trieIndex = trie_getBase(trie, state) + character;
    if (trie_getCheck(trie, trieIndex) > 0) {
        state = trieIndex;
    }

    return state;
}

Trie *createAutomaton(Trie *trie) {
    List *queue = create_List();

    TrieBase poolStartBase = trie_getBase(trie, TRIE_POOL_START);

    for (TrieChar i = 1; i <= MAX_ALPHABET_SIZE; i++) {
        TrieIndex trieIndex = poolStartBase + i;
        if (trie_getCheck(trie, trieIndex) == TRIE_POOL_START) {
            trie_setFail(trie, trieIndex, TRIE_POOL_START);
            list_enqueue(queue, trieIndex);
        }
    }

    while (!list_queueIsEmpty(queue)) {
        TrieIndex check = list_dequeue(queue);
        TrieIndex checkBase = trie_getBase(trie, check);
        TrieIndex checkFail = trie_getFail(trie, check);

        for (TrieChar i = 1; i <= MAX_ALPHABET_SIZE; i++) {
            TrieIndex state = checkBase + i;
            if (trie_getCheck(trie, state) == check) {

                TrieIndex next = step(trie, checkFail, i);

                trie_setFail(trie, state, next);

                if (trie_getBase(trie, trie_getCheck(trie, next)) + TRIE_END_OF_TEXT == next) {
                    trie_setShortcut(trie, state, next);
                } else {
                    TrieIndex ss = trie_getShortcut(trie, next);
                    trie_setShortcut(trie, state, ss);
                }

                list_enqueue(queue, state);
            }
        }
    }

    list_free(queue);

    return trie;
}

unsigned char isTail(Trie *trie, TrieNeedle *needle, AlphabetSize needleIndex, TailIndex tailIndex) {
    TailCell tailCell = tail_getCell(trie->tail, tailIndex);

    AlphabetSize t = 0;
    AlphabetSize c = needleIndex + 1;
    while (c < needle->length && t < tailCell.length && needle->characters[c] == tailCell.chars[t]) {
        c++, t++;
    }

    return c == needle->length && t == tailCell.length;
}

unsigned char search(Trie *trie, TrieNeedle *needle) {
    TrieIndex state = TRIE_POOL_START;
    TrieIndex back;
    for (AlphabetSize i = 0; i < needle->length; i++) {
        TrieChar character = needle->characters[i];
        state = AC_step(trie, state, character);
        back = state;

        while (back) {
            TrieBase base = trie_getBase(trie, state);

            if (base < 0 && isTail(trie, needle, i, -base) ||
                trie_getCheck(trie, base + TRIE_END_OF_TEXT) == state) {
                return 1;
            }

            back = trie_getShortcut(trie, back);
        }
    }

    return 0;
}