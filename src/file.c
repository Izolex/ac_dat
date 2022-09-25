#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ac.h"
#include "file.h"
#include "tail.h"
#include "memory.h"
#include "user_data.h"
#include "definitions.h"


static FILE *safeOpen(const char *filename, const char *mode);
static void safeClose(FILE *file);
static void safeWrite(const void * restrict pointer, size_t size, size_t items, FILE * restrict file);
static void safeRead(void * restrict pointer, size_t size, size_t items, FILE * restrict file);

static void file_storeAutomaton(FILE * restrict file, const Automaton *automaton);
static void file_storeTail(FILE * restrict file, const Tail *tail);
static void file_storeUserDataList(FILE * restrict file, AutomatonIndex size, const UserDataList *userDataList);
static Automaton *file_loadAutomaton(FILE * restrict file);
static Tail *file_loadTail(FILE * restrict file);
static UserDataList *file_loadUserDataList(FILE * restrict file, AutomatonIndex size);


static void safeWrite(const void * restrict pointer, const size_t size, const size_t items, FILE * restrict file) {
    const size_t writtenSize = fwrite(pointer, size, items, file);
    if (unlikely(items != writtenSize)) {
        error("can not write to file");
    }
}

static void safeRead(void * restrict pointer, const size_t size, const size_t items, FILE * restrict file) {
    const size_t readSize = fread(pointer, size, items, file);
    if (unlikely(items != readSize)) {
        error("can not read from file");
    }
}

static FILE *safeOpen(const char * filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (unlikely(!file)) {
        error("can not open file");
    }

    return file;
}

static void safeClose(FILE *file) {
    if (unlikely(0 != fclose(file))) {
        error("can not close file");
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

static void file_storeUserDataList(FILE * restrict file, const AutomatonIndex size, const UserDataList *userDataList) {
    for (AutomatonIndex i = 0; i < size; i++) {
        safeWrite((const void *) &userDataList->cells[i].size, sizeof(UserDataSize), 1, file);
        safeWrite((const void *) userDataList->cells[i].value, 1, userDataList->cells[i].size, file);
    }
}

void file_store(const char *targetPath, const Automaton *automaton, const Tail *tail, const UserDataList *userDataList) {
    FILE *file = safeOpen(targetPath, "w+b");

    file_storeAutomaton(file, automaton);
    file_storeTail(file, tail);
    file_storeUserDataList(file, automaton->size, userDataList);

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

static UserDataList *file_loadUserDataList(FILE * restrict file, const AutomatonIndex size) {
    UserDataList *userDataList = createUserDataList(size);

    for (AutomatonIndex i = 0; i < size; i++) {
        safeRead((void*) &userDataList->cells[i].size, sizeof(UserDataSize), 1, file);
        userDataList->cells[i].value = safeMalloc(userDataList->cells[i].size, "user data");
        safeRead((void*) userDataList->cells[i].value, 1, userDataList->cells[i].size, file);
    }

    return userDataList;
}

FileData file_load(const char *targetPath) {
    if (unlikely(0 != access(targetPath, F_OK))) {
        error("file does not exists");
    }

    FILE *file = safeOpen(targetPath, "rb");

    FileData fileData;
    fileData.automaton = file_loadAutomaton(file);
    fileData.tail = file_loadTail(file);
    fileData.userDataList = file_loadUserDataList(file, fileData.automaton->size);

    safeClose(file);

    return fileData;
}
