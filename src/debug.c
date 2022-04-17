#include <stdio.h>
#include "dat.h"

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
}

uint32_t u8decode(uint32_t c)
{
    if (c >= 97) {
        return c - 95;
    }
    return 0;
}
