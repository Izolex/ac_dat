#include <event2/bufferevent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "ac.h"
#include "needle.h"
#include "mem.h"
#include "tail.h"
#include "socket.h"
#include "socket_ac.h"


static void safeRead(BufferEvent *bufferEvent, void *data, size_t size);
static void safeWrite(BufferEvent *bufferEvent, const void *data, size_t size);

static SearchMode readSearchMode(BufferEvent *bufferEvent);
static Needle *readNeedle(BufferEvent *bufferEvent);

static void writeNeedle(BufferEvent *bufferEvent, const Needle *needle);
static void writeUserDataSize(BufferEvent *bufferEvent, const Occurrence *occurrence);
static void writeUserDataValue(BufferEvent *bufferEvent, const Occurrence *occurrence);
static void writeOccurrence(BufferEvent *bufferEvent, SearchMode mode, Occurrence * restrict occurrence);
static void writeNoOccurrence(BufferEvent *bufferEvent);


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

    char characters[needleLength + 1];
    safeRead(bufferEvent, characters, (size_t)needleLength);

    characters[needleLength] = '\0';
    return createNeedle(characters);
}


static void writeNeedle(BufferEvent *bufferEvent, const Needle *needle) {
    safeWrite(bufferEvent, &needle->length, sizeof(NeedleIndex));
    safeWrite(bufferEvent, needle->characters, sizeof(Character) * needle->length);
}

static void writeUserDataSize(BufferEvent *bufferEvent, const Occurrence *occurrence) {
    safeWrite(bufferEvent, (const void *) &occurrence->userData.size, sizeof(UserDataSize));
}

static void writeUserDataValue(BufferEvent *bufferEvent, const Occurrence *occurrence) {
    safeWrite(bufferEvent, (const void *) occurrence->userData.value, occurrence->userData.size);
}

static void writeNoOccurrence(BufferEvent *bufferEvent) {
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
            writeNeedle(bufferEvent, occurrence->needle);
            needle_free(occurrence->needle);
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

    needle_free(needle);
}
