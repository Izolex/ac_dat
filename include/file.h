#ifndef __AC_DAT__FILE__H__
#define __AC_DAT__FILE__H__


#include "ac.h"


struct fileData {
    struct automaton *automaton;
    struct tail *tail;
};


void file_store(
    const char *targetPath,
    const struct automaton *automaton,
    const struct tail *tail
);
struct fileData file_load(const char *targetPath);

#endif
