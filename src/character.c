#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "character.h"

typedef struct {
    unsigned char mask;
    unsigned char lead;
    unsigned char bites;
} utf8Mask;

utf8Mask *utf8MaskMap[] = {
        [0] = &(utf8Mask) {0b00111111, 0b10000000, 6},
        [1] = &(utf8Mask) {0b01111111, 0b00000000, 7},
        [2] = &(utf8Mask) {0b00011111, 0b11000000, 5},
        [3] = &(utf8Mask) {0b00001111, 0b11100000, 4},
        [4] = &(utf8Mask) {0b00000111, 0b11110000, 3},
};

unsigned char utf8Length(const char string) {
    for (char i = 1; i <= 4; i++) {
        utf8Mask *u = utf8MaskMap[i];

        if ((string & ~u->mask) == u->lead) {
            return i;
        }
    }

    return 0;
}

unsigned char utf8validate(const char *string) {
    unsigned char length = utf8Length(string[0]);
    if (length == 0) {
        return 0;
    }

    for (unsigned char i = 1; i < length; i++) {
        if ((string[i] & ~utf8MaskMap[0]->mask) != utf8MaskMap[0]->lead) {
            return 0;
        }
    }

    return 1;
}

TrieChar utf8toUnicode(const char string[4]) {
    unsigned char length = utf8Length(string[0]);
    unsigned char shift = utf8MaskMap[0]->bites * (length - 1);

    utf8Mask *u = utf8MaskMap[length];
    uint32_t unicode = (string[0] & u->mask) << shift;

    for (unsigned char i = 1; i < length; i++) {
        shift -= utf8MaskMap[0]->bites;
        unicode |= (string[i] & utf8MaskMap[0]->mask) << shift;
    }

    return unicode;
}


Needle *createNeedle(const char *needle) {
    Needle *trieNeedle = calloc(1, sizeof(Needle));
    if (trieNeedle == NULL) {
        fprintf(stderr, "can not allocate memory for trie needle");
        exit(1);
    }

    AlphabetSize index = 0;
    while (needle[index] != '\0') {
        int length = utf8Length(needle[index]);
        if (length == 0) {
            return NULL;
        }
        index += length;
        trieNeedle->length++;
    }

    trieNeedle->characters = calloc(trieNeedle->length, sizeof(TrieChar));
    if (trieNeedle->characters == NULL) {
        fprintf(stderr, "can not allocate %lu memory for needle chars", sizeof(TrieChar) * trieNeedle->length);
        exit(1);
    }

    AlphabetSize needleIndex = 0;
    for (AlphabetSize i = 0; i < trieNeedle->length; i++) {
        unsigned char length = utf8Length(needle[needleIndex]);

        char utf8Char[4];
        for (unsigned char c = 0; c < length; c++) {
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

void needle_free(Needle *needle) {
    free(needle);
}

TrieChar *allocateTrieChars(const AlphabetSize size) {
    TrieChar *chars = calloc(size, sizeof(TrieChar));
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
