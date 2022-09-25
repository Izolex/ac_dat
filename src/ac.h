#ifndef AC_H
#define AC_H

#include "../include/ac.h"
#include "user_data.h"


typedef int32_t AutomatonTransition, AutomatonIndex;

typedef struct {
    AutomatonIndex base, check, fail, output;
} AutomatonCell;

typedef struct automaton {
    AutomatonIndex size;
    AutomatonCell *cells;
} Automaton;

typedef struct occurrence {
    struct occurrence *next;
    struct needle *needle;
    UserData userData;
} Occurrence;
typedef enum searchMode SearchMode;

Automaton *createAutomaton(AutomatonIndex initialSize);

#endif
