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