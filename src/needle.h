#ifndef NEEDLE_H
#define NEEDLE_H

#include "../include/needle.h"

typedef u_int32_t NeedleIndex;

typedef struct needle {
    Character *characters;
    NeedleIndex length;
} Needle;

#endif
