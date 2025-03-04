#ifndef LF_QUEUE_H
#define LF_QUEUE_H

#include <stdlib.h>
#include <stdatomic.h>

// Node structure
typedef struct lf_node_t {
    int value;
    _Atomic(struct lf_node_t*) next;  // Next pointer must be atomic
} lf_node_t;

// LF_Queue structure
typedef struct lf_queue_t {
    _Atomic(lf_node_t *) head;
    _Atomic(lf_node_t *) tail;
} lf_queue_t;

// Function prototypes
void LF_Queue_Init(lf_queue_t *q);
void LF_Queue_Enqueue(lf_queue_t *q, int value);
int LF_Queue_Dequeue(lf_queue_t *q);
void LF_Queue_Delete(lf_queue_t *q);

#endif 
