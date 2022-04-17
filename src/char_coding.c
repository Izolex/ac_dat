#include "unicode/utf8.h"

typedef struct {
    char mask;
    char lead;
    int bites;
} utf8Mask;

utf8Mask *utf8MaskMap[] = {
        [0] = &(utf8Mask){0b00111111, 0b10000000, 6},
        [1] = &(utf8Mask){0b01111111, 0b00000000, 7},
        [2] = &(utf8Mask){0b00011111, 0b11000000, 5},
        [3] = &(utf8Mask){0b00001111, 0b11100000, 4},
        [4] = &(utf8Mask){0b00000111, 0b11110000, 3},
};

int utf8Length(const char * string) {
    for (int i = 1; i <= 4; i++) {
        utf8Mask *u = utf8MaskMap[i];

        if ((string[0] & ~u->mask) == u->lead) {
            return i;
        }
    }

    return 0;
};

int utf8validate(const char * string) {
    int length = utf8Length(string);
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

uint32_t utf8toUnicode(const char string[4]) {
    int length = utf8Length(string);
    int shift = utf8MaskMap[0]->bites * (length - 1);

    utf8Mask *u = utf8MaskMap[length];
    uint32_t unicode = (string[0] & u->mask) << shift;

    for (int i = 1; i < length; i++) {
        shift -= utf8MaskMap[0]->bites;
        unicode |= (string[i] & utf8MaskMap[0]->mask) << shift;
    }

    return unicode;
}
