#ifndef __AC_H__
#define __AC_H__

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
    QueueIndex firstFree;
    QueueIndex rear;
    QueueIndex front;
} Queue;

Queue *create_Queue();
void queue_poolReallocate(Queue *queue);
QueueIndex queue_Enqueue(Queue *queue, TrieIndex value);
TrieIndex queue_Dequeue(Queue *queue);
void queue_print(Queue *queue);

#endif