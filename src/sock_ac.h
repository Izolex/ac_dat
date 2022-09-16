#ifndef SOCK_AC_H
#define SOCK_AC_H

#include "defs.h"
#include "sock.h"

typedef struct {
   const Automaton *automaton;
   const Tail *tail;
   const UserDataList *userDataList;
} HandlerData;

void handlerData_free(HandlerData *data);
HandlerData *createHandlerData(const Automaton *automaton, const Tail *tail, const UserDataList *userDataList);
void ahoCorasickHandler(BufferEvent *bufferEvent, void *handlerContext);

#endif
