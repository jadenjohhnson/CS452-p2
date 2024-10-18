#include "lab.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct queue {
    void **data;
    int capacity;
    int size;
    int front;
    int rear;
    bool is_shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};

//Initialize the Queue
queue_t queue_init(int capacity) {
    //This is the queue
    queue_t theQ = (queue_t) malloc(sizeof(struct queue));
    
    //make sure queueu properly initialized
    if(theQ == NULL){
        return NULL;
    }

    //make sure data in queue is clean
    theQ->data = (void **) malloc(capacity *sizeof(void *));
    if(theQ->data == NULL){
        free(theQ);
        return NULL;
    }

    //assign all queue data
    theQ->capacity = capacity;
    theQ->size = 0;
    theQ->front = 0;
    theQ->rear = -1;
    theQ->is_shutdown = false;

    //Set the mutex
    pthread_mutex_init(&theQ->mutex, NULL);
    pthread_cond_init(&theQ->not_empty, NULL);
    pthread_cond_init(&theQ->not_empty, NULL);

    //The Queue is now set up
    return theQ;
}

//Destroy the Queue
void queue_destroy(queue_t theQ){

    //Make sure queue is not null
    if(theQ == NULL){
        return;
    }

    //destroy mutexs that are on the queue
    pthread_mutex_destroy(&theQ->mutex);
    pthread_cond_destroy(&theQ->not_full);
    pthread_cond_destroy(&theQ->not_empty);

    //free the data in the queue and the queue
    free(theQ->data);
    free(theQ);
}

void enqueue(queue_t theQ, void *data){

    //This is a critical section, so lock the queue
    pthread_mutex_lock(&theQ->mutex);

    //Wait on the not_full condition
    while(theQ->size == theQ->capacity && !theQ->is_shutdown){
        pthread_cond_wait(&theQ->not_full, &theQ->mutex);
    }

    //unlock the queue if it is shutdown
    if(theQ->is_shutdown){
        pthread_mutex_unlock(&theQ->mutex);
    }

    //increase the location of the rear of the queue
    theQ->rear = (theQ->rear + 1) % theQ->capacity;
    theQ->data[theQ->rear] = data;
    theQ->size++;

    //signal not empty then unlock
    pthread_cond_signal(&theQ->not_empty);
    pthread_mutex_unlock(&theQ->mutex);
}