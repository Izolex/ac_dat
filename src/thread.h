#ifndef THREAD_H
#define THREAD_H

#include "../include/thread.h"
#include "definitions.h"

typedef struct job {
    void *userData;
    struct job *nextJob;
} Job;

typedef struct workerPool {
    struct worker *workerList;
    JobHandler *jobHandler;
    pthread_mutex_t jobMutex;
    pthread_cond_t jobCondition;
    Job *jobFirst;
    Job *jobLast;
} WorkerPool;

typedef struct worker {
    pthread_t thread;
    struct workerPool *pool;
    struct worker *nextWorker;
    bool terminate;
    pthread_rwlock_t terminateLock;
} Worker;

Job *createJob(void *userData);
void workerPool_join(WorkerPool *pool);
void workerPool_addJob(WorkerPool *pool, Job *job);

#endif
