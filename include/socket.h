#ifndef __AC_DAT__SOCKET__H__
#define __AC_DAT__SOCKET__H__


#include <event2/bufferevent.h>
#include "thread.h"


typedef void (Handler)(struct bufferevent *bufferEvent, void *handlerContext);

struct socketInfo;
struct serverConfig;
struct server;


void socketJobHandler(void * restrict userData);

struct serverConfig *createServerConfig(
        int backlog,
        const struct timeval *clientTimeout,
        const struct timeval *serverTimeout,
        Handler *handler,
        void *handlerData
);
void serverConfig_free(struct serverConfig *config);

struct server *createServer(const struct serverConfig *config, struct workerPool *pool);
void server_run(struct server *server, const struct socketInfo *socketInfo);
void server_stop(struct server *server);
void server_free(struct server *server);

struct socketInfo *createUnixSocketInfo(const char path[104]);
struct socketInfo *createTCPSocketInfo(int port);

void socketInfo_free(struct socketInfo *pointer);

#endif
