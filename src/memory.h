#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>
#include "typedefs.h"

void *safeCalloc(size_t count, size_t size, const char *message);
void *safeRealloc(void *pointer, size_t count, size_t size, const char *message);
size_t calculateAllocation(size_t currentSize);

#endif