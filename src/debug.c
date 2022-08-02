#include <stdio.h>
#include "dat.h"

void tail_print(Tail *tail) {
    for (int i = 0; i < tail->cellsSize; i++) {
        TailCell cell = tail->cells[i];
        printf("%d (%ld): ", i, cell.nextFree);
        if (cell.length > 0) {
            for (int c = 0; c < cell.length; c++) {
                printf("%d ", cell.chars[c]);
            }
        } else {
            printf("empty");
        }
        printf("\n");
    }
    printf("\n\n");
}

void trieBuilder_print(Trie *trie) {
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
    printf("\n\n");
    tail_print(trie->tail);
}

uint32_t u8decode(uint32_t c)
{
    if (c >= 97) {
        return c - 95;
    }
    return 0;
}
