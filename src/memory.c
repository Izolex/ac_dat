#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>


static void error(size_t size, const char *message);


static void error(const size_t size, const char *message) {
    fprintf(stderr, "can not allocate %lu memory for %s", size, message);
    exit(EXIT_FAILURE);
}

void *safeCalloc(const size_t count, const size_t size, const char *message) {
    void *pointer = calloc(count, size);
    if (pointer == NULL) {
        error(size * count, message);
    }
    return pointer;
}

void *safeRealloc(void *pointer, const size_t count, const size_t size, const char *message) {
    pointer = realloc(pointer, count * size);
    if (pointer == NULL) {
        error(size * count, message);
    }
    return pointer;
}

size_t calculateAllocation(const size_t size) {
    if (size > LONG_MAX) {
        fprintf(stderr, "needs more than a max of %lu size", LONG_MAX);
        exit(EXIT_FAILURE);
    }
    const size_t newSize = size + (size_t) ceill((long double)size / 2) + 1;
    if (newSize > LONG_MAX) {
        return LONG_MAX;
    }
    return newSize;
}
