#include <stdlib.h>
#include "needle.h"
#include "memory.h"


static inline char utf8Fill(Character unicode, int number, int shift);
static inline bool utf8Validate(unsigned char byte);
static inline Character unicodeFill(unsigned char byte, int number, int shift);


typedef struct {
    unsigned char mask, lead;
    char bites;
} utf8Mask;

utf8Mask *utf8MaskMap[] = {
        [0] = &(utf8Mask) { 0b00111111, 0b10000000, 6 },
        [1] = &(utf8Mask) { 0b01111111, 0b00000000, 7 },
        [2] = &(utf8Mask) { 0b00011111, 0b11000000, 5 },
        [3] = &(utf8Mask) { 0b00001111, 0b11100000, 4 },
        [4] = &(utf8Mask) { 0b00000111, 0b11110000, 3 },
};

typedef struct {
    Character last, first;
} unicodeMask;

unicodeMask *unicodeMaskMap[] = {
        [1] = &(unicodeMask) { 0x00007F, 0 },
        [2] = &(unicodeMask) { 0x0007FF, 0x000080 },
        [3] = &(unicodeMask) { 0x00FFFF, 0x000800 },
        [4] = &(unicodeMask) { 0x10FFFF, 0x010000 },
};


int unicodeLength(const Character unicode) {
    for (int i = 1; i <= 4; i++) {
        if (unicodeMaskMap[i]->first <= unicode && unicode <= unicodeMaskMap[i]->last) {
            return i;
        }
    }

    return 0;
}

int utf8Length(const unsigned char firstByte) {
    for (int i = 1; i <= 4; i++) {
        if ((firstByte & ~utf8MaskMap[i]->mask) == utf8MaskMap[i]->lead) {
            return i;
        }
    }

    return 0;
}


static inline char utf8Fill(const Character unicode, const int number, const int shift) {
    return (char)((unicode >> shift & utf8MaskMap[number]->mask) | utf8MaskMap[number]->lead);
}

void unicodeToUtf8(const Character unicode, const int length, char *output, const int outputStart) {
    const char bites = utf8MaskMap[0]->bites;

    output[outputStart] = utf8Fill(unicode, length, (length - 1) * bites);

    for (int c = 1; c < length; c++) {
        output[outputStart + c] = utf8Fill(unicode, 0, (length - c - 1) * bites);
    }
}


static inline bool utf8Validate(const unsigned char byte) {
    return (byte & ~utf8MaskMap[0]->mask) != utf8MaskMap[0]->lead;
}

static inline Character unicodeFill(const unsigned char byte, int number, const int shift) {
    return (byte & utf8MaskMap[number]->mask) << shift;
}

Character utf8ToUnicode(const char *needle, const int index, const int length) {
    const char bites = utf8MaskMap[0]->bites;
    Character unicode = unicodeFill((unsigned char)needle[index], length, (length - 1) * bites);

    for (int c = 1; c < length; c++) {
        if (unlikely(utf8Validate((unsigned char)needle[index + c]))) {
            return 0;
        }

        unicode |= unicodeFill((unsigned char)needle[index + c], 0, (length - c - 1) * bites);
    }

    return unicode;
}


TrieNeedle *createTrieNeedle(const char *needle) {
    TrieNeedle *trieNeedle = safeMalloc(sizeof(TrieNeedle), "needle");
    trieNeedle->length = 0;
    int index, length;

    index = 0;
    while (needle[index] != '\0') {
        length = utf8Length((unsigned char)needle[index]);

        if (unlikely(!length)) {
            return NULL;
        }

        index += length;
        trieNeedle->length++;
    }

    trieNeedle->characters = safeMalloc(trieNeedle->length * sizeof(Character), "needle characters");

    index = 0;
    for (TrieNeedleIndex i = 0; i < trieNeedle->length; i++) {
        length = utf8Length((unsigned char)needle[index]);
        const Character unicode = utf8ToUnicode(needle, index, length);

        if (unlikely(!unicode)) {
            return NULL;
        }

        trieNeedle->characters[i] = unicode;
        index += length;
    }

    return trieNeedle;
}

size_t trieNeedle_getLength(const TrieNeedle *needle) {
    return (size_t)needle->length;
}

void trieNeedle_free(TrieNeedle *needle) {
    free(needle->characters);
    free(needle);
    needle = NULL;
}

void needle_free(Needle *needle) {
    free(needle);
}
