#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include "defs.h"

void *safeCalloc(size_t count, size_t size, const char *message);
void *safeMalloc(size_t size, const char *message);
void *safeRealloc(void *pointer, size_t count, size_t size, const char *message);
size_t calculateAllocation(size_t currentSize);

#endif