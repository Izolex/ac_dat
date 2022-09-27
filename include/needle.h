#ifndef __AC_DAT__NEEDLE__H__
#define __AC_DAT__NEEDLE__H__


typedef int32_t Character;
typedef char Needle;

struct trieNeedle;


struct trieNeedle *createTrieNeedle(const char *needle);
size_t trieNeedle_getLength(const struct trieNeedle *needle);
void trieNeedle_free(struct trieNeedle *needle);
void needle_free(Needle *needle);

#endif
