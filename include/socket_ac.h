#ifndef __AC_DAT__SOCKET_AC__H__
#define __AC_DAT__SOCKET_AC__H__


#include "ac.h"
#include "socket.h"
#include "user_data.h"


typedef struct {
   const struct automaton *automaton;
   const struct tail *tail;
   const struct userDataList *userDataList;
} HandlerData;


void handlerData_free(HandlerData *data);
HandlerData *createHandlerData(const struct automaton *automaton, const struct tail *tail, const struct userDataList *userDataList);
void ahoCorasickHandler(struct bufferevent *bufferEvent, void *handlerContext);

#endif
