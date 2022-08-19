#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ac_list.h"

QueueIndex queue_getFirstFree(Queue *queue) {
    return queue->cells[0].next;
}

void queue_setFirstFree(Queue *queue, QueueIndex index) {
    queue->cells[0].next = index;
}

void queue_print(Queue *queue) {
    printf("\n\n");
    printf("FirstFree: %ld, front: %ld, rear: %ld", queue_getFirstFree(queue), queue->front, queue->rear);
    printf("\n\n");
    for (int i = 0; i < queue->size; i++) {
        printf("%4d | ", i);
    }
    printf("\n");
    for (int i = 0; i < queue->size; i++) {
        printf("%4ld | ", queue->cells[i].trieIndex);
    }
    printf("\n");
    for (int i = 0; i < queue->size; i++) {
        printf("%4ld | ", queue->cells[i].next);
    }
    printf("\n\n");
}

void queue_connectList(Queue *queue, QueueIndex fromIndex, QueueIndex toIndex) {
    for (QueueIndex i = fromIndex; i < toIndex; i++) {
        queue->cells[i].next = i+1;
    }
}

Queue *create_Queue() {
    Queue *queue = calloc(1, sizeof(Queue));
    if (queue == NULL) {
        fprintf(stderr, "can not allocate %lu memory for queue", sizeof(Queue));
        exit(1);
    }

    queue->size = MAX_ALPHABET_SIZE;
    queue->cells = malloc(sizeof(QueueCell) * queue->size);
    if (queue->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for queue values", sizeof(TrieIndex) * queue->size);
        exit(1);
    }

    queue_connectList(queue, 0, queue->size - 1);

    return queue;
}

void queue_poolReallocate(Queue *queue) {
    QueueSize newSize = queue->size + (long int)ceill(((long double)queue->size / 2));
    queue->cells = realloc(queue->cells, sizeof(QueueCell) * newSize);
    if (queue->cells == NULL) {
        fprintf(stderr, "can not allocate %lu memory for queue values", sizeof(TrieIndex) * newSize);
        exit(1);
    }

    queue_connectList(queue, queue->size, newSize);

    if (queue_getFirstFree(queue) == 0) {
        queue_setFirstFree(queue, queue->size);
    } else {
        QueueIndex prevEmpty = queue->size - 1;
        while (prevEmpty != 0 && queue->cells[prevEmpty].trieIndex != 0) {
            prevEmpty--;
        }

        queue->cells[prevEmpty].next = queue->size;
    }

    queue->size = newSize;
}

QueueIndex queue_Enqueue(Queue *queue, TrieIndex value) {
    QueueIndex firstFree = queue_getFirstFree(queue);
    if (firstFree == 0) {
        queue_poolReallocate(queue);
        firstFree = queue_getFirstFree(queue);
    }
    QueueIndex newFirstFree = queue->cells[firstFree].next;

    queue->cells[firstFree].trieIndex = value;
    queue->cells[firstFree].next = 0;

    if (queue->rear) {
        queue->cells[queue->rear].next = firstFree;
    }
    if (!queue->front) {
        queue->front = firstFree;
    }

    queue->rear = firstFree;
    queue->cells[0].next = newFirstFree;
    queue_setFirstFree(queue, newFirstFree);

    return firstFree;
}

TrieIndex queue_Dequeue(Queue *queue) {
    QueueIndex index = queue->front;
    TrieIndex trieIndex = queue->cells[index].trieIndex;

    queue->cells[index].trieIndex = 0;
    queue->front = queue->cells[index].next;

    if (index == queue->rear) {
        queue->rear = 0;
        queue->cells[index].next = 0;
        queue_setFirstFree(queue, index);
        return trieIndex;
    }

    QueueIndex firstFree = queue_getFirstFree(queue);
    if (firstFree == 0) {
        queue_setFirstFree(queue, index);
    } else if (firstFree > index) {
        queue->cells[index].next = firstFree;
        queue_setFirstFree(queue, index);
    } else {
        QueueIndex prevEmpty = index;
        while (prevEmpty != 0) {
            if (queue->cells[prevEmpty].trieIndex == 0) {
                break;
            }
            prevEmpty--;
        }


        queue->cells[index].next = queue->cells[prevEmpty].next;
        queue->cells[prevEmpty].next = index;
    }

    return trieIndex;
}

unsigned char queue_isEmpty(Queue *queue) {
    return queue->front == 0 && queue->rear == 0;
}

void queue_free(Queue *queue) {
    free(queue);
}
