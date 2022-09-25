#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/ac.h"
#include "../include/dat.h"
#include "../include/needle.h"
#include "../include/print.h"
#include "../include/tail.h"
#include "../include/list.h"
#include "../include/file.h"
#include "../include/thread.h"
#include "../include/socket.h"
#include "../include/socket_ac.h"
#include "../include/user_data.h"


static struct trieNeedle *safeCreateNeedle(const char *string) {
    struct trieNeedle *needle = createTrieNeedle(string);
    if (needle == NULL) {
        fprintf(stderr, "needle is not valid UTF8");
        exit(EXIT_FAILURE);
    }
    return needle;
}

int main(void) {
    const char *needles[] = {
// it is really long when printing trie with these characters
//            "\xc2\xa5\0", // 2B
//            "\xe2\xbf\x86\0", // 3B
//            "\xf0\x9f\xa6\x84\0", // 4B
//            "\xf0\x9f\xa7\x99\xf0\x9f\xa7\x99\0", // two same 4B emojis
            "bachelor\0",
            "jar\0",
            "badge\0",
            "baby\0",
            "ara\0",
            "bar\0",
            "arab\0",
            "baraba\0",
            "barbara\0",
    };

    const int needlesLength = sizeof(needles) / sizeof(needles[0]);
    struct userData userData = createUserData(4, "\xf0\x9f\xa7\x99");

    struct trieOptions *options = createTrieOptions(true, true, 4);
    struct tailBuilder *tailBuilder = createTailBuilder(10);
    struct userDataList *userDataList = createUserDataList(10);
    struct trie *trie = createTrie(options, tailBuilder, userDataList, 10);

    struct trieNeedle *trieNeedle;
    for (int i = 0; i < needlesLength; i++) {
        trieNeedle = safeCreateNeedle(needles[i]);
        printf("%s (%lu)\n", needles[i], trieNeedle_getLength(trieNeedle));
        trie_addNeedleWithData(trie, trieNeedle, userData);
        trieNeedle_free(trieNeedle);
    }

    trie_print(trie);
    tailBuilder_print(tailBuilder);
    userDataList_print(trie_getSize(trie), userDataList);

    struct tail *tail = createTailFromBuilder(tailBuilder);
    tailBuilder_free(tailBuilder);

    struct list *list = createList(10);
    struct automaton *automaton = createAutomaton_BFS(trie, list);

    automaton_print(automaton);
    tail_print(tail);

    const char *path = "../dictionary";
    file_store(path, automaton, tail, userDataList);

    automaton_free(automaton);
    tail_free(tail);
    userDataList_free(userDataList);

    struct fileData data = file_load(path);

    if (0 != remove(path)) {
        fprintf(stderr, "can not remove dictionary");
        exit(EXIT_FAILURE);
    }

    automaton = data.automaton;
    tail = data.tail;
    userDataList = data.userDataList;

    automaton_print(automaton);
    tail_print(tail);
    userDataList_print(automaton_getSize(automaton), userDataList);

    for (size_t i = 0; i < needlesLength; i++) {
        struct occurrence *occurrence = automaton_search(automaton, tail, userDataList, needles[i], SEARCH_MODE_USER_DATA|SEARCH_MODE_FIRST|SEARCH_MODE_NEEDLE);
        if (!occurrence) {
            fprintf(stderr, "can not find word %s", needles[i]);
            exit(EXIT_FAILURE);
        }

        needle_free(occurrence_getNeedle(occurrence));
        occurrence_free(occurrence);
    }

    struct timeval serverTimeout = { 2, 0 };
    HandlerData *handlerData = createHandlerData(automaton, tail, userDataList);
    struct workerPool *pool = createWorkerPool(getAvailableCores(), socketJobHandler);
    struct serverConfig *config = createServerConfig(
        -1,
        NULL,
        &serverTimeout,
        ahoCorasickHandler,
        handlerData
    );
//    SocketInfo *socketInfo = createUnixSocketInfo("../test.sock");
    struct socketInfo *socketInfo = createTCPSocketInfo(80);

    struct server *server = createServer(config, pool);

    server_run(server, socketInfo);

    handlerData_free(handlerData);
    serverConfig_free(config);
    automaton_free(automaton);
    userDataList_free(userDataList);
    tail_free(tail);
    list_free(list);
    trie_free(trie);
    trieOptions_free(options);
}
