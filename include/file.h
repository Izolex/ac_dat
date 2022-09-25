#ifndef __AC_DAT__FILE__H__
#define __AC_DAT__FILE__H__


#include "ac.h"
#include "user_data.h"


struct fileData {
    struct automaton *automaton;
    struct tail *tail;
    struct userDataList *userDataList;
};


void file_store(
    const char *targetPath,
    const struct automaton *automaton,
    const struct tail *tail,
    const struct userDataList *userDataList
);
struct fileData file_load(const char *targetPath);

#endif
