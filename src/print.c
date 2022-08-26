#include <stdio.h>
#include "typedefs.h"
#include "ac.h"

void list_print(const List *list) {
    printf("\n\n");
    printf("FirstFree: %ld, lastFree: %ld, front: %ld, rear: %ld", list->cells[0].next, list->cells[0].prev, list->front, list->rear);
    printf("\n\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4ld | ", list->cells[i].value);
    }
    printf("\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4ld | ", list->cells[i].next);
    }
    printf("\n");
    for (int i = 0; i < list->size; i++) {
        printf("%4ld | ", list->cells[i].prev);
    }
    printf("\n\n");
}

void tail_print(const Tail *tail) {
    for (int i = 0; i < tail->size; i++) {
        TailCell cell = tail->cells[i];
        printf("%d (%ld, %ld): ", i, cell.length, cell.nextFree);
        if (cell.length > 0) {
            for (int c = 0; c < cell.length; c++) {
                printf("%ld ", cell.chars[c]);
            }
        } else {
            printf("empty");
        }
        printf("\n");
    }
    printf("\n\n");
}

void trie_print(const Trie *trie) {
    printf("\n");
    for (int i = 0; i < trie->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (int i = 0; i < trie->size; i++) {
        printf("%4ld | ", trie->cells[i].base);
    }
    printf("\n");
    for (int i = 0; i < trie->size; i++) {
        printf("%4ld | ", trie->cells[i].check);
    }
    printf("\n");
    for (int i = 0; i < trie->size; i++) {
        printf("%4d | ", trie->cells[i].children == NULL);
    }
    printf("\n");
    for (int i = 0; i < trie->size; i++) {
        printf("%4ld | ", trie->cells[i].children == NULL ? 0 : trie->cells[i].children->rear);
    }
    printf("\n\n");
}

void automaton_print(const Automaton *automaton) {
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4ld | ", automaton->cells[i].base);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4ld | ", automaton->cells[i].check);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4ld | ", automaton->cells[i].fail);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4ld | ", automaton->cells[i].output);
    }
    printf("\n\n");
}
