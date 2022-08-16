#ifndef __CHAR_CODING_H__
#define __CHAR_CODING_H__

#include "unicode/utf8.h"

typedef int AlphabetSize;
typedef u_int32_t TrieChar;

typedef struct {
    TrieChar * characters;
    int length;
} TrieNeedle;

typedef struct {
    TrieChar chars[144697]; // Unicode 14.0.0
    AlphabetSize count;
} CharSet;


TrieNeedle *createNeedle(const char *needle);
void trieNeedle_free(TrieNeedle *needle);
TrieChar *allocateTrieChars(AlphabetSize size);


CharSet *charSet_create();
void charSet_free(CharSet *charSet);
void charSet_reset(CharSet *charSet);
void charSet_append(CharSet *charSet, TrieChar character);
void charSet_removeLast(CharSet *charSet);

#endif
