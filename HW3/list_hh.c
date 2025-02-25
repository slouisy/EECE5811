#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "list_hh.h"

void List_HH_Init(list_hh_t* L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

void List_HH_Insert(list_hh_t *L, int key) {
    node_hh_t* new = (node_hh_t*)malloc(sizeof(node_hh_t));
    if (new == NULL) {
        perror("malloc failure");
        return;
    }
    new->key = key;
    pthread_mutex_init(&new->lock, NULL);

    pthread_mutex_lock(&L->lock);
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->lock);
}

int List_HH_Lookup(list_hh_t *L, int key) {
    pthread_mutex_lock(&L->lock);  /*Lock list for safe head access*/
    node_hh_t* curr = L->head;

    if (curr != NULL) {
        pthread_mutex_lock(&curr->lock);  /*Lock first node*/
    }
    pthread_mutex_unlock(&L->lock);  /*Unlock list*/

    int found = 0;
    while (curr != NULL) {
        if (curr->key == key) {
            found = 1;
            pthread_mutex_unlock(&curr->lock);
            break;
        }

        node_hh_t* next = curr->next;
        if (next != NULL) {
            pthread_mutex_lock(&next->lock);  /*Lock next node*/
        }
        pthread_mutex_unlock(&curr->lock);  /*Unlock current node*/
        curr = next;
    }
    return found;
}

void List_HH_Destroy(list_hh_t *L) {
    /*destroy everything*/
    pthread_mutex_lock(&L->lock);
    node_hh_t *curr = L->head;
    while (curr) {
        pthread_mutex_lock(&curr->lock);
        node_hh_t *next = curr->next;
        pthread_mutex_unlock(&curr->lock);
        pthread_mutex_destroy(&curr->lock);
        free(curr);
        curr = next;
    }
    pthread_mutex_unlock(&L->lock);
    pthread_mutex_destroy(&L->lock);
    free(L);
}