#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "list.h"

void List_Init(list_t* L) {
    L->head = NULL; /*set head to null*/
    pthread_mutex_init(&L->lock, NULL); /*initialize lock*/
}

void List_Insert(list_t *L, int key) {
    node_t* new = (node_t*)malloc(sizeof(node_t)); /*create new node*/
    if(new == NULL) { /*check for failure*/
        perror("malloc failure"); /*error*/
        return; /*exit*/
    }
    new->key = key;
    
    pthread_mutex_lock(&L->lock); /*lock critical section*/
    new->next = L->head; /*insert at the front of list*/
    L->head = new; 
    pthread_mutex_unlock(&L->lock); /*unlock critical section*/
}

int List_Lookup(list_t *L, int key) {
    int rv = 0; /*zero for failure*/
    pthread_mutex_lock(&L->lock); /*lock critical section*/
    node_t *curr = L->head;
    while (curr) {
        if (curr->key == key) {
             rv = 1; /*one for success*/
             break;
        }
        curr = curr->next;
     }
    pthread_mutex_unlock(&L->lock); /*unlock critical section*/
    return rv; 
}

void List_Destroy(list_t *L) {
    /*destory everything*/
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head; 
    node_t *next = NULL;
    while(curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    pthread_mutex_unlock(&L->lock);
    pthread_mutex_destroy(&L->lock);
    free(L);
}