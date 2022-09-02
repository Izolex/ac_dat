#ifndef TAIL_H
#define TAIL_H

#include "typedefs.h"

TailBuilder *createTailBuilder(TailIndex size);
void tailBuilder_free(TailBuilder *tailBuilder);
void tailBuilder_freeCell(TailBuilder *tailBuilder, TailIndex index);
void tailBuilder_minimize(TailBuilder *tailBuilder);
TailIndex tailBuilder_insertChars(TailBuilder *tailBuilder, TailCharIndex length, Character *string);
TailBuilderCell tailBuilder_getCell(const TailBuilder *tailBuilder, TailIndex index);
Character *allocateCharacters(TailCharIndex size);

#endif
