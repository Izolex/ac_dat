#include "typedefs.h"
#include "ac.h"
#include "list.h"
#include "dat.h"
#include "tail.h"

static void AC_fillRoot(Trie *trie, List *list);
static TrieIndex AC_step(Trie *trie, TrieIndex state, TrieChar character);
static Trie *createAutomaton(Trie *trie, TrieIndex (*obtainNode)(List *list));
static unsigned char isTail(const Trie *trie, const Needle *needle, AlphabetSize needleIndex, TailIndex tailIndex);


static void AC_fillRoot(Trie *trie, List *list) {
    const TrieBase base = trie_getBase(trie, TRIE_POOL_START);
    const List *children = trie_getChildren(trie, TRIE_POOL_START);

    ListIndex listIndex = list_iterate(children, 0);
    while (listIndex > 0) {
        const TrieChar character = list_getValue(children, listIndex);
        const ListIndex state = base + character;
        listIndex = list_iterate(children, listIndex);

        trie_setFail(trie, state, TRIE_POOL_START);
        list_push(list, state);
    }
}

static TrieIndex AC_step(Trie *trie, TrieIndex state, const TrieChar character) {
    TrieIndex trieIndex;

    trieIndex = trie_getBase(trie, state) + character;
    while (trie_getCheck(trie, trieIndex) != state && state != TRIE_POOL_START) {
        state = trie_getFail(trie, state);
        trieIndex = trie_getBase(trie, state) + character;
    }

    trieIndex = trie_getBase(trie, state) + character;
    if (trie_getCheck(trie, trieIndex) == state) {
        state = trieIndex;
    }

    return state;
}

static Trie *createAutomaton(Trie *trie, TrieIndex (*obtainNode)(List *list)) {
    List *list = create_List(MAX_ALPHABET_SIZE);

    AC_fillRoot(trie, list);

    while (!list_isEmpty(list)) {
        const TrieIndex check = obtainNode(list);
        const TrieIndex checkBase = trie_getBase(trie, check);
        const TrieIndex checkFail = trie_getFail(trie, check);
        List *checkChildren = trie_getChildren(trie, check);

        ListIndex listIndex = list_iterate(checkChildren, 0);
        while (listIndex) {
            TrieChar character = list_getValue(checkChildren, listIndex);
            listIndex = list_iterate(checkChildren, listIndex);

            if (character == TRIE_END_OF_TEXT) {
                continue;
            }

            const ListIndex state = checkBase + character;
            const TrieIndex next = AC_step(trie, checkFail, character);

            trie_setFail(trie, state, next);

            if (trie_getCheck(trie, trie_getBase(trie, next) + TRIE_END_OF_TEXT) == next) {
                trie_setShortcut(trie, state, next);
            } else {
                trie_setShortcut(trie, state, trie_getShortcut(trie, next));
            }

            list_push(list, state);
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

static unsigned char isTail(const Trie *trie, const Needle *needle, const AlphabetSize needleIndex, const TailIndex tailIndex) {
    TailCell tailCell = tail_getCell(trie->tail, tailIndex);

    AlphabetSize t = 0;
    AlphabetSize c = needleIndex + 1;
    while (c < needle->length && t < tailCell.length && needle->characters[c] == tailCell.chars[t]) {
        c++, t++;
    }

    return c == needle->length && t == tailCell.length;
}

unsigned char search(Trie *trie, const Needle *needle) {
    TrieIndex state = TRIE_POOL_START;
    TrieIndex back;
    for (AlphabetSize i = 0; i < needle->length; i++) {
        TrieChar character = needle->characters[i];
        state = AC_step(trie, state, character);
        back = state;

        while (back) {
            TrieBase base = trie_getBase(trie, state);

            if ((base < 0 && isTail(trie, needle, i, -base)) ||
                trie_getCheck(trie, base + TRIE_END_OF_TEXT) == state) {
                return 1;
            }

            back = trie_getShortcut(trie, back);
        }
    }

    return 0;
}