#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include "definitions.h"

void *safeAlloc(size_t size, const char *message);
void *safeRealloc(void *pointer, size_t oldCount, size_t newCount, size_t size, const char *message);
void resetMemory(void *pointer, size_t size);
size_t calculateAllocation(size_t currentSize);

#endif
