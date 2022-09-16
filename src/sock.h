#ifndef SOCK_H
#define SOCK_H

#include <event2/bufferevent.h>
#include "defs.h"
#include "thread.h"

typedef struct {
    socklen_t length;
    struct sockaddr *address;
} SocketInfo;

typedef struct event_base EventBase;
typedef struct bufferevent BufferEvent;
typedef struct timeval Timeval;

typedef void (Handler)(BufferEvent *bufferEvent, void *handlerContext);

typedef struct {
    void *handlerData;
    EventBase *base;
} HandlerContext;

typedef struct {
    int backlog;
    const Timeval *clientTimeout;
    const Timeval *serverTimeout;
    Handler *handler;
    void *handlerData;
} ServerConfig;

typedef struct {
    EventBase *base;
    WorkerPool *pool;
    const ServerConfig *config;
} Server;


void socketJobHandler(void * restrict userData);

ServerConfig *createServerConfig(int backlog, const Timeval *clientTimeout, const Timeval *serverTimeout, Handler *handler, void *handlerData);
void serverConfig_free(ServerConfig *config);

Server *createServer(const ServerConfig *config, WorkerPool *pool);
void server_run(Server *server, const SocketInfo *socketInfo);
void server_stop(Server *server);
void server_free(Server *server);

void socketInfo_free(SocketInfo *pointer);
SocketInfo *createUnixSocketInfo(const char path[104]);
SocketInfo *createTCPSocketInfo(int port);

#endif
