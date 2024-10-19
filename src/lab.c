#include "lab.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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

    //Allocate memory for the queue
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
    pthread_cond_init(&theQ->not_full, NULL);

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

//Add to queue
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

    //increase the location of the rear of the queue (in a circular style)
    theQ->rear = (theQ->rear + 1) % theQ->capacity;
    theQ->data[theQ->rear] = data;
    theQ->size++;

    //signal not empty then unlock
    pthread_cond_signal(&theQ->not_empty);
    pthread_mutex_unlock(&theQ->mutex);
}

//remove from front of queue
void *dequeue(queue_t theQ) {
    //Lock the queue while dealing with it
    pthread_mutex_lock(&theQ->mutex);

    //wait until the queue is not empty
    while(theQ->size == 0 && !theQ->is_shutdown){
        pthread_cond_wait(&theQ->not_empty, & theQ->mutex);
    }

    //unlock the queue if there is nothing in it
    if(theQ->size == 0) {
        pthread_mutex_unlock(&theQ->mutex);
        return NULL;
    }

    //Get the data that is being removed (also in a circular style)
    void *data = theQ->data[theQ->front];
    theQ->front = (theQ->front + 1) % theQ->capacity;
    theQ->size--;

    //signal not full and unlock queue
    pthread_cond_signal(&theQ->not_full);
    pthread_mutex_unlock(&theQ->mutex);

    //return what is dequeued
    return data;
}

//prepare the queue to be shutdown
void queue_shutdown(queue_t theQ) {
    //lock the queue
    pthread_mutex_lock(&theQ->mutex);

    printf("Time to shutdown queue!\n");

    //set is_shutdown flag to true
    theQ->is_shutdown = true;

    //Broadcast to both mutex conditions
    pthread_cond_broadcast(&theQ->not_full);
    pthread_cond_broadcast(&theQ->not_empty);

    //unlock queue
    pthread_mutex_unlock(&theQ->mutex);
}

//check if the thread is empty
bool is_empty(queue_t theQ) {
    //lock the queue
    pthread_mutex_lock(&theQ->mutex);
    
    //determine whether queue is empty
    bool empty = (theQ->size == 0);

    //unlock the queue
    pthread_mutex_unlock(&theQ->mutex);

    //return whether the queue is empty
    return empty;
}

//check if the queue is shutdown
bool is_shutdown (queue_t theQ) {
    //lock the queue
    pthread_mutex_lock(&theQ->mutex);

    //determine whether queue is shutdown
    bool shutdown = theQ->is_shutdown;

    //unlock the queue
    pthread_mutex_unlock(&theQ->mutex);

    //return whether the queue is shutdown
    return shutdown;
}