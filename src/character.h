#ifndef __CHAR_CODING_H__
#define __CHAR_CODING_H__

#include "typedefs.h"

Needle *createNeedle(const char *needle);
void needle_free(Needle *needle);
TrieChar *allocateTrieChars(AlphabetSize size);

CharSet *charSet_create();
void charSet_free(CharSet *charSet);
void charSet_reset(CharSet *charSet);
void charSet_append(CharSet *charSet, TrieChar character);
void charSet_removeLast(CharSet *charSet);

#endif
