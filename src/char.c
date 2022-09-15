#include <stdlib.h>
#include <string.h>
#include "char.h"
#include "mem.h"


static size_t utf8Length(unsigned char byte);
static bool utf8Validate(unsigned char byte);
static Character unicodeFill(const unsigned char byte, size_t number, const unsigned char shift);
static Character createCharacter(const char *needle, size_t index, size_t length);


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

static bool utf8Validate(const unsigned char byte) {
    return (byte & ~utf8MaskMap[0]->mask) != utf8MaskMap[0]->lead;
}

static Character unicodeFill(const unsigned char byte, size_t number, const unsigned char shift) {
    return (byte & utf8MaskMap[number]->mask) << shift;
}

static Character createCharacter(const char *needle, const size_t index, const size_t length) {
    const char bites = utf8MaskMap[0]->bites;
    Character unicode = unicodeFill((unsigned char)needle[index], length, (length - 1) * bites);

    for (size_t c = 1; c < length; c++) {
        if (unlikely(utf8Validate((unsigned char)needle[index + c]))) {
            return 0;
        }

        unicode |= unicodeFill((unsigned char)needle[index + c], 0, (length - c - 1) * bites);
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

        if (unlikely(!length)) {
            return NULL;
        }

        index += length;
        trieNeedle->length++;
    }

    trieNeedle->characters = safeMalloc(trieNeedle->length * sizeof(Character), "needle characters");

    index = 0;
    for (NeedleIndex i = 0; i < trieNeedle->length; i++) {
        const size_t length = utf8Length((unsigned char)needle[index]);
        const Character unicode = createCharacter(needle, index, length);

        if (unlikely(!unicode)) {
            return NULL;
        }

        trieNeedle->characters[i] = unicode;
        index += length;
    }

    return trieNeedle;
}

void needle_free(Needle *needle) {
    free(needle->characters);
    free(needle);
    needle = NULL;
}
