#include <stdlib.h>
#include <stdio.h>
#include "typedefs.h"
#include "ac.h"
#include "list.h"
#include "dat.h"
#include "tail.h"

static Automaton *createAutomaton(AutomatonIndex initialSize);
static Automaton *createAutomatonFromTrie(const Trie *trie, List *list);
static Automaton *buildAutomaton(const Trie *trie, List *list, TrieIndex (*obtainNode)(List *list));
static AutomatonIndex automaton_step(const Automaton *automaton, AutomatonIndex state, AutomatonTransition transition);
static void automaton_copyCell(Automaton *automaton, const Trie *trie, TrieIndex trieIndex);
static void automaton_setBase(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static void automaton_setCheck(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static void automaton_setFail(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static void automaton_setShortcut(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static AutomatonIndex automaton_getBase(const Automaton *automaton, AutomatonIndex index);
static AutomatonIndex automaton_getCheck(const Automaton *automaton, AutomatonIndex index);
static AutomatonIndex automaton_getFail(const Automaton *automaton, AutomatonIndex index);
static AutomatonIndex automaton_getShortcut(const Automaton *automaton, AutomatonIndex index);
static unsigned char isTail(const Tail *tail, const Needle *needle, AlphabetSize needleIndex, TailIndex tailIndex);


static void automaton_setBase(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].base = value;
}

static void automaton_setCheck(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].check = value;
}

static void automaton_setFail(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].fail = value;
}

static void automaton_setShortcut(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].shortcut = value;
}


static AutomatonIndex automaton_getBase(const Automaton *automaton, const AutomatonIndex index) {
    return automaton->cells[index].base;
}

static AutomatonIndex automaton_getCheck(const Automaton *automaton, const AutomatonIndex index) {
    return index < automaton->size ? automaton->cells[index].check : 0;
}

static AutomatonIndex automaton_getFail(const Automaton *automaton, const AutomatonIndex index) {
    return automaton->cells[index].fail ?: 1;
}

static AutomatonIndex automaton_getShortcut(const Automaton *automaton, const AutomatonIndex index) {
    return automaton->cells[index].shortcut;
}


static void automaton_copyCell(Automaton *automaton, const Trie *trie, const TrieIndex trieIndex) {
    automaton_setBase(automaton, trieIndex, trie_getBase(trie, trieIndex));
    automaton_setCheck(automaton, trieIndex, trie_getCheck(trie, trieIndex));
}

static AutomatonIndex automaton_step(const Automaton *automaton, AutomatonIndex state, const AutomatonTransition transition) {
    AutomatonIndex nextState;

    nextState = automaton_getBase(automaton, state) + transition;
    while (automaton_getCheck(automaton, nextState) != state && state != TRIE_POOL_START) {
        state = automaton_getFail(automaton, state);
        nextState = automaton_getBase(automaton, state) + transition;
    }

    nextState = automaton_getBase(automaton, state) + transition;
    if (automaton_getCheck(automaton, nextState) == state) {
        state = nextState;
    }

    return state;
}

static Automaton *createAutomaton(AutomatonIndex initialSize) {
    Automaton *automaton = calloc(1, sizeof(Automaton));
    if (automaton == NULL) {
        fprintf(stderr, "can not allocate %lu memory for automaton", sizeof(Automaton));
        exit(1);
    }

    automaton->size = initialSize;
    automaton->cells = calloc(initialSize, sizeof(AutomatonCell));
    if (automaton->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for automaton cells", sizeof(Automaton) * initialSize);
        exit(1);
    }

    return automaton;
}

static Automaton *createAutomatonFromTrie(const Trie *trie, List *list) {
    TrieIndex lastFilled = -trie_getBase(trie, 0);
    while (trie_getCheck(trie, lastFilled) < 0) {
        lastFilled--;
    }

    Automaton *automaton = createAutomaton(lastFilled);

    automaton_copyCell(automaton, trie, TRIE_POOL_START);

    for (TrieIndex state = TRIE_POOL_START + 1; state <= lastFilled; state++) {
        TrieIndex check = trie_getCheck(trie, state);
        if (check > 0) {
            automaton_copyCell(automaton, trie, state);

            if (check == TRIE_POOL_START) {
                automaton_setFail(automaton, state, TRIE_POOL_START);
                list_push(list, state);
            }
        }
    }

    return automaton;
}

static Automaton *buildAutomaton(const Trie *trie, List *list, TrieIndex (*obtainNode)(List *list)) {
    Automaton *automaton = createAutomatonFromTrie(trie, list);

    while (!list_isEmpty(list)) {
        const AutomatonIndex check = obtainNode(list);
        List *checkChildren = trie_getChildren(trie, check);

        ListIndex listIndex = list_iterate(checkChildren, 0);
        while (listIndex) {
            AutomatonTransition transition = list_getValue(checkChildren, listIndex);
            listIndex = list_iterate(checkChildren, listIndex);

            if (transition == END_OF_TEXT) {
                continue;
            }

            const AutomatonIndex state = automaton_getBase(automaton, check) + transition;
            const AutomatonIndex next = automaton_step(automaton, automaton_getFail(automaton, check), transition);

            automaton_setFail(automaton, state, next);

            if (automaton_getCheck(automaton, automaton_getBase(automaton, next) + END_OF_TEXT) == next) {
                automaton_setShortcut(automaton, state, next);
            } else {
                automaton_setShortcut(automaton, state, automaton_getShortcut(automaton, next));
            }

            list_push(list, state);
        }
    }

    return automaton;
}

Automaton *createAutomaton_DFS(const Trie *trie, List *list) {
    return buildAutomaton(trie, list, list_pop);
}

Automaton *createAutomaton_BFS(const Trie *trie, List *list) {
    return buildAutomaton(trie, list, list_shift);
}

static unsigned char isTail(const Tail *tail, const Needle *needle, const AlphabetSize needleIndex, const TailIndex tailIndex) {
    TailCell tailCell = tail_getCell(tail, tailIndex);

    AlphabetSize t = 0;
    AlphabetSize c = needleIndex + 1;
    while (c < needle->length && t < tailCell.length && needle->characters[c] == tailCell.chars[t]) {
        c++, t++;
    }

    return c == needle->length && t == tailCell.length;
}

unsigned char automaton_search(const Automaton *automaton, const Tail *tail, const Needle *needle) {
    TrieIndex state = TRIE_POOL_START;

    for (AlphabetSize i = 0; i < needle->length; i++) {
        TrieChar character = needle->characters[i];
        TrieIndex back = state = automaton_step(automaton, state, character);

        while (back) {
            TrieBase base = automaton_getBase(automaton, state);

            if ((base < 0 && isTail(tail, needle, i, -base)) ||
                automaton_getCheck(automaton, base + END_OF_TEXT) == state) {
                return 1;
            }

            back = automaton_getShortcut(automaton, back);
        }
    }

    return 0;
}