void enqueue(queue_t q, void *data) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->size == q->capacity && !q->is_shutdown) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    
    if (q->is_shutdown) {
        pthread_mutex_unlock(&q->mutex);
        return;
    }
    
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = data;
    q->size++;
    
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

void *dequeue(queue_t q) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->size == 0 && !q->is_shutdown) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    
    if (q->size == 0) {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }
    
    void *data = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    
    return data;
}

void queue_shutdown(queue_t q) {
    pthread_mutex_lock(&q->mutex);
    q->is_shutdown = true;
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
}

bool is_empty(queue_t q) {
    pthread_mutex_lock(&q->mutex);
    bool empty = (q->size == 0);
    pthread_mutex_unlock(&q->mutex);
    return empty;
}

bool is_shutdown(queue_t q) {
    pthread_mutex_lock(&q->mutex);
    bool shutdown = q->is_shutdown;
    pthread_mutex_unlock(&q->mutex);
    return shutdown;
}