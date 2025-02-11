#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int food = 0; /*shared variable -- race condition here*/
int consumed = 1;/*shared variable -- race condition here*/
const short MAX_ENTRIES = 5;
int count = 0;

pthread_mutex_t lock; /*mutex lock*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; /*condition*/



/*Producer function*/
void *Producer(void *arg) { 
    while (count < MAX_ENTRIES) { /*loop until entries are done*/
        pthread_mutex_lock(&lock); /*lock varaibles*/
        while (!consumed) {  /*wait for consumed signal*/
            pthread_cond_wait(&cond, &lock); /*wait for signal from consumer*/
        }
        /*Enter food*/
        printf("Producer: "); 
        scanf("%d", &food);
        consumed = 0; /*consumed = false*/
        count++; /*increment entries*/
        pthread_cond_signal(&cond); /*signal consumer*/
        pthread_mutex_unlock(&lock); /*unlock variables*/
    }
    return NULL;
}

void *Consumer(void *arg) {
    int last_food = -1; 
    while (count < MAX_ENTRIES) {
        pthread_mutex_lock(&lock); /*lock variables*/
        while (consumed) { // Wait if no new food is available
            pthread_cond_wait(&cond, &lock); /*Wait for signal from producer*/
        }
        if (food != last_food) { /*Print only if food value changes*/
            printf("Consumer: %d\n", food); 
            last_food = food; /*update previos value*/
            consumed = 1; /*consumed = true*/
            pthread_cond_signal(&cond); /*signal producer*/
        }
        pthread_mutex_unlock(&lock); /*unlock variables*/
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&lock, NULL); /*create lock*/
    pthread_cond_init(&cond, NULL); /*create condition*/

    /*create and join threads*/
    pthread_t producer_id;
    pthread_t consumer_id;
    pthread_create(&producer_id, NULL, Producer, NULL);
    pthread_create(&consumer_id, NULL, Consumer, NULL);
    pthread_join(consumer_id, NULL);
    pthread_join(producer_id, NULL);

    /*destroy mutex and condition*/
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    exit(0);
}