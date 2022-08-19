#ifndef __AC_LIST_H__
#define __AC_LIST_H__

#include "typedefs.h"

typedef unsigned long int QueueSize;
typedef unsigned long int QueueIndex;

typedef struct {
    TrieIndex trieIndex;
    QueueIndex next;
} QueueCell;

typedef struct {
    QueueCell *cells;
    QueueSize size;
    QueueIndex rear;
    QueueIndex front;
} Queue;

Queue *create_Queue();
void queue_free(Queue *queue);
QueueIndex queue_Enqueue(Queue *queue, TrieIndex value);
TrieIndex queue_Dequeue(Queue *queue);
unsigned char queue_isEmpty(Queue *queue);

#endif