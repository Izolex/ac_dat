#include <stdlib.h>
#include <stdio.h>
#include "definitions.h"
#include "ac.h"
#include "list.h"
#include "dat.h"
#include "tail.h"
#include "memory.h"


static Automaton *createAutomatonFromTrie(const Trie *trie, List *list);
static AutomatonIndex createState(AutomatonTransition transition, AutomatonIndex base);
static Automaton *buildAutomaton(const Trie *trie, List *list, TrieIndex (*obtainNode)(List *list));
static AutomatonIndex automaton_step(const Automaton *automaton, AutomatonIndex state, AutomatonTransition transition);
static void automaton_copyCell(Automaton *automaton, const Trie *trie, TrieIndex trieIndex);
static void automaton_setBase(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static void automaton_setCheck(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static void automaton_setFail(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static void automaton_setOutput(Automaton *automaton, AutomatonIndex index, AutomatonIndex value);
static AutomatonIndex automaton_getBase(const Automaton *automaton, AutomatonIndex index);
static AutomatonIndex automaton_getCheck(const Automaton *automaton, AutomatonIndex index);
static AutomatonIndex automaton_getFail(const Automaton *automaton, AutomatonIndex index);
static AutomatonIndex automaton_getOutput(const Automaton *automaton, AutomatonIndex index);
static bool isTail(const Tail *tail, const Needle *needle, NeedleIndex needleIndex, TailIndex tailIndex);


static void automaton_setBase(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].base = value;
}

static void automaton_setCheck(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].check = value;
}

static void automaton_setFail(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].fail = value;
}

static void automaton_setOutput(Automaton *automaton, const AutomatonIndex index, const AutomatonIndex value) {
    automaton->cells[index].output = value;
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

static AutomatonIndex automaton_getOutput(const Automaton *automaton, const AutomatonIndex index) {
    return automaton->cells[index].output;
}


static AutomatonIndex createState(const AutomatonTransition transition, const AutomatonIndex base) {
    AutomatonIndex state = 0;
    if (unlikely(add_overflow(transition, base, &state))) {
        error("index reached maximum value");
    }
    return state;
}


static void automaton_copyCell(Automaton *automaton, const Trie *trie, const TrieIndex trieIndex) {
    automaton_setBase(automaton, (AutomatonIndex)trieIndex, (AutomatonIndex)trie_getBase(trie, trieIndex));
    automaton_setCheck(automaton, (AutomatonIndex)trieIndex, (AutomatonIndex)trie_getCheck(trie, trieIndex));
}

static AutomatonIndex automaton_step(const Automaton *automaton, AutomatonIndex state, const AutomatonTransition transition) {
    AutomatonIndex nextState, base;

    base = automaton_getBase(automaton, state);
    if (base > 0) {
        nextState = createState(transition, base);
        while (nextState > 0 && automaton_getCheck(automaton, nextState) != state && state != TRIE_POOL_START) {
            state = automaton_getFail(automaton, state);
            nextState = createState(transition, automaton_getBase(automaton, state));
        }
    }

    base = automaton_getBase(automaton, state);
    if (base < 0) {
        return state;
    }

    nextState = base + transition;
    if (automaton_getCheck(automaton, nextState) == state) {
        state = nextState;
    }

    return state;
}

void automaton_free(Automaton *automaton) {
    free(automaton->cells);
    free(automaton);
    automaton = NULL;
}

Automaton *createAutomaton(const AutomatonIndex initialSize) {
    Automaton *automaton = safeCalloc(1, sizeof(Automaton), "AC automaton");

    automaton->size = initialSize;
    automaton->cells = safeCalloc(initialSize, sizeof(AutomatonCell), "AC automaton cells");

    return automaton;
}

static Automaton *createAutomatonFromTrie(const Trie *trie, List *list) {
    TrieIndex lastFilled = -trie_getBase(trie, 0);
    while (likely(trie_getCheck(trie, lastFilled) <= 0)) {
        lastFilled--;
    }

    Automaton *automaton = createAutomaton(lastFilled + 1);

    automaton_copyCell(automaton, trie, TRIE_POOL_START);

    for (TrieIndex state = TRIE_POOL_START + 1; state <= lastFilled; state++) {
        const TrieIndex check = trie_getCheck(trie, state);
        if (likely(check > 0)) {
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

    while (likely(!list_isEmpty(list))) {
        const AutomatonIndex check = obtainNode(list);
        const List *checkChildren = trie_getChildren(trie, check);
        if (checkChildren == NULL) {
            continue;
        }

        ListIndex listIndex = list_iterate(checkChildren, 0);
        while (listIndex) {
            const AutomatonTransition transition = list_getValue(checkChildren, listIndex);
            listIndex = list_iterate(checkChildren, listIndex);

            if (transition == END_OF_TEXT) {
                continue;
            }

            const AutomatonIndex state = createState(transition, automaton_getBase(automaton, check));
            const AutomatonIndex next = automaton_step(automaton, automaton_getFail(automaton, check), transition);
            const AutomatonIndex nextBase = automaton_getBase(automaton, next);

            automaton_setFail(automaton, state, next);

            if (nextBase > 0 && automaton_getCheck(automaton, nextBase + END_OF_TEXT) == next) {
                automaton_setOutput(automaton, state, next);
            } else {
                automaton_setOutput(automaton, state, automaton_getOutput(automaton, next));
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

static bool isTail(const Tail *tail, const Needle *needle, const NeedleIndex needleIndex, const TailIndex tailIndex) {
    const TailCell tailCell = tail_getCell(tail, tailIndex);

    TailCharIndex t = 0;
    NeedleIndex c = needleIndex + 1;
    while (c < needle->length && t < tailCell.length && needle->characters[c] == tailCell.chars[t]) {
        c++, t++;
    }

    return c == needle->length && t == tailCell.length;
}

bool automaton_search(const Automaton *automaton, const Tail *tail, const Needle *needle) {
    AutomatonIndex state = TRIE_POOL_START;

    for (NeedleIndex i = 0; i < needle->length; i++) {
        const AutomatonTransition transition = (AutomatonTransition)needle->characters[i];
        AutomatonIndex nextState = state = automaton_step(automaton, state, transition);

        while (nextState) {
            const AutomatonIndex base = automaton_getBase(automaton, state);

            if ((base < 0 && isTail(tail, needle, i, -base)) ||
                automaton_getCheck(automaton, base + END_OF_TEXT) == state) {
                return true;
            }

            nextState = automaton_getOutput(automaton, nextState);
        }
    }

    return false;
}

size_t automaton_getSize(const Automaton *automaton) {
    return (size_t)automaton->size;
}
