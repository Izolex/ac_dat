#include <stdio.h>
#include "ac.h"
#include "defs.h"
#include "dat.h"
#include "print.h"
#include "list.h"
#include "tail.h"
#include "user_data.h"


void list_print(const List *list) {
    printf("\n\n");
    printf("FirstFree: %u, lastFree: %u, front: %u, rear: %u", list->cells[0].next, list->cells[0].prev, list->front, list->rear);
    printf("\n\n");
    for (ListIndex i = 0; i < list->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (ListIndex i = 0; i < list->size; i++) {
        printf("%4d | ", list->cells[i].value);
    }
    printf("\n");
    for (ListIndex i = 0; i < list->size; i++) {
        printf("%4u | ", list->cells[i].next);
    }
    printf("\n");
    for (ListIndex i = 0; i < list->size; i++) {
        printf("%4u | ", list->cells[i].prev);
    }
    printf("\n\n");
}

void tailBuilder_print(const TailBuilder *tailBuilder) {
    for (TailIndex i = 0; i < tailBuilder->size; i++) {
        TailBuilderCell cell = tailBuilder->cells[i];
        printf("%d (%u, %d): ", i, cell.length, cell.nextFree);
        if (cell.length > 0) {
            for (TailCharIndex c = 0; c < cell.length; c++) {
                printf("%d ", cell.chars[c]);
            }
        } else {
            printf("empty");
        }
        printf("\n");
    }
    printf("\n\n");
}

void tail_print(const Tail *tail) {
    for (TailIndex i = 0; i < tail->size; i++) {
        TailCell cell = tail->cells[i];
        printf("%d (%u): ", i, cell.length);
        if (cell.length > 0) {
            for (TailCharIndex c = 0; c < cell.length; c++) {
                printf("%d ", cell.chars[c]);
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
    for (TrieIndex i = 0; i < trie->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (TrieIndex i = 0; i < trie->size; i++) {
        printf("%4d | ", trie->cells[i].base);
    }
    printf("\n");
    for (TrieIndex i = 0; i < trie->size; i++) {
        printf("%4d | ", trie->cells[i].check);
    }
    printf("\n");
    for (TrieIndex i = 0; i < trie->size; i++) {
        printf("%4d | ", trie->cells[i].children == NULL);
    }
    printf("\n");
    for (TrieIndex i = 0; i < trie->size; i++) {
        printf("%4u | ", trie->cells[i].children == NULL ? 0 : trie->cells[i].children->rear);
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
        printf("%4d | ", automaton->cells[i].base);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4d | ", automaton->cells[i].check);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4d | ", automaton->cells[i].fail);
    }
    printf("\n");
    for (int i = 0; i < automaton->size; i++) {
        printf("%4d | ", automaton->cells[i].output);
    }
    printf("\n\n");
}
