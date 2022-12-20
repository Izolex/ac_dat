#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "memory.h"
#include "definitions.h"


#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif


static void allocError(const char *message);


static void allocError(const char *message) {
    puts(message);
    error("can not allocate memory");
}

void *safeAlloc(const size_t neededSize, const char *message) {
    void *pointer = aligned_alloc(
            CACHE_LINE_SIZE,
            (size_t) ceil((double) neededSize / CACHE_LINE_SIZE) * CACHE_LINE_SIZE
    );

    if (unlikely(!pointer)) {
        allocError(message);
    }

    return pointer;
}

void *safeRealloc(void *pointer, const size_t oldCount, const size_t newCount, const size_t size, const char *message) {
    void *newPointer;
    if (newCount <= oldCount) {
        newPointer = realloc(pointer, newCount * size);
    } else {
        newPointer = safeAlloc(newCount * size, message);
        memcpy(newPointer, pointer, oldCount * size);
        free(pointer);
    }

    if (unlikely(!pointer)) {
        allocError(message);
    }

    return newPointer;
}

void resetMemory(void *pointer, const size_t size) {
    bzero(pointer, size);
}

size_t calculateAllocation(const size_t size) {
    if (unlikely(size > INT32_MAX)) {
        error("allocation needs more than a max of signed 32 bit integer");
    }
    const size_t newSize = size + (size_t) ceill((long double)size / 2) + 1;
    if (unlikely(newSize > INT32_MAX)) {
        return INT32_MAX;
    }
    return newSize;
}
