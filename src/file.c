#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ac.h"
#include "tail.h"
#include "typedefs.h"


static FILE *safeOpen(const char *filename, const char *mode);
static void safeClose(FILE *file);
static void safeWrite(const void * restrict pointer, size_t size, size_t items, FILE * restrict file);
static void safeRead(void * restrict pointer, size_t size, size_t items, FILE * restrict file);

static void file_storeAutomaton(FILE * restrict file, const Automaton *automaton);
static void file_storeTail(FILE * restrict file, const Tail *tail);
static Automaton *file_loadAutomaton(FILE * restrict file);
static Tail *file_loadTail(FILE * restrict file);


static void safeWrite(const void * restrict pointer, const size_t size, const size_t items, FILE * restrict file) {
    const size_t writtenSize = fwrite(pointer, size, items, file);
    if (unlikely(items != writtenSize)) {
        fprintf(stderr, "can not write %ld items to file (%ld written)", items, writtenSize);
        exit(EXIT_FAILURE);
    }
}

static void safeRead(void * restrict pointer, const size_t size, const size_t items, FILE * restrict file) {
    const size_t readSize = fread(pointer, size, items, file);
    if (unlikely(items != readSize)) {
        fprintf(stderr, "can not read %ld items from file (read %ld)", items, readSize);
        exit(EXIT_FAILURE);
    }
}

static FILE *safeOpen(const char * filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (unlikely(!file)) {
        fprintf(stderr, "can not open file \"%s\" with mode \"%s\"", filename, mode);
        exit(EXIT_FAILURE);
    }

    return file;
}

static void safeClose(FILE *file) {
    if (unlikely(0 != fclose(file))) {
        fprintf(stderr, "can not close file");
        exit(EXIT_FAILURE);
    }
}


static void file_storeAutomaton(FILE * restrict file, const Automaton *automaton) {
    safeWrite((const void*) &automaton->size, sizeof(AutomatonIndex), 1, file);

    for (AutomatonIndex i = 0; i < automaton->size; i++) {
        safeWrite((const void*) &automaton->cells[i], sizeof(AutomatonCell), 1, file);
    }
}

static void file_storeTail(FILE * restrict file, const Tail *tail) {
    TailIndex tailSize = 0;
    if (tail != NULL) {
        tailSize = tail->size;
    }

    safeWrite((const void*) & tailSize, sizeof(TailIndex), 1, file);

    if (tailSize) {
        for (TailIndex i = 1; i < tailSize; i++) {
            const TailCell cell = tail->cells[i];

            safeWrite((const void*) &cell.length, sizeof(TailCharIndex), 1, file);
            safeWrite((const void*) cell.chars, sizeof(Character), (size_t) cell.length, file);
        }
    }
}

void file_store(const char *targetPath, const Automaton *automaton, const Tail *tail) {
    FILE *file = safeOpen(targetPath, "w+b");

    file_storeAutomaton(file, automaton);
    file_storeTail(file, tail);

    safeClose(file);
}


static Automaton *file_loadAutomaton(FILE * restrict file) {
    AutomatonIndex automatonSize;
    safeRead((void*) &automatonSize, sizeof(AutomatonIndex), 1, file);

    Automaton *automaton = createAutomaton(automatonSize);
    for (AutomatonIndex i = 0; i < automatonSize; i++) {
        safeRead((void*) &automaton->cells[i], sizeof(AutomatonCell), 1, file);
    }

    return automaton;
}

static Tail *file_loadTail(FILE * restrict file) {
    TailIndex tailSize;
    safeRead(&tailSize, sizeof(TailIndex), 1, file);

    Tail *tail = createTail(tailSize);
    tail->cells[0] = (TailCell) {NULL,0};

    for (TailIndex i = 0; i < tailSize - 1; i++) {
        const TailIndex index = i + 1;
        TailCell cell;

        safeRead((void*) &cell.length, sizeof(TailCharIndex), 1, file);
        cell.chars = allocateCharacters(cell.length);
        safeRead((void*) cell.chars, sizeof(Character), (size_t) cell.length, file);

        tail->cells[index] = cell;
    }

    return tail;
}

FileData file_load(const char *targetPath) {
    if (unlikely(0 != access(targetPath, F_OK))) {
        fprintf(stderr, "File \"%s\" does not exists", targetPath);
        exit(EXIT_FAILURE);
    }

    FILE *file = safeOpen(targetPath, "rb");

    const FileData fileData = (FileData) {file_loadAutomaton(file), file_loadTail(file)};

    safeClose(file);

    return fileData;
}
