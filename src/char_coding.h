#ifndef __CHAR_CODING_H__
#define __CHAR_CODING_H__

#include "unicode/utf8.h"

int utf8Length(const char * string);
int utf8validate(const char * string);
uint32_t utf8toUnicode(const char string[4]);

#endif
