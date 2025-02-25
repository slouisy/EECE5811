#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "list.h"
#include "list_hh.h"

#define BORDER print_border() /*prints hash border*/

int Node_Count = 15; /*workload/ num of nodes*/
list_t* Regular = NULL; /*regular list*/
list_hh_t* HandOverHand = NULL; /*hand over hand list*/

void createLists() { /*create the lists*/
    Regular = (list_t*)malloc(sizeof(list_t));
    HandOverHand = (list_hh_t*)malloc(sizeof(list_hh_t));
    if(!Regular || !HandOverHand) {perror("malloc failure");}

    List_Init(Regular);
    List_HH_Init(HandOverHand);
}

void destoryLists() { /*create the lists*/
    List_Destroy(Regular);
    List_HH_Destroy(HandOverHand);
}

void* thread_insert(void* arg) {
    struct timespec start, end;

    int list_type = *(int*)arg; /*0 for normal and 1 for handovhand*/
    switch(list_type) {
         /*insert into regular list*/
        case 0:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Node_Count; i++) {
                List_Insert(Regular, i);
            }
            break;
        /*insert into handovhand list*/
        case 1:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Node_Count; i++) { 
                List_HH_Insert(HandOverHand, i);
            }
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Thread Execution time = %f seconds\n", time_taken);

    return NULL;
}

void* thread_lookup(void* arg) {
    struct timespec start, end;

    int list_type = *(int*)arg; /*0 for normal and 1 for handovhand*/
    switch(list_type) {
         /*insert into regular list*/
        case 0:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Node_Count; i++) {
                List_Lookup(Regular, i);
            }
            break;
        /*insert into handovhand list*/
        case 1:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Node_Count; i++) { 
                List_HH_Lookup(HandOverHand, i);
            }
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Thread Execution time = %f seconds\n", time_taken);

    return NULL;
}

void print_border() {
    for(int i=0; i<15; i++) {
        printf("#");
    }
    printf("#\n");
}

int main(int argc, char *argv[]) {

    if(argc == 2) { /*check for node code argument*/
        Node_Count = atoi(argv[1]);
    }

    createLists();
    
    /*thread array for simplicity*/
    pthread_t threads[8];

     /*Test insert time for normal list*/
    int* arg0 = malloc(sizeof(int));
    int* arg1 = malloc(sizeof(int));
    *arg0 = 0;
    *arg1 = 1;

    BORDER;

    printf("Testing regular list insert times for %d nodes:\n", Node_Count);
    pthread_create(&threads[0], NULL, thread_insert, arg0);
    pthread_create(&threads[1], NULL, thread_insert, arg0);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    BORDER;

     /*Test insert time for handovhand list*/
    printf("Testing hand over hand list insert times for %d nodes:\n", Node_Count);
    pthread_create(&threads[2], NULL, thread_insert, arg1);
    pthread_create(&threads[3], NULL, thread_insert, arg1);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);

    BORDER;

     /*Test lookup time for normal list*/
    printf("Testing regular list lookup times for %d nodes:\n", Node_Count);
    pthread_create(&threads[4], NULL, thread_lookup, arg0);
    pthread_create(&threads[5], NULL, thread_lookup, arg0);
    pthread_join(threads[4], NULL);
    pthread_join(threads[5], NULL);

    BORDER;

    /*Test lookup time for handovhand list*/
    printf("Testing hand over hand list lookup times for %d nodes:\n", Node_Count);
    pthread_create(&threads[6], NULL, thread_lookup, arg1);
    pthread_create(&threads[7], NULL, thread_lookup, arg1);
    pthread_join(threads[6], NULL);
    pthread_join(threads[7], NULL);

    BORDER;

    free(arg0); free(arg1);
    destoryLists();
}