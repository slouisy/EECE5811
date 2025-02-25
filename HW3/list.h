#ifndef LIST_H
#define LIST_H

#include <pthread.h>

typedef struct node {
    int key;
    struct node* next;
} node_t;

typedef struct {
    node_t* head;
    pthread_mutex_t lock;
} list_t;

void List_Init(list_t* L);
void List_Insert(list_t *L, int key);
int List_Lookup(list_t *L, int key);
void List_Destroy(list_t *L);
#endif