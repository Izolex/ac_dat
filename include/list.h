#ifndef __AC_DAT__LIST__H__
#define __AC_DAT__LIST__H__


struct list;


void list_free(struct list *list);
struct list *createList(size_t initialSize);

#endif
