#ifndef __AC_DAT__NEEDLE__H__
#define __AC_DAT__NEEDLE__H__


typedef int32_t Character;

struct needle;


struct needle *createNeedle(const char *needle);
size_t needle_getLength(struct needle *needle);
void needle_free(struct needle *needle);

#endif
