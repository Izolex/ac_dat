#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "character.h"

typedef struct {
    char mask;
    char lead;
    int bites;
} utf8Mask;

utf8Mask *utf8MaskMap[] = {
        [0] = &(utf8Mask) {0b00111111, 0b10000000, 6},
        [1] = &(utf8Mask) {0b01111111, 0b00000000, 7},
        [2] = &(utf8Mask) {0b00011111, 0b11000000, 5},
        [3] = &(utf8Mask) {0b00001111, 0b11100000, 4},
        [4] = &(utf8Mask) {0b00000111, 0b11110000, 3},
};


int utf8Length(const char string) {
    for (int i = 1; i <= 4; i++) {
        utf8Mask *u = utf8MaskMap[i];

        if ((string & ~u->mask) == u->lead) {
            return i;
        }
    }

    return 0;
}

int utf8validate(const char *string) {
    int length = utf8Length(string[0]);
    if (length == 0) {
        return 0;
    }

    for (int i = 1; i < length; i++) {
        if ((string[i] & ~utf8MaskMap[0]->mask) != utf8MaskMap[0]->lead) {
            return 0;
        }
    }

    return 1;
}

TrieChar utf8toUnicode(const char string[4]) {
    int length = utf8Length(string[0]);
    int shift = utf8MaskMap[0]->bites * (length - 1);

    utf8Mask *u = utf8MaskMap[length];
    uint32_t unicode = (string[0] & u->mask) << shift;

    for (int i = 1; i < length; i++) {
        shift -= utf8MaskMap[0]->bites;
        unicode |= (string[i] & utf8MaskMap[0]->mask) << shift;
    }

    return unicode;
}


TrieNeedle *createNeedle(const char *needle) {
    TrieNeedle *trieNeedle = calloc(1, sizeof(TrieNeedle));
    if (trieNeedle == NULL) {
        fprintf(stderr, "can not allocate memory for trie needle");
        exit(1);
    }

    int index = 0;
    while (needle[index] != '\0') {
        int length = utf8Length(needle[index]);
        if (length == 0) {
            return NULL;
        }
        index += length;
        trieNeedle->length++;
    }

    trieNeedle->characters = malloc(sizeof(TrieChar) * trieNeedle->length);

    int needleIndex = 0;
    for (int i = 0; i < trieNeedle->length; i++) {
        int length = utf8Length(needle[needleIndex]);

        char utf8Char[4];
        for (int c = 0; c < length; c++) {
            utf8Char[c] = needle[needleIndex + c];
        }

        if (!utf8validate(utf8Char)) {
            return NULL;
        }

        trieNeedle->characters[i] = utf8toUnicode(utf8Char);
        needleIndex += length;
    }

    return trieNeedle;
}

void trieNeedle_free(TrieNeedle *needle) {
    free(needle);
}

TrieChar *allocateTrieChars(AlphabetSize size) {
    TrieChar *chars = malloc(sizeof(TrieChar) * size);
    if (chars == NULL) {
        fprintf(stderr, "can not allocate %lu memory for tail chars", sizeof(TrieChar) * size);
        exit(1);
    }

    return chars;
}


CharSet *charSet_create() {
    CharSet *ch = calloc(1, sizeof(CharSet));

    if (ch == NULL) {
        fprintf(stderr, "can not allocate %lu memory for charset", sizeof(CharSet));
        exit(1);
    }

    return ch;
}

void charSet_free(CharSet *charSet) {
    free(charSet);
}

void charSet_reset(CharSet *charSet) {
    memset(charSet, 0, sizeof(CharSet));
}

void charSet_append(CharSet *charSet, const TrieChar character) {
    charSet->chars[charSet->count] = character;
    charSet->count += 1;
}

void charSet_removeLast(CharSet *charSet) {
    charSet->chars[charSet->count - 1] = 0;
    charSet->count -= 1;
}