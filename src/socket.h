#ifndef SOCK_H
#define SOCK_H

#include <event2/bufferevent.h>
#include "../include/socket.h"
#include "thread.h"

typedef struct socketInfo {
    socklen_t length;
    struct sockaddr *address;
} SocketInfo;

typedef struct event_base EventBase;
typedef struct bufferevent BufferEvent;
typedef struct timeval Timeval;

typedef struct {
    void *handlerData;
    EventBase *base;
} HandlerContext;

typedef struct serverConfig {
    int backlog;
    const Timeval *clientTimeout;
    const Timeval *serverTimeout;
    Handler *handler;
    void *handlerData;
} ServerConfig;

typedef struct server {
    EventBase *base;
    WorkerPool *pool;
    const ServerConfig *config;
} Server;

#endif
