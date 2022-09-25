#ifndef __AC_DAT__THREAD__H__
#define __AC_DAT__THREAD__H__


typedef void (JobHandler)(void *userData);

struct job;
struct workerPool;
struct worker;


int getAvailableCores(void);
struct workerPool *createWorkerPool(int count, JobHandler *handler);
void workerPool_start(struct workerPool *pool);
void workerPool_stop(struct workerPool *pool);
void workerPool_free(struct workerPool *pool);

#endif
