#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include "defs.h"

typedef void (JobHandler)(void *userData);
typedef struct workerPool WorkerPool;
typedef struct worker Worker;

typedef struct job {
    void *userData;
    struct job *nextJob;
} Job;

struct workerPool {
    struct worker *workerList;
    JobHandler *jobHandler;
    pthread_mutex_t jobMutex;
    pthread_cond_t jobCondition;
    Job *jobFirst;
    Job *jobLast;
};

struct worker {
    pthread_t thread;
    struct workerPool *pool;
    struct worker *nextWorker;
    bool terminate;
    pthread_rwlock_t terminateLock;
};

int getAvailableCores(void);
Job *createJob(void *userData);
WorkerPool *createWorkerPool(int count, JobHandler *handler);
void workerPool_start(WorkerPool *pool);
void workerPool_stop(WorkerPool *pool);
void workerPool_join(WorkerPool *pool);
void workerPool_free(WorkerPool *pool);
void workerPool_addJob(WorkerPool *pool, Job *job);


#endif