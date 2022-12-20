#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "../include/ac.h"
#include "../include/dat.h"
#include "../include/needle.h"
#include "../include/print.h"
#include "../include/tail.h"
#include "../include/list.h"
#include "../include/file.h"
#include "../include/thread.h"
#include "../include/user_data.h"


const char *needles[] = {
        "add yours :)\0",
};
const int needlesLength = sizeof(needles) / sizeof(needles[0]);


static struct trieNeedle *safeCreateNeedle(const char *string) {
    struct trieNeedle *needle = createTrieNeedle(string);
    if (needle == NULL) {
        fprintf(stderr, "needle is not valid UTF8");
        exit(EXIT_FAILURE);
    }
    return needle;
}

static struct automaton *createAutomaton(void) {
    struct trieOptions *options = createTrieOptions(false, false, 4);
    struct trie *trie = createTrie(options, NULL, NULL, 4);

    struct trieNeedle *trieNeedle;
    for (int i = 0; i < needlesLength; i++) {
        trieNeedle = safeCreateNeedle(needles[i]);
        trie_addNeedle(trie, trieNeedle);
        trieNeedle_free(trieNeedle);
    }

    struct list *list = createList(10);
    struct automaton *automaton = createAutomaton_BFS(trie, list);

    trieOptions_free(options);
    trie_free(trie);
    list_free(list);

    return automaton;
}


static clock_t search(const int iterations) {
    struct automaton *automaton = createAutomaton();

    clock_t start = clock();

    for (int c = 0; c < iterations; c++) {
        for (size_t i = 0; i < needlesLength; i++) {
            struct occurrence *occurrence = automaton_search(automaton, NULL, NULL, needles[i], SEARCH_MODE_FIRST);
            if (!occurrence) {
                fprintf(stderr, "can not find word %s", needles[i]);
                exit(EXIT_FAILURE);
            }
            occurrence_free(occurrence);
        }
    }

    clock_t time = clock() - start;

    automaton_free(automaton);

    return time;
}

static clock_t build(const int iterations) {
    clock_t start = clock();

    for (int c = 0; c < iterations; c++) {
        automaton_free(createAutomaton());
    }

    return clock() - start;
}


int main(void) {
    printf("Build taken by CPU: %f\n", (double)build(10) / CLOCKS_PER_SEC);
    printf("Search taken by CPU: %f\n", (double)search(100000) / CLOCKS_PER_SEC);
}
