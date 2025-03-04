#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "ms_queue.h"
#include "lf_queue.h"

ms_queue_t* MS = NULL; /*Michael and Scott Concurrent Queue*/
lf_queue_t* LF = NULL; /*lock-free concurrent queue*/

int Item_Count = 15;

void createQueues() { /*create the lists*/
    MS = (ms_queue_t*)malloc(sizeof(ms_queue_t));
    LF = (lf_queue_t*)malloc(sizeof(lf_queue_t));
    if(!MS || !LF) {perror("malloc failure");}

    MS_Queue_Init(MS);
    LF_Queue_Init(LF);
}

void* thread_enqueue(void* arg) {
    struct timespec start, end;

    int queue_type = *(int*)arg; /*0 for mich.scott and 1 for lock free*/
    switch(queue_type) {
         /*insert into ms queue*/
        case 0:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Item_Count; i++) {
                MS_Queue_Enqueue(MS, i);
            }
            break;
        /*insert into lock free queue*/
        case 1:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Item_Count; i++) { 
                LF_Queue_Enqueue(LF, i);
            }
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Thread Execution time = %f seconds\n", time_taken);

    return NULL;
}

void* thread_dequeue(void* arg) {
    struct timespec start, end;

    int queue_type = *(int*)arg; /*0 for mich.scott and 1 for lock free*/
    switch(queue_type) {
         /*insert into ms queue*/
        case 0:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Item_Count; i++) {
                MS_Queue_Dequeue(MS);
            }
            break;
        /*insert into lock free queue*/
        case 1:
            clock_gettime(CLOCK_MONOTONIC, &start);
            for(int i = 0; i < Item_Count; i++) { 
                LF_Queue_Dequeue(LF);
            }
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Thread Execution time = %f seconds\n", time_taken);

    return NULL;
}

int main(int argc, char *argv[]) {
    createQueues();

    if(argc == 2) { /*check for node code argument*/
        Item_Count = atoi(argv[1]);
    }

     /*thread array for simplicity*/
     pthread_t threads[8];

     /*Test insert time for normal list*/
    int* arg0 = malloc(sizeof(int));
    int* arg1 = malloc(sizeof(int));
    *arg0 = 0;
    *arg1 = 1;


    printf("Testing Michael and Scott queue enqueue times for %d items:\n", Item_Count);
    pthread_create(&threads[0], NULL, thread_enqueue, arg0);
    pthread_create(&threads[1], NULL, thread_enqueue, arg0);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    printf("Testing Lock Free queue enqueue times for %d items:\n", Item_Count);
    pthread_create(&threads[2], NULL, thread_enqueue, arg1);
    pthread_create(&threads[3], NULL, thread_enqueue, arg1);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);

    printf("Testing Michael and Scott queue dequeue times for %d items:\n", Item_Count);
    pthread_create(&threads[4], NULL, thread_dequeue, arg0);
    pthread_create(&threads[5], NULL, thread_dequeue, arg0);
    pthread_join(threads[4], NULL);
    pthread_join(threads[5], NULL);

    printf("Testing Lock Free queue dequeue times for %d items:\n", Item_Count);
    pthread_create(&threads[6], NULL, thread_dequeue, arg1);
    pthread_create(&threads[7], NULL, thread_dequeue, arg1);
    pthread_join(threads[6], NULL);
    pthread_join(threads[7], NULL);

    /*Delete queues*/
    MS_Queue_Delete(MS);
    LF_Queue_Delete(LF);
}