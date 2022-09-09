#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "mem.h"
#include "defs.h"


static void allocError(const char *message);


static void allocError(const char *message) {
    error(strcat("can not allocate memory for", message));
}

void *safeMalloc(const size_t size, const char *message) {
    void *pointer = malloc(size);
    if (unlikely(!pointer)) {
        allocError(message);
    }
    return pointer;
}

void *safeCalloc(const size_t count, const size_t size, const char *message) {
    void *pointer = calloc(count, size);
    if (unlikely(!pointer)) {
        allocError(message);
    }
    return pointer;
}

void *safeRealloc(void *pointer, const size_t count, const size_t size, const char *message) {
    pointer = realloc(pointer, count * size);
    if (unlikely(!pointer)) {
        allocError(message);
    }
    return pointer;
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
