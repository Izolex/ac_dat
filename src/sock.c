#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include "sock.h"
#include "defs.h"
#include "mem.h"
#include "thread.h"


typedef struct event Event;
typedef struct evconnlistener ConnectionListener;

typedef struct {
    EventBase *base;
    const ServerConfig *config;
} JobContext;


static EventBase *createEventBase(void);
static SocketInfo *createSocketInfo(socklen_t length, struct sockaddr *address);
static JobContext *createJobContext(EventBase *base, const ServerConfig *config);
static HandlerContext *createHandlerContext(EventBase *base, void *handlerData);
static ConnectionListener *bindConnectionListener(const SocketInfo *socketInfo, const Server *server);
static void connection_event(BufferEvent *bufferEvent, short events, void *userData);
static void connection_accept(ConnectionListener *listener, evutil_socket_t descriptor, struct sockaddr *address, int length, void * userData);

static void jobContext_free(JobContext *context);
static void handlerContext_free(HandlerContext *context);

static void signalCallback(int signal, short events, void *userData);


void socketJobHandler(void * restrict userData) {
    JobContext *context = (JobContext*)userData;

    if (unlikely(0 > event_base_dispatch(context->base))) {
        error("can not dispatch job base");
    }

    jobContext_free(context);
}


static SocketInfo *createSocketInfo(const socklen_t length, struct sockaddr *address) {
    SocketInfo *info = safeMalloc(sizeof(SocketInfo), "socket info");
    info->length = length;
    info->address = address;

    return info;
}

SocketInfo *createUnixSocketInfo(const char path[104]) {
    struct sockaddr_un *address = safeMalloc(sizeof(struct sockaddr_un), "unix socket");
    memset(address, 0, sizeof(struct sockaddr_un));

    address->sun_family = AF_UNIX;

    strncpy(address->sun_path, path, sizeof(address->sun_path));

    return createSocketInfo(SUN_LEN(address), (struct sockaddr*) address);
}

SocketInfo *createTCPSocketInfo(const int port) {
    struct sockaddr_in *address = safeMalloc(sizeof(struct sockaddr_in), "internet socket");
    memset(address, 0, sizeof(struct sockaddr_in));

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = htonl(INADDR_ANY);
    address->sin_port = htons(port);

    return createSocketInfo(sizeof(struct sockaddr_in), (struct sockaddr*) address);
}

void socketInfo_free(SocketInfo *pointer) {
    free(pointer->address);
    free(pointer);
}


static JobContext *createJobContext(EventBase *base, const ServerConfig *config) {
    JobContext *context = safeMalloc(sizeof(JobContext), "job context");
    context->base = base;
    context->config = config;

    return context;
}

static void jobContext_free(JobContext *context) {
    free(context);
}

static HandlerContext *createHandlerContext(EventBase *base, void *handlerData) {
    HandlerContext *context = safeMalloc(sizeof(HandlerContext), "handler context");
    context->handlerData = handlerData;
    context->base = base;

    return context;
}

static void handlerContext_free(HandlerContext *context) {
    free(context);
}

static EventBase *createEventBase(void) {
    EventBase *base = event_base_new();
    if (unlikely(!base)) {
        error("could not initialize base");
    }

    return base;
}

static void connection_event(BufferEvent *bufferEvent, const short events, void * restrict userData) {
    unused(events);

    HandlerContext *context = (HandlerContext *)userData;

#ifdef VERBOSE
    if (likely(events & BEV_EVENT_EOF)) {
        log("connection closed");
    } else if (events & BEV_EVENT_TIMEOUT) {
        log("connection timeout");
    } else if (events & BEV_EVENT_ERROR) {
        log("error on connection");
    } else {
        log("unknown event");
    }
#endif

    bufferevent_free(bufferEvent);
    event_base_loopbreak(context->base);
    event_base_free(context->base);
    handlerContext_free(context);
}

static void connection_accept(
    ConnectionListener *listener,
    const evutil_socket_t descriptor,
    struct sockaddr *address,
    const int length,
    void *userData
) {
    unused(listener, address, length);

    const Server *server = (Server *) userData;

    EventBase *base = createEventBase();
    BufferEvent *bufferEvent = bufferevent_socket_new(base, descriptor, BEV_OPT_CLOSE_ON_FREE);
    if (unlikely(!bufferEvent)) {
        error("error constructing bufferEvent");
    }

    void *handlerContext = createHandlerContext(base, server->config->handlerData);
    void *jobContext = createJobContext(base, server->config);

    bufferevent_set_timeouts(bufferEvent, server->config->clientTimeout, NULL);
    bufferevent_setcb(
        bufferEvent,
        *server->config->handler,
        NULL,
        connection_event,
        handlerContext
    );
    bufferevent_enable(bufferEvent, EV_READ|EV_WRITE);

    Job *job = createJob(jobContext);
    workerPool_addJob(server->pool, job);
}


static ConnectionListener *bindConnectionListener(const SocketInfo *socketInfo, const Server *server) {
    ConnectionListener *listener = evconnlistener_new_bind(
        server->base,
        connection_accept,
        (void *) server,
        LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE,
        server->config->backlog,
        socketInfo->address,
        (int)socketInfo->length
    );
    if (unlikely(!listener)) {
        error("could not create a listener");
    }

    return listener;
}

static void signalCallback(int signal, short events, void *userData) {
    unused(events);

    Server *server = (Server*)userData;

#ifdef VERBOSE
    char signalStr[sizeof(int)];
    sprintf(signalStr, "%d", signal);
    log(signalStr);
    log("got signal");
#endif

    if (SIGINT == signal) {
        puts("got interrupt signal\n");
    }

    event_base_loopexit(server->base, server->config->serverTimeout);
}


ServerConfig *createServerConfig(
    int backlog,
    const Timeval *clientTimeout,
    const Timeval *serverTimeout,
    Handler *handler,
    void *handlerData
) {
    ServerConfig *config = safeMalloc(sizeof(ServerConfig), "server config");
    config->backlog = backlog;
    config->clientTimeout = clientTimeout;
    config->serverTimeout = serverTimeout;
    config->handler = handler;
    config->handlerData = handlerData;

    return config;
}

void serverConfig_free(ServerConfig *config) {
    free(config);
}


Server *createServer(const ServerConfig *config, WorkerPool *pool) {
    Server *server = safeMalloc(sizeof(Server), "server");
    server->config = config;
    server->pool = pool;
    server->base = createEventBase();

    return server;
}

void server_free(Server *server) {
    event_base_free(server->base);
    free(server);
}

void server_run(Server *server, const SocketInfo *socketInfo) {
    workerPool_start(server->pool);
    ConnectionListener *listener = bindConnectionListener(socketInfo, server);

    Event *signalEvent = evsignal_new(server->base, SIGINT, signalCallback, server);

    if (unlikely(!signalEvent || 0 > event_add(signalEvent, NULL))) {
        error("could not create/add a signal event");
    }

    if (unlikely(0 > event_base_dispatch(server->base))) {
        error("can not dispatch base");
    }

    workerPool_stop(server->pool);
    workerPool_join(server->pool);
    evconnlistener_free(listener);
    workerPool_free(server->pool);
}

void server_stop(Server *server) {
    event_base_loopexit(server->base, server->config->serverTimeout);
}
