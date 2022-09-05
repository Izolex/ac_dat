#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "defs.h"


static void error(size_t size, const char *message);


static void error(const size_t size, const char *message) {
    fprintf(stderr, "can not allocate %lu memory for %s", size, message);
    exit(EXIT_FAILURE);
}

void *safeMalloc(const size_t size, const char *message) {
    void *pointer = malloc(size);
    if (unlikely(!pointer)) {
        error(size, message);
    }
    return pointer;
}

void *safeCalloc(const size_t count, const size_t size, const char *message) {
    void *pointer = calloc(count, size);
    if (unlikely(!pointer)) {
        error(size * count, message);
    }
    return pointer;
}

void *safeRealloc(void *pointer, const size_t count, const size_t size, const char *message) {
    pointer = realloc(pointer, count * size);
    if (unlikely(!pointer)) {
        error(size * count, message);
    }
    return pointer;
}

size_t calculateAllocation(const size_t size) {
    if (unlikely(size > INT32_MAX)) {
        fprintf(stderr, "needs more than a max of %d size", INT32_MAX);
        exit(EXIT_FAILURE);
    }
    const size_t newSize = size + (size_t) ceill((long double)size / 2) + 1;
    if (unlikely(newSize > INT32_MAX)) {
        return INT32_MAX;
    }
    return newSize;
}
