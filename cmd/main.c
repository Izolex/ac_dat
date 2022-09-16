#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../include/ac.h"
#include "../include/tail.h"
#include "../include/file.h"
#include "../include/thread.h"
#include "../include/socket.h"
#include "../include/socket_ac.h"


static const char *mustEnv(const char *name) {
    const char *value = getenv(name);
    if (NULL == value) {
        fprintf(stderr, "env name %s does not exists\n", name);
        exit(EXIT_FAILURE);
    }
    return value;
}

static int toInt(const char *value) {
    return (int)strtol(value, NULL, 0);
}

static struct socketInfo *createSocketInfo(void) {
    const char *socket = mustEnv("SOCKET");

    if (0 == strcmp("unix", socket)) {
        return createUnixSocketInfo(mustEnv("SOCKET_PATH"));
    } else if (0 == strcmp("tcp", socket)) {
        return createTCPSocketInfo(toInt(mustEnv("TCP_PORT")));
    } else {
        fprintf(stderr, "unknown socket env value %s\n", socket);
        exit(EXIT_FAILURE);
    }
}

static void *createTimeVal(const char *name) {
    const char *value = getenv(name);
    if (NULL == value) {
        return NULL;
    }

    return &(struct timeval){ toInt(value), 0 };
}

static int createBacklog(void) {
    const char *value = getenv("BACKLOG");
    if (NULL == value) {
        return -1;
    }

    return toInt(value);
}

static int createWorkers(void) {
    const char *value = getenv("WORKERS");
    if (NULL == value) {
        return getAvailableCores();
    }

    return toInt(value);
}

int main(void) {
    const char *filePath = mustEnv("DICTIONARY");
    if (0 != access(filePath, F_OK|R_OK)) {
        fprintf(stderr, "dictionary does not exists %s\n", filePath);
        exit(EXIT_FAILURE);
    }

    const int backlog = createBacklog();
    struct socketInfo *socketInfo = createSocketInfo();
    struct timeval *serverTimeout = createTimeVal("SERVER_TIMEOUT");
    struct timeval *clientTimeout = createTimeVal("CLIENT_TIMEOUT");

    struct fileData data = file_load(filePath);
    HandlerData *handlerData = createHandlerData(data.automaton, data.tail, data.userDataList);
    struct workerPool *pool = createWorkerPool(createWorkers(), socketJobHandler);
    struct serverConfig *config = createServerConfig(
        backlog,
        clientTimeout,
        serverTimeout,
        ahoCorasickHandler,
        handlerData
    );
    struct server *server = createServer(config, pool);

    server_run(server, socketInfo);

    serverConfig_free(config);
    handlerData_free(handlerData);
    socketInfo_free(socketInfo);
    automaton_free(data.automaton);
    if (NULL != data.tail) tail_free(data.tail);
    if (NULL != serverTimeout) free(serverTimeout);
    if (NULL != clientTimeout) free(clientTimeout);
}
