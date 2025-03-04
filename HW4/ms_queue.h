#ifndef MS_QUEUE_H
#define MS_QUEUE_H

#include <stdlib.h>
#include <pthread.h>

// Node structure
typedef struct ms_node_t {
    int value;
    struct ms_node_t *next;
} ms_node_t;

// MS_Queue structure
typedef struct ms_queue_t {
    ms_node_t *head;
    ms_node_t *tail;
    pthread_mutex_t head_lock, tail_lock;
} ms_queue_t;

// Function prototypes
void MS_Queue_Init(ms_queue_t *q);
void MS_Queue_Enqueue(ms_queue_t *q, int value);
int MS_Queue_Dequeue(ms_queue_t *q);
void MS_Queue_Delete(ms_queue_t *q);

#endif 
