#include "typedefs.h"
#include "ac.h"
#include "list.h"
#include "dat.h"
#include "tail.h"

TrieIndex AC_step(Trie *trie, TrieIndex state, const TrieChar character) {
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

Trie *createAutomaton(Trie *trie, TrieIndex (*obtainNode)(List *list)) {
    List *list = create_List();

    TrieBase rootBase = trie_getBase(trie, TRIE_POOL_START);

    for (TrieChar i = 1; i <= MAX_ALPHABET_SIZE; i++) {
        TrieIndex state = rootBase + i;
        if (trie_getCheck(trie, state) == TRIE_POOL_START) {
            trie_setFail(trie, state, TRIE_POOL_START);
            list_push(list, state);
        }
    }

    while (!list_isEmpty(list)) {
        TrieIndex check = obtainNode(list);
        TrieBase checkBase = trie_getBase(trie, check);
        TrieIndex checkFail = trie_getFail(trie, check);

        for (TrieChar character = 1; character <= MAX_ALPHABET_SIZE; character++) {
            TrieIndex state = checkBase + character;
            if (trie_getCheck(trie, state) == check) {
                TrieIndex next = AC_step(trie, checkFail, character);

                trie_setFail(trie, state, next);

                if (trie_getBase(trie, trie_getCheck(trie, next)) + TRIE_END_OF_TEXT == next) {
                    trie_setShortcut(trie, state, next);
                } else {
                    trie_setShortcut(trie, state, trie_getShortcut(trie, next));
                }

                list_push(list, state);
            }
        }
    }

    list_free(list);
    return trie;
}

Trie *createAutomaton_DFS(Trie *trie) {
    return createAutomaton(trie, list_pop);
}

Trie *createAutomaton_BFS(Trie *trie) {
    return createAutomaton(trie, list_shift);
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