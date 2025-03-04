#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include "lf_queue.h"

void LF_Queue_Init(lf_queue_t *q) {
    lf_node_t *tmp = (lf_node_t *)malloc(sizeof(lf_node_t));
    assert(tmp != NULL);
    tmp->next = NULL;
    atomic_store(&q->head, tmp);
    atomic_store(&q->tail, tmp);
}

void LF_Queue_Enqueue(lf_queue_t *q, int value) {
    lf_node_t *new_node = (lf_node_t *)malloc(sizeof(lf_node_t));
    if (new_node == NULL) {
        return;
    }
    new_node->value = value;
    atomic_store(&new_node->next, NULL);  // Proper atomic initialization

    while (1) {
        lf_node_t *tail = atomic_load(&q->tail);
        lf_node_t *next = atomic_load(&tail->next);

        if (next == NULL) {  // Tail is at the last node 
            if (atomic_compare_exchange_strong(&tail->next, &next, new_node)) {
                // Successfully added new node now set new tail
                atomic_compare_exchange_strong(&q->tail, &tail, new_node);
                return; // Exit loop
            }
        } else {
            // Tail is behind, get updated tail
            atomic_compare_exchange_strong(&q->tail, &tail, next);
        }
    }
}

int LF_Queue_Dequeue(lf_queue_t *q) {
    while (1) {
        lf_node_t* head = atomic_load(&q->head);
        lf_node_t* tail = atomic_load(&q->tail);
        lf_node_t* next = atomic_load(&head->next);

        if (head == tail) {  // Queue might be empty
            if (next == NULL) {  // Confirm empty queue
                return -1;
            }
            // Tail is lagging, try to advance it
            atomic_compare_exchange_strong(&q->tail, &tail, next);
        } else {
            if (next == NULL) {  // Unexpected NULL, should not happen
                return -1;
            }
            int value = next->value;
            if (atomic_compare_exchange_strong(&q->head, &head, next)) {
                free(head);
                return value;
            }
        }
    }
}

void LF_Queue_Delete(lf_queue_t *q) {
    if (q == NULL) return; // Null check
    
    // Try to set the head to NULL atomically
    lf_node_t *head = atomic_exchange(&q->head, NULL);
    
    // Free all remaining nodes
    while (head != NULL) {
        lf_node_t *next = atomic_load(&head->next);
        free(head);
        head = next;
    }

    // Mark tail as NULL to signal queue is gone
    atomic_store(&q->tail, NULL);

    free(q);  // Free the queue structure if it was dynamically allocated
}

