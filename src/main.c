#include <stdlib.h>
#include <stdio.h>
#include "ac.h"
#include "dat.h"
#include "character.h"
#include "print.h"
#include "tail.h"
#include "list.h"
#include "typedefs.h"

Needle *safeCreateNeedle(const char *string) {
    Needle *needle = createNeedle(string);
    if (needle == NULL) {
        fprintf(stderr, "needle is not valid UTF8");
        exit(EXIT_FAILURE);
    }
    return needle;
}
int main() {
    const char *utf8Needles[] = {
// it is really long when printing trie with these characters
//            "\xc2\xa5\0", // 2B
//            "\xe2\xbf\x86\0", // 3B
//            "\xf0\x9f\xa6\x84\0", // 4B
//            "\xf0\x9f\xa7\x99\xf0\x9f\xa7\x99\0", // two same 4B emojis
            "bachelor\0",
            "jar\0",
            "badge\0",
            "baby\0",
            "ara\0",
            "bar\0",
            "arab\0",
            "baraba\0",
            "barbara\0",
    };

    const size_t needlesLength = sizeof(utf8Needles) / sizeof(utf8Needles[0]);
    Needle *needles[needlesLength];
    for (size_t i = 0; i < needlesLength; i++) {
        needles[i] = safeCreateNeedle(utf8Needles[i]);
    }

    TrieOptions *options = createTrieOptions(true);
    TailBuilder *tailBuilder = createTailBuilder(10);
    Trie *trie = createTrie(options, tailBuilder, 10);

    for (size_t i = 0; i < needlesLength; i++) {
        printf("%s (%lu)\n", utf8Needles[i], needles[i]->length);
        trie_addNeedle(trie, needles[i]);
    }

    trie_print(trie);
    tailBuilder_print(tailBuilder);

    Tail *tail = createTailFromBuilder(tailBuilder);
    tailBuilder_free(tailBuilder);

    List *list = createList(10);
    Automaton *automaton = createAutomaton_BFS(trie, list);

    automaton_print(automaton);
    tail_print(tail);

    for (size_t i = 0; i < needlesLength; i++) {
        if (!automaton_search(automaton, tail, needles[i])) {
            fprintf(stderr, "can not find word %s", utf8Needles[i]);
            exit(EXIT_FAILURE);
        }

        needle_free(needles[i]);
    }

    automaton_free(automaton);
    list_free(list);
    trie_free(trie);
    trieOptions_free(options);

}