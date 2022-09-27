#ifndef NEEDLE_H
#define NEEDLE_H

#include "../include/needle.h"

typedef u_int32_t TrieNeedleIndex;

typedef struct trieNeedle {
    Character *characters;
    TrieNeedleIndex length;
} TrieNeedle;


int utf8Length(unsigned char firstByte);
int unicodeLength(Character unicode);

void unicodeToUtf8(Character unicode, int length, char *output, int outputStart);
Character utf8ToUnicode(const char *needle, int index, int length);

#endif
