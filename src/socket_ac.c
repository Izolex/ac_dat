#include <event2/bufferevent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "ac.h"
#include "memory.h"
#include "tail.h"
#include "socket.h"
#include "socket_ac.h"


static void safeRead(BufferEvent *bufferEvent, void *data, size_t size);
static void safeWrite(BufferEvent *bufferEvent, const void *data, size_t size);

static SearchMode readSearchMode(BufferEvent *bufferEvent);
static Needle *readNeedle(BufferEvent *bufferEvent);

static inline void writeNeedle(BufferEvent *bufferEvent, const Occurrence *occurrence);
static inline void writeUserDataSize(BufferEvent *bufferEvent, const Occurrence *occurrence);
static inline void writeUserDataValue(BufferEvent *bufferEvent, const Occurrence *occurrence);
static inline void writeNoOccurrence(BufferEvent *bufferEvent);
static void writeOccurrence(BufferEvent *bufferEvent, SearchMode mode, Occurrence * restrict occurrence);


HandlerData *createHandlerData(const Automaton *automaton, const Tail *tail, const UserDataList *userDataList) {
    HandlerData *data = safeMalloc(sizeof(HandlerData), "handler data");
    data->automaton = automaton;
    data->tail = tail;
    data->userDataList = userDataList;

    return data;
}

void handlerData_free(HandlerData *data) {
    free(data);
    data = NULL;
}


static void safeRead(BufferEvent *bufferEvent, void *data, size_t size) {
    if (unlikely(bufferevent_read(bufferEvent, data, size) != size)) {
        error("can not read data from buffer");
    }
}

static void safeWrite(BufferEvent *bufferEvent, const void *data, size_t size) {
    if (unlikely(0 != bufferevent_write(bufferEvent, data, size))) {
        error("can not write data to buffer");
    }
}


static SearchMode readSearchMode(BufferEvent *bufferEvent) {
    SearchMode mode;
    safeRead(bufferEvent, &mode, 1);
    return mode;
}

static Needle *readNeedle(BufferEvent *bufferEvent) {
    int32_t needleLength = 0;
    safeRead(bufferEvent, &needleLength, sizeof(needleLength));

    Needle *characters = safeMalloc(sizeof(char) * (needleLength + 1), "AC search needle");
    safeRead(bufferEvent, characters, (size_t)needleLength);

    characters[needleLength] = '\0';
    return characters;
}


static inline void writeNeedle(BufferEvent *bufferEvent, const Occurrence *occurrence) {
    const int needleLength = occurrence_getNeedleLength(occurrence);
    safeWrite(bufferEvent, &needleLength, sizeof(needleLength));
    safeWrite(bufferEvent, occurrence_getNeedle(occurrence), needleLength);
}

static inline void writeUserDataSize(BufferEvent *bufferEvent, const Occurrence *occurrence) {
    safeWrite(bufferEvent, (const void *) &occurrence->userData.size, sizeof(UserDataSize));
}

static inline void writeUserDataValue(BufferEvent *bufferEvent, const Occurrence *occurrence) {
    safeWrite(bufferEvent, (const void *) occurrence->userData.value, occurrence->userData.size);
}

static inline void writeNoOccurrence(BufferEvent *bufferEvent) {
    safeWrite(bufferEvent, (const void *) &(UserDataSize){-1}, sizeof(UserDataSize));
}

static void writeOccurrence(BufferEvent *bufferEvent, const SearchMode mode, Occurrence * restrict occurrence) {
    if (NULL == occurrence) {
        writeNoOccurrence(bufferEvent);
        return;
    }
    writeUserDataSize(bufferEvent, occurrence);

    Occurrence *prev;
    do {
        if (mode & SEARCH_MODE_USER_DATA) {
            writeUserDataValue(bufferEvent, occurrence);
        }
        if (mode & SEARCH_MODE_NEEDLE) {
            writeNeedle(bufferEvent, occurrence);
            free(occurrence->needle.needle);
        }
        if (!(mode & SEARCH_MODE_FIRST)) {
            if (NULL == occurrence->next) {
                writeNoOccurrence(bufferEvent);
            } else {
                writeUserDataSize(bufferEvent, occurrence->next);
            }
        }

        prev = occurrence;
        occurrence = occurrence->next;

        occurrence_free(prev);
    } while (NULL != occurrence);
}

void ahoCorasickHandler(BufferEvent *bufferEvent, void *handlerContext) {
    const HandlerContext *context = (HandlerContext *)handlerContext;
    const HandlerData *data = (HandlerData *)context->handlerData;

    const SearchMode mode = readSearchMode(bufferEvent);
    Needle *needle = readNeedle(bufferEvent);

    Occurrence *occurrence = automaton_search(data->automaton, data->tail, data->userDataList, needle, mode);
    writeOccurrence(bufferEvent, mode, occurrence);

    free(needle);
}
