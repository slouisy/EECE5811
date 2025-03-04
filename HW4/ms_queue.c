#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "ms_queue.h"

void MS_Queue_Init(ms_queue_t *q) {
    ms_node_t *tmp = (ms_node_t *)malloc(sizeof(ms_node_t));
    assert(tmp != NULL);
    tmp->next = NULL;
    q->head = q->tail = tmp;
    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);
}

// Enqueue operation
void MS_Queue_Enqueue(ms_queue_t *q, int value) {
    ms_node_t *tmp = (ms_node_t *)malloc(sizeof(ms_node_t));
    assert(tmp != NULL);
    tmp->value = value;
    tmp->next = NULL;

    pthread_mutex_lock(&q->tail_lock);
    q->tail->next = tmp;
    q->tail = tmp;
    pthread_mutex_unlock(&q->tail_lock);
}

// Dequeue operation
int MS_Queue_Dequeue(ms_queue_t *q) {
    pthread_mutex_lock(&q->head_lock);
    ms_node_t *tmp = q->head;
    ms_node_t *new_head = tmp->next;

    if (new_head == NULL) {  // MS_Queue is empty
        pthread_mutex_unlock(&q->head_lock);
        return -1;
    }

    int value = new_head->value;
    q->head = new_head;
    pthread_mutex_unlock(&q->head_lock);
    free(tmp);
    return value;
}

void MS_Queue_Delete(ms_queue_t *q) {
    if (q == NULL) return; // Null check

    pthread_mutex_lock(&q->head_lock);
    pthread_mutex_lock(&q->tail_lock);

    ms_node_t *current = q->head;
    
    // Traverse and free all nodes
    while (current != NULL) {
        ms_node_t *next = current->next;
        free(current);
        current = next;
    }

    pthread_mutex_unlock(&q->head_lock);
    pthread_mutex_unlock(&q->tail_lock);

    // Destroy mutexes
    pthread_mutex_destroy(&q->head_lock);
    pthread_mutex_destroy(&q->tail_lock);
}
