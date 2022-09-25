#ifndef __AC_DAT__TAIL__H__
#define __AC_DAT__TAIL__H__


struct tail;
struct tailBuilder;


struct tailBuilder *createTailBuilder(size_t size);
struct tail *createTail(size_t size);
struct tail *createTailFromBuilder(const struct tailBuilder *tailBuilder);

void tail_free(struct tail *tail);
void tailBuilder_free(struct tailBuilder *tailBuilder);

#endif
