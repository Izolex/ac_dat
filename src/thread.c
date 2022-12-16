#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "definitions.h"
#include "memory.h"
#include "thread.h"


static void safeMutexLock(pthread_mutex_t * mutex);
static void safeMutexUnlock(pthread_mutex_t * mutex);

static void job_free(Job *job);

static Worker *createWorker(WorkerPool *pool);
static void worker_free(Worker *worker);
static void *worker_function(void *userData);
static void worker_setTerminate(Worker *worker);
static bool worker_shouldTerminate(Worker *worker);


const pthread_cond_t initializerRWLock = PTHREAD_RWLOCK_INITIALIZER;
const pthread_cond_t initializerCondition = PTHREAD_COND_INITIALIZER;
const pthread_mutex_t initializerMutex = PTHREAD_MUTEX_INITIALIZER;


static void safeMutexLock(pthread_mutex_t *mutex) {
    if (unlikely(0 != pthread_mutex_lock(mutex))) {
        error("can not lock mutex");
    }
}

static void safeMutexUnlock(pthread_mutex_t *mutex) {
    if (unlikely(0 != pthread_mutex_unlock(mutex))) {
        error("can not unlock mutex");
    }
}


int getAvailableCores(void) {
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
}


Job *createJob(void *userData) {
    Job *job = safeAlloc(sizeof(Job), "job");
    job->userData = userData;
    job->nextJob = NULL;

    return job;
}

static void job_free(Job *job) {
    free(job);
}


static void worker_free(Worker *worker) {
    free(worker);
    worker = NULL;
}

static bool worker_shouldTerminate(Worker *worker) {
    bool terminate;

    if (unlikely(0 != pthread_rwlock_rdlock(&worker->terminateLock))) {
        error("can not lock terminate lock for write");
    }
    terminate = worker->terminate;
    if (unlikely(0 != pthread_rwlock_unlock(&worker->terminateLock))) {
        error("can not unlock terminate lock after write");
    }

    return terminate;
}

static void worker_setTerminate(Worker *worker) {
    if (unlikely(0 != pthread_rwlock_wrlock(&worker->terminateLock))) {
        error("can not lock terminate lock for read");
    }
    worker->terminate = true;
    if (unlikely(0 != pthread_rwlock_unlock(&worker->terminateLock))) {
        error("can not unlock terminate lock after read");
    }
}


static void *worker_function(void *userData) {
    Job *job;
    Worker *worker = (Worker*)userData;

    for (;;) {
        safeMutexLock(&worker->pool->jobMutex);
        while (NULL == worker->pool->jobFirst) {
            if (unlikely(worker_shouldTerminate(worker))) {
                break;
            }

            if (unlikely(0 != pthread_cond_wait(&worker->pool->jobCondition, &worker->pool->jobMutex))) {
                error("can not wait for condition");
            }
        }

        job = worker->pool->jobFirst;
        if (NULL != job) {
            if (worker->pool->jobFirst == worker->pool->jobLast) {
                worker->pool->jobFirst = worker->pool->jobLast = NULL;
            } else {
                worker->pool->jobFirst = job->nextJob;
            }
        }

        safeMutexUnlock(&worker->pool->jobMutex);

        if (unlikely(worker_shouldTerminate(worker))) {
            break;
        }

        if (NULL == job) {
            continue;
        }

        worker->pool->jobHandler(job->userData);
        job_free(job);
    }

    return NULL;
}

static Worker *createWorker(WorkerPool *pool) {
    Worker *worker = safeAlloc(sizeof(Worker), "worker pool");
    worker->pool = pool;
    worker->thread = 0;
    worker->nextWorker = NULL;
    worker->terminate = false;

    memcpy(&worker->terminateLock, &initializerRWLock, sizeof(initializerRWLock));

    return worker;
}

WorkerPool *createWorkerPool(int count, JobHandler *handler) {
    if (unlikely(1 > count)) {
        error("minimum of 1 worker is required");
    }

    WorkerPool *pool = safeAlloc(sizeof(WorkerPool), "worker pool");
    pool->workerList = createWorker(pool);
    pool->jobHandler = handler;
    pool->jobFirst = NULL;
    pool->jobLast = NULL;

    memcpy(&pool->jobMutex, &initializerMutex, sizeof(initializerMutex));
    memcpy(&pool->jobCondition, &initializerCondition, sizeof(initializerCondition));

    Worker *next;
    Worker *last = pool->workerList;
    for (int i = 1; i < count; i++) {
        next = createWorker(pool);
        last->nextWorker = next;
        last = next;
    }

    return pool;
}

void workerPool_start(WorkerPool *pool) {
    Worker *worker = pool->workerList;
    while (worker) {
        if (unlikely(0 != pthread_create(&worker->thread, NULL, worker_function, worker))) {
            error("can not create thread");
        }
        worker = worker->nextWorker;
    }
}

void workerPool_stop(WorkerPool *pool) {
    Worker *worker = pool->workerList;
    while (NULL != worker) {
        worker_setTerminate(worker);
        worker = worker->nextWorker;
    }

    safeMutexLock(&pool->jobMutex);
    if (unlikely(0 != pthread_cond_broadcast(&pool->jobCondition))) {
        error("can not thread broadcast");
    }
    safeMutexUnlock(&pool->jobMutex);
}

void workerPool_join(WorkerPool *pool) {
    Worker *worker = pool->workerList;
    while (worker) {
        if (unlikely(0 != pthread_join(worker->thread, NULL))) {
            error("can not join thread");
        }
        worker = worker->nextWorker;
    }
}

void workerPool_free(WorkerPool *pool) {
    Worker *worker = pool->workerList;
    while (worker) {
        Worker *next = worker->nextWorker;
        worker_free(worker);
        worker = next;
    }

    Job *job = pool->jobFirst;
    while (job) {
        Job *nextJob = job->nextJob;
        job_free(job);
        job = nextJob;
    }

    free(pool);
    pool = NULL;
}

void workerPool_addJob(WorkerPool *pool, Job * restrict job) {
    safeMutexLock(&pool->jobMutex);

    if (pool->jobLast != NULL) {
        pool->jobLast->nextJob = job;
    } else {
        pool->jobFirst = job;
    }

    pool->jobLast = job;

    if (unlikely(0 != pthread_cond_signal(&pool->jobCondition))) {
        error("can not signal condition");
    }
    safeMutexUnlock(&pool->jobMutex);
}
