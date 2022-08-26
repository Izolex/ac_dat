#include <stdlib.h>
#include <string.h>
#include "character.h"
#include "memory.h"


static size_t utf8Length(unsigned char byte);
static bool utf8validate(const unsigned char *string, size_t stringLength);
static TrieChar utf8toUnicode(const unsigned char string[4]);


typedef struct {
    unsigned char mask;
    unsigned char lead;
    char bites;
} utf8Mask;

utf8Mask *utf8MaskMap[] = {
        [0] = &(utf8Mask) {0b00111111, 0b10000000, 6},
        [1] = &(utf8Mask) {0b01111111, 0b00000000, 7},
        [2] = &(utf8Mask) {0b00011111, 0b11000000, 5},
        [3] = &(utf8Mask) {0b00001111, 0b11100000, 4},
        [4] = &(utf8Mask) {0b00000111, 0b11110000, 3},
};


static size_t utf8Length(const unsigned char byte) {
    for (int i = 1; i <= 4; i++) {
        if ((byte & ~utf8MaskMap[i]->mask) == utf8MaskMap[i]->lead) {
            return i;
        }
    }

    return 0;
}

static bool utf8validate(const unsigned char *string, const size_t length) {
    const utf8Mask *u8mask = utf8MaskMap[0];
    for (size_t i = 1; i < length; i++) {
        if ((string[i] & ~u8mask->mask) != u8mask->lead) {
            return false;
        }
    }

    return true;
}

static TrieChar utf8toUnicode(const unsigned char string[4]) {
    const size_t length = utf8Length(string[0]);
    unsigned char shift = utf8MaskMap[0]->bites * (length - 1);

    uint32_t unicode = (string[0] & utf8MaskMap[length]->mask) << shift;

    for (size_t i = 1; i < length; i++) {
        shift -= utf8MaskMap[0]->bites;
        unicode |= (string[i] & utf8MaskMap[0]->mask) << shift;
    }

    return unicode;
}


Needle *createNeedle(const char *needle) {
    Needle *trieNeedle = safeMalloc(sizeof(Needle), "needle");
    trieNeedle->length = 0;
    size_t index;

    index = 0;
    while (needle[index] != '\0') {
        const size_t length = utf8Length((unsigned char)needle[index]);
        if (length == 0) {
            return NULL;
        }
        index += length;
        trieNeedle->length++;
    }

    trieNeedle->characters = safeMalloc(trieNeedle->length * sizeof(TrieChar), "needle characters");

    index = 0;
    for (NeedleIndex i = 0; i < trieNeedle->length; i++) {
        const size_t length = utf8Length((unsigned char)needle[index]);

        unsigned char utf8Char[4] = {0,0,0,0};
        for (size_t c = 0; c < length; c++) {
            utf8Char[c] = (unsigned char)needle[index + c];
        }

        if (!utf8validate(utf8Char, length)) {
            return NULL;
        }

        trieNeedle->characters[i] = utf8toUnicode(utf8Char);
        index += length;
    }

    return trieNeedle;
}

void needle_free(Needle *needle) {
    free(needle->characters);
    free(needle);
    needle = NULL;
}
